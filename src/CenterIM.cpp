/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010 by CenterIM developers
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

#include "CenterIM.h"

#include "Accounts.h"
#include "BuddyList.h"
#include "Conf.h"
#include "Connections.h"
#include "Conversations.h"
#include "Log.h"
#include "Transfers.h"

#include "AccountStatusMenu.h"
#include "GeneralMenu.h"
#include "Defines.h"

#include <cppconsui/ColorScheme.h>
#include <cppconsui/Keys.h>
#include <cstring>
#include "gettext.h"

#define CONTEXT_CENTERIM "centerim"

std::vector<CenterIM::LogBufferItem> *CenterIM::logbuf = NULL;

CenterIM *CenterIM::Instance(void)
{
	static CenterIM instance;
	return &instance;
}

CenterIM::CenterIM(void)
{
	memset(&centerim_core_ui_ops, 0, sizeof(centerim_core_ui_ops));
	memset(&logbuf_debug_ui_ops, 0, sizeof(logbuf_debug_ui_ops));
	memset(&centerim_glib_eventloops, 0, sizeof(centerim_glib_eventloops));

	DeclareBindables();
}

void CenterIM::Run(void)
{
	PurpleInit();

	// initialize Conf component so we can calculate area sizes of all windows
	Conf::Instance();

	ColorSchemeInit();

	// initialize Log component
	Log::Instance()->Show();
	if (logbuf) {
		for (LogBufferItems::iterator i = logbuf->begin(); i != logbuf->end(); i++) {
			LOG->purple_print(i->level, i->category, i->arg_s);
			g_free(i->category);
			g_free(i->arg_s);
		}

		delete logbuf;
		logbuf = NULL;
	}

	Accounts::Instance();
	Conversations::Instance();

	// initialize UI
	BuddyList::Instance()->Show();

	LOG->Write(Log::Level_info, _("Welcome to CenterIM 5. Press F4 to display main menu.\n"));
	Application::Run();
}

void CenterIM::Quit(void) {
	Application::Quit();

	// clean up
	purple_core_quit();
}

GHashTable *CenterIM::get_ui_info(void)
{ 
	static GHashTable *ui_info = NULL;

	if (ui_info == NULL) {
		ui_info = g_hash_table_new(g_str_hash, g_str_equal);

		g_hash_table_insert(ui_info, (void *) "name", (void *) PACKAGE_NAME);
		g_hash_table_insert(ui_info, (void *) "version", (void *) PACKAGE_VERSION);
		g_hash_table_insert(ui_info, (void *) "website", (void *) "http://www.centerim.org");
		// TODO
		g_hash_table_insert(ui_info, (void *) "dev_website", (void *) "http://www.centerim.org");
		g_hash_table_insert(ui_info, (void *) "client_type", (void *) "pc");
	}

	return ui_info;
}

guint CenterIM::timeout_add(guint interval, GSourceFunc function, gpointer data)
{
	return g_timeout_add(interval, function, data);
}

gboolean CenterIM::timeout_remove(guint handle)
{
	return g_source_remove(handle);
}

#define PURPLE_GLIB_READ_COND  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define PURPLE_GLIB_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

guint CenterIM::input_add(int fd, PurpleInputCondition condition,
	PurpleInputFunction function, gpointer data)
{
	IOClosure *closure = new IOClosure;
	GIOChannel *channel;
	int cond = 0;

	closure->function = function;
	closure->data = data;

	if (condition & PURPLE_INPUT_READ)
		cond |= PURPLE_GLIB_READ_COND;
	if (condition & PURPLE_INPUT_WRITE)
		cond |= PURPLE_GLIB_WRITE_COND;

	channel = g_io_channel_unix_new(fd);
	closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT, (GIOCondition)cond,
				purple_glib_io_input, closure, purple_glib_io_destroy);

	g_io_channel_unref(channel);
	return closure->result;
}

gboolean CenterIM::input_remove(guint handle)
{
	return g_source_remove(handle);
}

gboolean CenterIM::purple_glib_io_input(GIOChannel *source, GIOCondition condition, gpointer data)
{
	IOClosure *closure = (IOClosure *) data;
	int purple_cond = 0;

	if (condition & PURPLE_GLIB_READ_COND)
		purple_cond |= PURPLE_INPUT_READ;
	if (condition & PURPLE_GLIB_WRITE_COND)
		purple_cond |= PURPLE_INPUT_WRITE;

	closure->function(closure->data, g_io_channel_unix_get_fd(source),
			  (PurpleInputCondition)purple_cond);

	return TRUE;
}

