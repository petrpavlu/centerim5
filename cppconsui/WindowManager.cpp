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

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <glib.h>

WindowManager* WindowManager::instance = NULL;

WindowManager* WindowManager::Instance(void)
{
	if (!instance) instance = new WindowManager();
	return instance;
}

WindowManager::WindowManager(void)
: defaultwindow(NULL)
{
	defaultwindow = initscr();

	if (!defaultwindow)
		;//TODO throw an exception that we cant init curses

	AddCombo(Keys::Instance()->Key_ctrl_l() /* ^L */, sigc::mem_fun(this, &WindowManager::Draw), true);
}

WindowManager::~WindowManager(void)
{
	if (endwin() == ERR)
		;//TODO throw an exeption
}

void WindowManager::Delete(void)
{
	if (instance) {
		delete instance;
		instance = NULL;
	}
}

void WindowManager::Add(Window *window)
{
	WindowInfo info;

	if (!HasWindow(window)) {
		info.window = window;
		info.redraw = window->signal_redraw.connect(sigc::mem_fun(this, &WindowManager::Redraw));
		windows.insert(windows.begin(), info);
	}

	FocusPanel();
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
	windows.erase(i);

	werase(info.window->GetWindow());
	wnoutrefresh(info.window->GetWindow());

	FocusPanel();
	Redraw();
}

void WindowManager::FocusPanel(void)
{
	Window *win, *focus = NULL;
	InputProcessor *inputchild;

	inputchild = GetInputChild();

	if ((focus = dynamic_cast<Window*>(inputchild)) == NULL) {
		; //TODO error about invalid input child
	}

	/* Take the focus from the old window with the focus */
	if (focus) {
		focus->TakeFocus();
		SetInputChild(NULL);
	}

	/* Check if there are any windows left */
	if (windows.size()) {
		win = windows.front().window;
	} else {
		win = NULL;
	}

	/* Give the focus to the top window if there is one */
	if (win) {
		SetInputChild(win);
		win->GiveFocus();
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

void WindowManager::Draw(void)
{
	Windows::iterator i;

	for (i = windows.begin(); i != windows.end(); i++) {
		(*i).window->Draw();
		/* this updates the virtual ncurses screen */
		wnoutrefresh((*i).window->GetWindow());
	}

	/* this copies to virtual ncurses screen to the physical screen */
	doupdate();
}

void WindowManager::Redraw(void)
{
	//TODO disconnect redraw events from actual drawing to reduce the number of
	//draws. This means: multiple redraw = one actual draw.
	//Glib::signal_timeout can be used for this.
	Draw();
}
