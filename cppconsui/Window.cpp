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
/** @file Container.cpp Container class implementation
 * @ingroup cppconsui
 */

#include "Window.h"

#include "WindowManager.h"
#include "Keys.h"
#include "Container.h"
#include "CppConsUIInternal.h"

#define CONTEXT_WINDOW "window"

Window::Window(int x_, int y_, int w_, int h_, LineStyle::Type ltype)
: Container(*this, 1, 1, w_ - 2, h_ - 2)
, win_x(x_)
, win_y(y_)
, win_w(w_)
, win_h(h_)
, realwindow(NULL)
{
	//TODO just call moveresize
	if (win_w < 2) win_w = 2;
	if (win_h < 2) win_h = 2;
	//Container::MoveResize(0, 0, win_w - 2, win_h - 2);

	//TODO allow focus going to windows by default or not?
	can_focus = true;

	MakeRealWindow();
	UpdateArea();

	panel = new Panel(*this, 0, 0, win_w, win_h, ltype);
	AddWidget(panel);

	Redraw();
	DeclareBindables();
}

Window::~Window()
{
	if (realwindow)
		delete realwindow;
}

void Window::DeclareBindables()
{
	DeclareBindable(CONTEXT_WINDOW, "close-window", sigc::mem_fun(this, &Window::Close),
					InputProcessor::Bindable_Normal);
}

DEFINE_SIG_REGISTERKEYS(Window, RegisterKeys);
bool Window::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_WINDOW, "close-window", _("Close the menu"), Keys::Instance()->Key_esc());
	return true;
}

void Window::Close(void)
{
	signal_close(this);
	WindowManager::Instance()->CloseWindow(this);
}

/* NOTE
 * subclasses should do something sensible with the container
 * widgets when resizing a window, see TextWindow for example
 * */
void Window::MoveResize(int newx, int newy, int neww, int newh)
{
	if (newx == win_x && newy == win_y && neww == win_w && newh == win_h)
		return;

	win_x = newx;
	win_y = newy;
	win_w = neww;
	win_h = newh;

	if (win_w < 1) win_w = 1;
	if (win_h < 1) win_h = 1;

	MakeRealWindow();
	UpdateArea();

	// @todo is this a correct place where to do this?
	panel->MoveResize(0, 0, win_w, win_h);

	Container::MoveResize(1, 1, win_w - 2, win_h - 2);

	Redraw();
}

void Window::UpdateArea()
{
	if (area)
		delete area;
	area = Curses::Window::newpad(win_w, win_h);
}

/* create the `real' window (not a pad) and make sure its
 * dimensions do not exceed screen size */
void Window::MakeRealWindow(void)
{
	int left, right, top, bottom, maxx, maxy;

	maxx = Curses::getmaxx();
	maxy = Curses::getmaxy();

	left = (win_x < 0) ? 0 : win_x;
	top = (win_y < 0) ? 0 : win_y;
	right = (win_x + win_w >= maxx) ? maxx : win_x + win_w;
	bottom = (win_y + win_h >= maxy) ? maxy : win_y + win_h;

	copy_x = left - win_x;
	copy_y = top - win_y;
	copy_w = right - left - 1;
	copy_h = bottom - top - 1;

	/* this could fail if the window falls outside the visible
	 * area
	 * */
	if (realwindow)
		delete realwindow;
	realwindow = Curses::Window::newwin(left, top, right - left, bottom - top);
}

void Window::Draw(void)
{
	if (!area || !realwindow)
		return;

	Container::Draw();

	/* copy the virtual window to a window, then display it
	 * on screen.
	 * */
	area->copyto(realwindow, copy_x, copy_y, 0, 0, copy_w, copy_h, 0);
}

void Window::Show()
{
	//TODO emit signal to show window
	//(while keeping stacking order)
}

void Window::Hide()
{
	//TODO emit signal to hide window
	//(while keeping stacking order)
}

void Window::ScreenResized()
{
	// TODO: handle resize/reposition of child widgets/windows
}

void Window::SetBorderStyle(LineStyle::Type ltype)
{
	panel->SetBorderStyle(ltype);
}

LineStyle::Type Window::GetBorderStyle()
{
	return panel->GetBorderStyle();
}

Curses::Window *Window::GetSubPad(const Widget &child, int begin_x, int begin_y, int ncols, int nlines)
{
	if (!area)
		return NULL;

	// handle panel child specially
	if (&child == panel)
		return area->subpad(begin_x, begin_y, ncols, nlines);

	int realw = area->getmaxx() - 2;
	int realh = area->getmaxy() - 2;

	/* Extend requested subpad to whole panel area or shrink requested area if
	 * necessary. */
	if (nlines == -1 || nlines > realh - begin_y)
		nlines = realh - begin_y;

	if (ncols == -1 || ncols > realw - begin_x)
		ncols = realw - begin_x;

	// add `+1' offset to normal childs so they can not overwrite the panel
	return area->subpad(begin_x + 1, begin_y + 1, ncols, nlines);
}