void CenterIM::purple_glib_io_destroy(gpointer data)
{
	delete (IOClosure *) data;
}

void CenterIM::tmp_purple_print(PurpleDebugLevel level, const char *category, const char *arg_s)
{
	if (!logbuf)
		logbuf = new std::vector<LogBufferItem>;

	LogBufferItem item;
	item.level = level;
	item.category = g_strdup(category);
	item.arg_s = g_strdup(arg_s);
	logbuf->push_back(item);
}

void CenterIM::PurpleInit(void)
{
	// set the configuration file location
	char *path;
	path = g_build_filename(purple_home_dir(), CIM_CONFIG_PATH, NULL);
	purple_util_set_user_dir(path);
	g_free(path);

	/* This does not disable debugging, but rather it disables printing to
	 * stdout. Don't change this to TRUE or things will get messy. */
	purple_debug_set_enabled(FALSE);

	/* This catches and buffers libpurple debug messages until the Log object
	 * can be instantiated
	 * */
	logbuf_debug_ui_ops.print = tmp_purple_print;
	logbuf_debug_ui_ops.is_enabled = tmp_is_enabled;
	purple_debug_set_ui_ops(&logbuf_debug_ui_ops);

	centerim_core_ui_ops.get_ui_info = get_ui_info;
	purple_core_set_ui_ops(&centerim_core_ui_ops);

	// set the uiops for the eventloop
	centerim_glib_eventloops.timeout_add = timeout_add;
	centerim_glib_eventloops.timeout_remove = timeout_remove;
	centerim_glib_eventloops.input_add = input_add;
	centerim_glib_eventloops.input_remove = input_remove;
	purple_eventloop_set_ui_ops(&centerim_glib_eventloops);

	// in case we ever write centerim specific plugins
	path = g_build_filename(purple_user_dir(), "plugins", NULL);
	purple_plugins_add_search_path(path);
	g_free(path);

	if (!purple_core_init(PACKAGE_NAME)) {
		// can't do much without libpurple
		throw EXCEPTION_PURPLE_CORE_INIT;
	}
}

void CenterIM::ColorSchemeInit()
{
	// default colors init
	/// @todo move this to a default cfg
	COLORSCHEME->SetColorPair("accountstatusmenu",	"panel",		"line",		Curses::Color::CYAN,	Curses::Color::BLACK);
	COLORSCHEME->SetColorPair("accountstatusmenu",	"horizontalline",	"line",		Curses::Color::CYAN,	Curses::Color::BLACK);
	COLORSCHEME->SetColorPair("accountstatusmenu",	"button",		"normal",	Curses::Color::CYAN,	Curses::Color::BLACK);

	COLORSCHEME->SetColorPair("accountstatuspopup",	"panel",		"line",		Curses::Color::CYAN,	Curses::Color::BLACK);
	COLORSCHEME->SetColorPair("accountstatuspopup",	"horizontalline",	"line",		Curses::Color::CYAN,	Curses::Color::BLACK);
	COLORSCHEME->SetColorPair("accountstatuspopup",	"button",		"normal",	Curses::Color::CYAN,	Curses::Color::BLACK);

	COLORSCHEME->SetColorPair("accountwindow",	"panel",		"line",		Curses::Color::CYAN,	Curses::Color::BLACK);
	COLORSCHEME->SetColorPair("accountwindow",	"horizontalline",	"line",		Curses::Color::CYAN,	Curses::Color::BLACK);
	COLORSCHEME->SetColorPair("accountwindow",	"verticalline",		"line",		Curses::Color::CYAN,	Curses::Color::BLACK);

	COLORSCHEME->SetColorPair("buddylist",		"treeview",		"line",		Curses::Color::GREEN,	Curses::Color::BLACK);
	COLORSCHEME->SetColorPair("buddylist",		"panel",		"line",		Curses::Color::BLUE,	Curses::Color::BLACK,	Curses::Attr::BOLD);
	COLORSCHEME->SetColorPair("buddylist",		"button",		"normal",	Curses::Color::GREEN,	Curses::Color::BLACK);
	COLORSCHEME->SetColorPair("buddylistgroup",	"button",		"normal",	Curses::Color::YELLOW,	Curses::Color::BLACK,	Curses::Attr::BOLD);

	COLORSCHEME->SetColorPair("generalmenu",	"panel",		"line",		Curses::Color::CYAN,	Curses::Color::BLACK);
	COLORSCHEME->SetColorPair("generalmenu",	"horizontalline",	"line",		Curses::Color::CYAN,	Curses::Color::BLACK);
	COLORSCHEME->SetColorPair("generalmenu",	"button",		"normal",	Curses::Color::CYAN,	Curses::Color::BLACK);

	COLORSCHEME->SetColorPair("log",		"panel",		"line",		Curses::Color::BLUE,	Curses::Color::BLACK,	Curses::Attr::BOLD);
	COLORSCHEME->SetColorPair("log",		"textview",		"text",		Curses::Color::CYAN,	Curses::Color::BLACK);

	COLORSCHEME->SetColorPair("conversation",	"textview",		"text",		Curses::Color::MAGENTA,	Curses::Color::BLACK);
	COLORSCHEME->SetColorPair("conversation",	"textview",		"color1",	Curses::Color::CYAN,	Curses::Color::BLACK);
	COLORSCHEME->SetColorPair("conversation",	"textview",		"color2",	Curses::Color::YELLOW,	Curses::Color::BLACK,	Curses::Attr::BOLD);
	COLORSCHEME->SetColorPair("conversation",	"panel",		"line",		Curses::Color::BLUE,	Curses::Color::BLACK,	Curses::Attr::BOLD);
	COLORSCHEME->SetColorPair("conversation",	"horizontalline",	"line",		Curses::Color::BLUE,	Curses::Color::BLACK,	Curses::Attr::BOLD);
}

