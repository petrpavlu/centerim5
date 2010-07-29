/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2009-2010 by CenterIM developers
 *
 * This file is part of CenterIM.
 *
 * CenterIM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * CenterIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * */

#include "CoreManager.h"

#include "ConsuiCurses.h"
#include "KeyConfig.h"

#include <sys/ioctl.h>
#include <cstring>
#include <cstdio>
#include "gettext.h"

// based on glibmm code
class SourceConnectionNode
{
	public:
		explicit inline SourceConnectionNode(const sigc::slot_base& nslot);

		static void *notify(void *data);
		static void destroy_notify_callback(void *data);
		static gboolean source_callback(void *data);

		inline void install(GSource *nsource);
		inline sigc::slot_base *get_slot();

	protected:

	private:
		sigc::slot_base slot;
		GSource *source;
};

inline SourceConnectionNode::SourceConnectionNode(
		const sigc::slot_base& nslot)
: slot(nslot)
, source(0)
{
	slot.set_parent(this, &SourceConnectionNode::notify);
}

void *SourceConnectionNode::notify(void *data)
{
	SourceConnectionNode *self
		= reinterpret_cast<SourceConnectionNode *>(data);

	/* If there is no object, this call was triggered from
	 * destroy_notify_handler(), because we set self->source to 0 there. */
	if (self->source) {
		GSource *s = self->source;
		self->source = 0;
		g_source_destroy(s);

		/* Destroying the object triggers execution of
		 * destroy_notify_handler(), eiter immediately or later, so we leave
		 * that to do the deletion. */
	}

	return 0;
}

void SourceConnectionNode::destroy_notify_callback(void *data)
{
	SourceConnectionNode *self
		= reinterpret_cast<SourceConnectionNode *>(data);

	if (self) {
		/* The GLib side is disconnected now, thus the GSource* is no longer
		 * valid. */
		self->source = 0;

		delete self;
	}
}

gboolean SourceConnectionNode::source_callback(void *data)
{
	SourceConnectionNode *conn_data
		= reinterpret_cast<SourceConnectionNode *>(data);

	// recreate the specific slot from the generic slot node
	return (*static_cast<sigc::slot<bool> *>(conn_data->get_slot()))();
}

inline void SourceConnectionNode::install(GSource *nsource)
{
	source = nsource;
}

inline sigc::slot_base *SourceConnectionNode::get_slot()
{
	return &slot;
}

#define CONTEXT_COREMANAGER "coremanager"

CoreManager *CoreManager::Instance()
{
	static CoreManager instance;
	return &instance;
}

CoreManager::CoreManager()
: top_input_processor(NULL), channel(NULL), channel_id(0), tk(NULL)
, utf8(false), gmainloop(NULL) , screen_width(0), screen_height(0)
, redrawpending(false), resizepending(false)
{
	StdinInputInit();

	/**
	 * @todo Check all return values here. Throw an exception if we can't init
	 * curses.
	 */
	Curses::initscr();

	if (Curses::has_colors())
		Curses::start_color();
	Curses::curs_set(0);
	Curses::nonl();
	Curses::raw();

	screen_width = Curses::getmaxx();
	screen_height = Curses::getmaxy();

	// create a new loop
	gmainloop = g_main_loop_new(NULL, FALSE);

	/* Register all InputProcessor key configuration (it needs to be called
	 * before the first DeclareBindable). */
	KEYCONFIG->Register();
	DeclareBindables();
}

CoreManager::~CoreManager()
{
	StdinInputUnInit();

	/// @todo Close all windows?

	Curses::endwin();
}

void CoreManager::DeclareBindables()
{
	DeclareBindable(CONTEXT_COREMANAGER, "redraw-screen",
			sigc::mem_fun(this, &CoreManager::Redraw),
			InputProcessor::Bindable_Override);
}

DEFINE_SIG_REGISTERKEYS(CoreManager, RegisterKeys);
bool CoreManager::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_COREMANAGER, "redraw-screen",
			_("Redraw the complete screen immediately"),
			Keys::UnicodeTermKey("l", TERMKEY_KEYMOD_CTRL));
	return true;
}

void CoreManager::StartMainLoop()
{
	g_main_loop_run(gmainloop);
}

void CoreManager::QuitMainLoop()
{
	g_main_loop_quit(gmainloop);
}

void CoreManager::AddWindow(Window& window)
{
	if (!HasWindow(window)) {
		WindowInfo info;
		info.window = &window;
		info.redraw = window.signal_redraw.connect(sigc::mem_fun(this, &CoreManager::WindowRedraw));
		info.resize = signal_resize.connect(sigc::mem_fun(&window, &Window::ScreenResized));
		windows.push_back(info);
	}

	window.ScreenResized();
	FocusWindow();
	Redraw();
}

void CoreManager::RemoveWindow(Window& window)
{
	Windows::iterator i;

	for (i = windows.begin(); i != windows.end(); i++)
		if (i->window == &window)
			break;

	g_assert(i != windows.end());

	i->redraw.disconnect();
	i->resize.disconnect();
	windows.erase(i);

	FocusWindow();
	Redraw();
}

bool CoreManager::HasWindow(Window& window) const
{
	for (Windows::const_iterator i = windows.begin(); i != windows.end(); i++)
		if (i->window == &window)
			return true;

	return false;
}

Window *CoreManager::GetTopWindow()
{
	if (windows.size())
		return windows.back().window;
	return NULL;
}

void CoreManager::EnableResizing()
{
	ScreenResized();

	// register resize handler
	struct sigaction sig;
	sig.sa_handler = SignalHandler;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = 0;
	sigaction(SIGWINCH, &sig, NULL);
}

