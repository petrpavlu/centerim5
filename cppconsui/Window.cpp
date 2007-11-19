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

#include "Window.h"

#include "Container.h"

#include "Curses.h"
#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <panel.h>

Window::Window(int x, int y, int w, int h, Border *border)
: Container(*this, 1, 1, w-2, h-2)
, realwindow(NULL)
, panel(NULL)
, win_x(x)
, win_y(y)
, win_w(w)
, win_h(h)
, border(border)
{
	//TODO just call moveresize
	if (win_w < 1) win_w = 1;
	if (win_h < 1) win_h = 1;

	UpdateArea();
	MakeRealWindow();

	if (border) {
		border->Resize(win_w, win_h);
	}
	Container::MoveResize(0, 0, win_w, win_h);

	Redraw();
}

Window::~Window()
{
	if (panel) {
		hide_panel(panel);
		del_panel(panel);
		delwin(realwindow);
	}
}

void Window::Move(int newx, int newy)
{
	if (newx == win_x && newy == win_y)
		return;

	win_x = newx;
	win_y = newy;

	MakeRealWindow();

	Redraw();
}

/* NOTE
 * subclasses should do something sensible with the container
 * widgets when resizing a window, see TextWindow for example
 * */
void Window::Resize(int neww, int newh)
{
	if (neww == win_w && newh == win_h)
		return;

	win_w = neww;
	win_h = newh;

	if (win_w < 1) win_w = 1;
	if (win_h < 1) win_h = 1;

	UpdateArea();
	MakeRealWindow();

	if (border) {
		border->Resize(win_w, win_h);
	}

	Container::Resize(win_w, win_h);

	Redraw();
}

void Window::MoveResize(int newx, int newy, int neww, int newh)
{
	//TODO this has an overhead of 1 Redraw()
	//we cant combine the functions  move and resize
	//here because then each subclass would have to
	//combine its implementations of move and redaw too
	//i dont think that is what we want
	//update: the WindowManager should only do screen updates
	//in its spare time. see the todo in WindowManager::Redraw()
	Move(newx, newy);
	Resize(neww, newh);
}

void Window::UpdateArea()
{
	if (area->w)
		delwin(area->w);

	area->w = newpad(win_h, win_w);

	if (area->w == NULL)
		;//TODO throw an exception
		//actually, dont!
		//after a container resize, all widgets are updatearea()'d
		//which will probably result (unless resizing to bigger) in
		//area == null because no pad can be made
}

void Window::SetBorder(Border *border)
{
	this->border = border;
	
	if (border) {
		border->Resize(win_w, win_h);
	}
}

Border* Window::GetBorder(void)
{
	return border;
}

/* create the `real' window (not a pad) and make sure its
 * dimensions do not exceed screen size */
void Window::MakeRealWindow(void)
{
	WINDOW *win;
	int left, right, top, bottom, maxx, maxy;

	getmaxyx(stdscr, maxy, maxx);

	left = (win_x < 0) ? 0 : win_x;
	top = (win_y < 0) ? 0 : win_y;
	right = (win_x + win_w >= maxx) ? maxx : win_x + win_w;
	bottom = (win_y + win_h >= maxy) ? maxy : win_y + win_h;

	copy_x = left - win_x;
	copy_y = top - win_y;
	copy_w = right - left - 1;
	copy_h = bottom - top - 1;

	/* this could fails if the window falls outside the visible
	 * area
	 * */
	win = newwin(bottom - top, right - left, top, left);

	if (!win) {
		if (panel) {
			del_panel(panel);
			panel = NULL;
		}
		delwin(realwindow);
		realwindow = NULL;

		/* we can't make a real window
		 * so the window is probably offscreen 
		 * */
		return;
	} else {

		if (panel) {
			replace_panel(panel, win);
			delwin(realwindow);
			realwindow = win;
		} else {
			delwin(realwindow);
			realwindow = win;
			panel = new_panel(realwindow);
			set_panel_userptr(panel, this);
			show_panel(panel);
		}
	}

	if (!panel)
		;//TODO throw an exception
}

void Window::Draw(void)
{
	if (!realwindow)
		return;

	if (border)
		border->Draw(area->w); //TODO draw the border
	
	Container::Draw();

	/* copy the virtual window to a window, then display it
	 * on screen.
	 * */
	copywin(area->w, realwindow, copy_y, copy_x, 0, 0, copy_h, copy_w, 0);
}

void Window::Show()
{
	//TODO emit signal to show panel
	//(while keeping stacking order)
}

void Window::Hide()
{
	//TODO emit signal to hide panel
	//(while keeping stacking order)
}
