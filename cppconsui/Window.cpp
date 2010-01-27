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

#define CONTEXT_WINDOW "window"

Window::Window(int x, int y, int w, int h, Border *border)
: Container(*this, 1, 1, w-2, h-2)
, win_x(x)
, win_y(y)
, win_w(w)
, win_h(h)
, realwindow(NULL)
, border(border)
{

	//TODO just call moveresize
	if (win_w < 1) win_w = 1;
	if (win_h < 1) win_h = 1;

	//TODO allow focus going to windows by default or not?
	can_focus = true;

	MakeRealWindow();
	UpdateArea();

	if (border) {
		border->Resize(win_w, win_h);
	}
	Container::MoveResize(0, 0, win_w, win_h);
	Redraw();
	DeclareBindables();
}

Window::~Window()
{
	Curses::delwin(realwindow);
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

	MakeRealWindow();
	UpdateArea();

	if (border) {
		border->Resize(win_w, win_h);
	}

	Container::Resize(win_w, win_h);

	Redraw();
}

void Window::MoveResize(int newx, int newy, int neww, int newh)
{
	/** @todo this has an overhead of 1 Redraw()
	 * we cant combine the functions  move and resize
	 * here because then each subclass would have to
	 * combine its implementations of move and redaw too
	 * i dont think that is what we want
	 * update: the WindowManager should only do screen updates
	 * in its spare time. See the todo in WindowManager::Redraw()
	 */
	Move(newx, newy);
	Resize(neww, newh);
}

void Window::UpdateArea()
{
	//LOG("/tmp/ua.log","Window::UpdateArea (%d,%d,%d,%d) parent: %x this: %x areaw: %x \n", x,y,w,h, this->parent, this, area->w);
	if (area)
		Curses::delwin(area);

	area = Curses::newpad(win_h, win_w);

	/*
	if (area->w == NULL)
		{}//TODO throw an exception
		//actually, dont!
		//after a container resize, all widgets are updatearea()'d
		//which will probably result (unless resizing to bigger) in
		//area == null because no pad can be made
		*/
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
	Curses::Window *win;
	int left, right, top, bottom, maxx, maxy;

	Curses::ngetmaxyx(NULL, &maxy, &maxx);

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
	win = Curses::newwin(bottom - top, right - left, top, left);

	Curses::delwin(realwindow);
	realwindow = win;
}

void Window::Draw(void)
{
	if (!realwindow)
		return;

	if (border)
		border->Draw(area); //TODO draw the border
	
	Container::Draw();

	/* copy the virtual window to a window, then display it
	 * on screen.
	 * */
	Curses::copywin(area, realwindow, copy_y, copy_x, 0, 0, copy_h, copy_w, 0);
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
