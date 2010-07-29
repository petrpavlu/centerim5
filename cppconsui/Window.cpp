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

/**
 * @file
 * Window class implementation.
 *
 * @ingroup cppconsui
 */

#include "Window.h"

#include "CoreManager.h"
#include "Keys.h"

#include "gettext.h"

#define CONTEXT_WINDOW "window"

Window::Window(int x, int y, int w, int h, LineStyle::Type ltype)
: Container(w < 2 ? 0 : w - 2, h < 2 ? 0 : h - 2)
, win_x(x)
, win_y(y)
, win_w(w)
, win_h(h)
, realwindow(NULL)
{
	MakeRealWindow();
	UpdateArea();

	panel = new Panel(win_w, win_h, ltype);
	AddWidget(*panel, 0, 0);

	signal_redraw(*this);
	DeclareBindables();
}

Window::~Window()
{
	if (realwindow)
		delete realwindow;
}

void Window::DeclareBindables()
{
	DeclareBindable(CONTEXT_WINDOW, "close-window",
			sigc::mem_fun(this, &Window::ActionClose),
			InputProcessor::Bindable_Normal);
}

DEFINE_SIG_REGISTERKEYS(Window, RegisterKeys);
bool Window::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_WINDOW, "close-window",
			_("Close the window."),
			Keys::SymbolTermKey(TERMKEY_SYM_ESCAPE));
	return true;
}

void Window::MoveResize(int newx, int newy, int neww, int newh)
{
	win_x = newx;
	win_y = newy;
	win_w = neww;
	win_h = newh;

	MakeRealWindow();
	UpdateArea();

	panel->MoveResize(0, 0, win_w, win_h);

	Container::MoveResize(1, 1, win_w < 2 ? 0 : win_w - 2,
			win_h < 2 ? 0 : win_h - 2);
}

void Window::UpdateArea()
{
	if (area)
		delete area;
	area = Curses::Window::newpad(win_w, win_h);
	signal_redraw(*this);
}

void Window::Draw()
{
	if (!area || !realwindow)
		return;

	Container::Draw();

	// copy the virtual window to a window, then display it on screen
	area->copyto(realwindow, 0, 0, 0, 0, copy_w, copy_h, 0);

	// update virtual ncurses screen
	realwindow->touch();
	realwindow->noutrefresh();
}

bool Window::SetFocusChild(Widget& child)
{
	if (focus_child) {
		/* The currently focused widget is in a different branch of the widget
		 * tree, so unfocus that widget first.*/
		focus_child->CleanFocus();
	}

	focus_child = &child;
	SetInputChild(child);

	if (COREMANAGER->HasWindow(*this) && COREMANAGER->GetTopWindow() != this)
		return false;

	return true;
}

bool Window::IsWidgetVisible(const Widget& child) const
{
	return true;
}

Curses::Window *Window::GetSubPad(const Widget &child, int begin_x,
		int begin_y, int ncols, int nlines)
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
	if (nlines < 0 || nlines > realh - begin_y)
		nlines = realh - begin_y;

	if (ncols < 0 || ncols > realw - begin_x)
		ncols = realw - begin_x;

	// add `+1' offset to normal childs so they can not overwrite the panel
	return area->subpad(begin_x + 1, begin_y + 1, ncols, nlines);
}

void Window::Show()
{
	COREMANAGER->AddWindow(*this);
	signal_show(*this);
}

void Window::Hide()
{
	COREMANAGER->RemoveWindow(*this);
	signal_hide(*this);
}

void Window::Close()
{
	COREMANAGER->RemoveWindow(*this);

	delete this;
}

void Window::ScreenResized()
{
}

void Window::SetBorderStyle(LineStyle::Type ltype)
{
	panel->SetBorderStyle(ltype);
}

LineStyle::Type Window::GetBorderStyle() const
{
	return panel->GetBorderStyle();
}

void Window::MakeRealWindow()
{
	int maxx = Curses::getmaxx();
	int maxy = Curses::getmaxy();

	int left = win_x < 0 ? 0 : win_x;
	int top = win_y < 0 ? 0 : win_y;
	int right = win_x + win_w >= maxx ? maxx : win_x + win_w;
	int bottom = win_y + win_h >= maxy ? maxy : win_y + win_h;

	copy_w = right - left - 1;
	copy_h = bottom - top - 1;

	if (realwindow)
		delete realwindow;
	// this could fail if the window falls outside the visible area
	realwindow = Curses::Window::newwin(left, top, right - left, bottom - top);
}

void Window::ActionClose()
{
	signal_close(*this);
	Close();
}
