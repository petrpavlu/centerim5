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

#include <panel.h>

#include <glib.h>

WindowManager* WindowManager::instance = NULL;

WindowManager* WindowManager::Instance(void)
{
	if (!instance) instance = new WindowManager();
	return instance;
}

WindowManager::WindowManager(void)
: focuswindow(NULL)
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
	WindowPair win;

	if (!HasWindow(window)) {
		win.first = window;
		win.second = window->signal_redraw.connect(sigc::mem_fun(this, &WindowManager::Redraw));
		windows.insert(windows.begin(), win);
	}

	FocusPanel();
	Redraw();
}

void WindowManager::Remove(Window *window)
{
	WindowPair win;
	Windows::iterator i;

	if (!window) return;

	i = FindWindow(window);

	if (i != windows.end()) {
		win = *i;

		win.second.disconnect();
		windows.erase(i);
	}

	FocusPanel();
	Redraw();
}

void WindowManager::FocusPanel(void)
{
	PANEL *pan;
	Window *win;

	pan = panel_below(NULL);
	if (!pan) {
		if (focuswindow) {
			focuswindow->TakeFocus();
			focuswindow = NULL;
			SetInputChild(NULL);
		}
		return;
	}

	win = (Window*)panel_userptr(pan);
	if (!win)
		; //TODO throw exception about foreign panel

	if (focuswindow != win) {
		if (focuswindow)
			focuswindow->TakeFocus();
		focuswindow = win;
		focuswindow->GiveFocus();
		SetInputChild(focuswindow);
	}
}

//TODO i doubt this function will ever be used
void WindowManager::Swap(Window* fst, Window* snd)
{
	Windows::iterator ifst, isnd;
	Window *wb;
	sigc::connection cb;

	if (fst == snd) return;

	ifst = FindWindow(fst);
	isnd = FindWindow(snd);

	if (ifst == windows.end() || isnd == windows.end())
		return;
	
	wb = (*ifst).first;
	cb = (*ifst).second;
	(*ifst).first = (*isnd).first;
	(*ifst).second = (*isnd).second;
	(*isnd).first = wb;
	(*isnd).second = cb;
}

WindowManager::Windows::iterator WindowManager::FindWindow(Window *window)
{
	Windows::iterator i;

	if (windows.size()) {
		for (i = windows.begin(); i != windows.end(); i++)
			if ((*i).first == window) break;
	}

	return i;
}

bool WindowManager::HasWindow(Window *window)
{
	Windows::iterator i;

	if (windows.size()) {
		for (i = windows.begin(); i != windows.end(); i++)
			if ((*i).first == window) return true;
	}

	return false;
}

void WindowManager::Draw(void)
{
	Windows::iterator i;

	for (i = windows.begin(); i != windows.end(); i++)
		(*i).first->Draw();


	update_panels();
	doupdate();
}

void WindowManager::Redraw(void)
{
	//TODO disconnect redraw events from actual drawing to reduce the number of
	//draws. This means: multiple redraw = one actual draw.
	//Glib::signal_timeout can be used for this.
	Draw();
}