void CoreManager::DisableResizing()
{
	// unregister resize handler
	struct sigaction sig;
	sig.sa_handler = SIG_DFL;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = 0;
	sigaction(SIGWINCH, &sig, NULL);
}

sigc::connection CoreManager::TimeoutConnect(const sigc::slot<bool>& slot,
		unsigned interval, int priority)
{
	SourceConnectionNode *conn_node = new SourceConnectionNode(slot);
	sigc::connection connection(*conn_node->get_slot());

	GSource *source = g_timeout_source_new(interval);

	if (priority != G_PRIORITY_DEFAULT)
		g_source_set_priority(source, priority);

	g_source_set_callback(
			source, &SourceConnectionNode::source_callback, conn_node,
			&SourceConnectionNode::destroy_notify_callback);

	g_source_attach(source, NULL);
	g_source_unref(source); // GMainContext holds a reference

	conn_node->install(source);
	return connection;
}

void CoreManager::TimeoutOnceConnect(const sigc::slot<void>& slot,
		unsigned interval, int priority)
{
	TimeoutConnect(sigc::bind_return(slot, FALSE), interval, priority);
}

bool CoreManager::ProcessInput(const TermKeyKey& key)
{
	if (top_input_processor && top_input_processor->ProcessInput(key))
		return true;

	return InputProcessor::ProcessInput(key);
}

gboolean CoreManager::io_input_error(GIOChannel *source, GIOCondition cond)
{
	// log a critical warning and bail out if we lost stdin
	g_critical("Stdin lost!\n");
	exit(1);

	return TRUE;
}

gboolean CoreManager::io_input(GIOChannel *source, GIOCondition cond)
{
	termkey_advisereadable(tk);

	TermKeyKey key;
	/**
	 * @todo Actually we should call termkey_getkey() instead of
	 * termkey_getkey_force(). See libtermkey async demo.
	 */
	while (termkey_getkey_force(tk, &key) == TERMKEY_RES_KEY) {
		if (key.type == TERMKEY_TYPE_UNICODE && !utf8) {
			gsize bwritten;
			GError *err = NULL;
			gchar *utf8;

			// convert data from user charset to UTF-8
			if (!(utf8 = g_locale_to_utf8(key.utf8, -1, NULL, &bwritten,
							&err))) {
				if (err) {
					g_warning(_("Error converting input to UTF-8 (%s).\n"),
							err->message);
					g_error_free(err);
					err = NULL;
				}
				else
					g_warning(_("Error converting input to UTF-8.\n"));
				continue;
			}

			memcpy(key.utf8, utf8, bwritten + 1);
			g_free(utf8);

			key.code.codepoint = g_utf8_get_char(key.utf8);
		}

		ProcessInput(key);
	}

	return TRUE;
}

void CoreManager::StdinInputInit()
{
	// init libtermkey
	TERMKEY_CHECK_VERSION;
	if (!(tk = termkey_new(STDIN_FILENO, TERMKEY_FLAG_NOTERMIOS))) {
		g_critical(_("Libtermkey initialization failed.\n"));
		exit(1);
	}
	utf8 = g_get_charset(NULL);

	channel = g_io_channel_unix_new(STDIN_FILENO);
	// set channel encoding to NULL so it can be unbuffered
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);
	g_io_channel_set_close_on_unref(channel, TRUE);

	channel_id = g_io_add_watch_full(channel, G_PRIORITY_HIGH,
			(GIOCondition)(G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_PRI),
			io_input_, this, NULL);

	g_io_add_watch_full(channel, G_PRIORITY_HIGH, (G_IO_NVAL),
			io_input_error_, this, NULL);

	g_io_channel_unref(channel);
}

void CoreManager::StdinInputUnInit()
{
	termkey_destroy(tk);

	g_source_remove(channel_id);
	channel_id = 0;
	g_io_channel_unref(channel);
	channel = NULL;
}

void CoreManager::SignalHandler(int signum)
{
	if (signum == SIGWINCH)
		COREMANAGER->ScreenResized();
}

void CoreManager::ScreenResized()
{
	if (!resizepending) {
		resizepending = true;
		TimeoutOnceConnect(sigc::mem_fun(this, &CoreManager::Resize), 0, 0);
	}
}

void CoreManager::Resize()
{
	struct winsize size;

	if (resizepending)
		resizepending = false;

	if (ioctl(fileno(stdout), TIOCGWINSZ, &size) >= 0)
		Curses::resizeterm(size.ws_row, size.ws_col);

	// save new screen size
	screen_width = size.ws_col;
	screen_height = size.ws_row;

	signal_resize();
}

void CoreManager::Draw()
{
	if (redrawpending) {
		Curses::erase();
		Curses::noutrefresh();
		
		for (Windows::iterator i = windows.begin(); i != windows.end(); i++)
			i->window->Draw();

		// copy virtual ncurses screen to the physical screen
		Curses::doupdate();

		redrawpending = false;
	}
}

void CoreManager::Redraw()
{
	if (!redrawpending) {
		redrawpending = true;
		TimeoutOnceConnect(sigc::mem_fun(this, &CoreManager::Draw), 0, 0);
	}
}

void CoreManager::WindowRedraw(Widget& widget)
{
	Redraw();
}

void CoreManager::FocusWindow()
{
	Window *focus = dynamic_cast<Window *>(GetInputChild());

	// take the focus from the old window with the focus
	if (focus) {
		Widget *widget = focus->GetFocusWidget();
		if (widget)
			widget->UngrabFocus();
		ClearInputChild();
	}

	// check if there are any windows left
	Window *win = NULL;
	if (windows.size())
		win = windows.back().window;

	// give the focus to the top window if there is one
	if (win) {
		SetInputChild(*win);
		win->RestoreFocus();
	}
}