void CenterIM::DeclareBindables()
{
	DeclareBindable(CONTEXT_CENTERIM, "quit",
			sigc::mem_fun(this, &CenterIM::Quit),
			InputProcessor::Bindable_Override);
	DeclareBindable(CONTEXT_CENTERIM, "accountstatusmenu",
			sigc::mem_fun(this, &CenterIM::OpenAccountStatusMenu),
			InputProcessor::Bindable_Override);
	DeclareBindable(CONTEXT_CENTERIM, "generalmenu",
			sigc::mem_fun(this, &CenterIM::OpenGeneralMenu),
			InputProcessor::Bindable_Override);
}

DEFINE_SIG_REGISTERKEYS(CenterIM, RegisterKeys);
bool CenterIM::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_CENTERIM, "quit", _("Quit CenterIM."),
			Keys::UnicodeTermKey("q", TERMKEY_KEYMOD_CTRL));
	RegisterKeyDef(CONTEXT_CENTERIM, "accountstatusmenu",
			_("Open the account status menu."),
			Keys::FunctionTermKey(3));
	RegisterKeyDef(CONTEXT_CENTERIM, "generalmenu",
			_("Open the general menu."),
			Keys::FunctionTermKey(4));
	return true;
}

Rect CenterIM::ScreenAreaSize(ScreenArea area)
{
	return areaSizes[area];
}

void CenterIM::ScreenResized(void) {
	Rect size = CONF->GetBuddyListDimensions();
	size.width = (int) (size.width * (windowmanager->getScreenW() / (double) originalW));
	size.height = windowmanager->getScreenH();
	areaSizes[BuddyListArea] = size;
	
	size = CONF->GetLogDimensions();
	size.x = areaSizes[BuddyListArea].width;
	size.width = windowmanager->getScreenW() - size.x;
	size.height = (int) (size.height * (windowmanager->getScreenH() / (double) originalH));
	size.y = windowmanager->getScreenH() - size.height;
	areaSizes[LogArea] = size;

	areaSizes[ChatArea].x = areaSizes[BuddyListArea].width;
	areaSizes[ChatArea].y = 0;
	areaSizes[ChatArea].width = windowmanager->getScreenW() - areaSizes[ChatArea].x;
	areaSizes[ChatArea].height = windowmanager->getScreenH() - areaSizes[LogArea].height;

	areaSizes[WholeArea].x = 0;
	areaSizes[WholeArea].y = 0;
	areaSizes[WholeArea].width = windowmanager->getScreenW();
	areaSizes[WholeArea].height = windowmanager->getScreenH();
}

void CenterIM::OpenAccountStatusMenu(void)
{
	//TODO get coords from config
	AccountStatusMenu *menu = new AccountStatusMenu(40,0, 40, 20);
	menu->Show();
}

void CenterIM::OpenGeneralMenu(void)
{
	//TODO get coords from config
	GeneralMenu *menu = new GeneralMenu(40, 0, 40, 6);
	menu->Show();
}

