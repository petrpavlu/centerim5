/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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

#include "WindowManager.h"

#include "Window.h"
#include "Keys.h"
#include "KeyConfig.h"

#include <sys/ioctl.h>
#include <signal.h>
#include <cstring>
#include <cstdio>
#include <glib.h>
#include "gettext.h"

#define CONTEXT_WINDOWMANAGER "windowmanager"

WindowManager *WindowManager::Instance()
{
	static WindowManager instance;
	return &instance;
}

WindowManager::WindowManager(void)
: screenW(Curses::getmaxx())
, screenH(Curses::getmaxy())
, redrawpending(false)
, resizepending(false)
{
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

	KEYCONFIG->Register(); // registers all InputProcessor key configuration (it needs to be called before the first DeclareBindable)
	DeclareBindables();
}

WindowManager::~WindowManager(void)
{
	// @todo close all windows?

	if (Curses::endwin() == Curses::C_ERR)
		{}//TODO throw an exeption
}

void WindowManager::signal_handler(int signum)
{
	if (signum == SIGWINCH)
		Instance()->ScreenResized();
}

gboolean WindowManager::timeout_once_draw(gpointer data)
{
	static_cast<WindowManager *>(data)->Draw();
	return FALSE;
}

gboolean WindowManager::timeout_once_resize(gpointer data)
{
	static_cast<WindowManager *>(data)->Resize();
	return FALSE;
}

void WindowManager::DeclareBindables()
{
	DeclareBindable(CONTEXT_WINDOWMANAGER, "redraw-screen",
			sigc::mem_fun(this, &WindowManager::Redraw),
			InputProcessor::Bindable_Override);
}

DEFINE_SIG_REGISTERKEYS(WindowManager, RegisterKeys);
bool WindowManager::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_WINDOWMANAGER, "redraw-screen",
			_("Redraw the complete screen immediately"),
			Keys::UnicodeTermKey("l", TERMKEY_KEYMOD_CTRL));
	return true;
}

void WindowManager::Add(Window *window)
{
	WindowInfo info;

	if (!HasWindow(window)) {
		info.window = window;
		info.redraw = window->signal_redraw.connect(sigc::mem_fun(this, &WindowManager::WindowRedraw));
		info.resize = signal_resize.connect(sigc::mem_fun(window, &Window::ScreenResized));
		windows.push_back(info);
	}

	window->ScreenResized();
	FocusWindow();
	Redraw();
}

void WindowManager::Remove(Window *window)
{
	WindowInfo info;
	Windows::iterator i;

	if (!window) return;

	i = FindWindow(window);

	if (i == windows.end())
		return; //TODO some debug here. cannot remove non-managed window

	info = *i;

	info.redraw.disconnect();
	info.resize.disconnect();
	windows.erase(i);

	if (window == GetInputChild())
		ClearInputChild();

	info.window->GetWindow()->erase();
	info.window->GetWindow()->noutrefresh();

	FocusWindow();
	Draw();
}

void WindowManager::FocusWindow(void)
{
	Window *win, *focus = NULL;
	InputProcessor *inputchild;
	Widget *widget = NULL;

	inputchild = GetInputChild();

	if ((focus = dynamic_cast<Window*>(inputchild)) == NULL) {
		; //TODO error about invalid input child
	}

	/* Take the focus from the old window with the focus */
	if (focus) {
		widget = focus->GetFocusWidget();
		if (widget)
			widget->UngrabFocus();
		ClearInputChild();
	}

	/* Check if there are any windows left */
	if (windows.size()) {
		win = windows.back().window;
	} else {
		win = NULL;
	}

	/* Give the focus to the top window if there is one */
	if (win) {
		SetInputChild(*win);
		win->RestoreFocus();
	}
}

WindowManager::Windows::iterator WindowManager::FindWindow(Window *window)
{
	Windows::iterator i;

	if (windows.size()) {
		for (i = windows.begin(); i != windows.end(); i++)
			if ((*i).window == window) break;
	}

	return i;
}

bool WindowManager::HasWindow(Window *window)
{
	Windows::iterator i;

	if (windows.size()) {
		for (i = windows.begin(); i != windows.end(); i++)
			if ((*i).window == window) return true;
	}

	return false;
}

bool WindowManager::Draw(void)
{
	Windows::iterator i;
	Curses::Window *window;

	if (redrawpending) {
		Curses::erase();
		Curses::noutrefresh();
		
		for (i = windows.begin(); i != windows.end(); i++) {
			(*i).window->Draw();
			/* this updates the virtual ncurses screen */
			window = (*i).window->GetWindow();
			window->touch();
			window->noutrefresh();
		}

		/* this copies to virtual ncurses screen to the physical screen */
		Curses::doupdate();

		redrawpending = false;
	}

	return false;
}

void WindowManager::Redraw(void)
{
	if (!redrawpending) {
		redrawpending = true;
		g_timeout_add(0, timeout_once_draw, this);
	}
}

void WindowManager::WindowRedraw(Widget& widget)
{
	Redraw();
}

bool WindowManager::Resize(void)
{
	struct winsize size;

	if (resizepending)
		resizepending = false;

	if (ioctl(fileno(stdout), TIOCGWINSZ, &size) >= 0) {
		Curses::resizeterm(size.ws_row, size.ws_col);
		// TODO next line needed?
		//wrefresh(curscr);
	}

	// save new screen size
	screenW = size.ws_col;
	screenH = size.ws_row;

	signal_resize();

	return false;
}

void WindowManager::EnableResizing(void)
{
	// register resize handler
	struct sigaction sig;
	sig.sa_handler = signal_handler;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = 0;
	sigaction(SIGWINCH, &sig, NULL);
}

void WindowManager::DisableResizing(void)
{
	// unregister resize handler
	struct sigaction sig;
	sig.sa_handler = SIG_DFL;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = 0;
	sigaction(SIGWINCH, &sig, NULL);
}

void WindowManager::ScreenResized(void)
{
	if (!resizepending) {
		resizepending = true;
		g_timeout_add(0, timeout_once_resize, this);
	}
}
