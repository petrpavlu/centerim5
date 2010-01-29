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

	// @todo is this a correct place where to do this?
	panel->Resize(win_w, win_h);

	Container::Resize(win_w - 2, win_h - 2);

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
		delete area;
	area = Curses::Window::newpad(win_h, win_w);

	/*
	if (area->w == NULL)
		{}//TODO throw an exception
		//actually, dont!
		//after a container resize, all widgets are updatearea()'d
		//which will probably result (unless resizing to bigger) in
		//area == null because no pad can be made
		*/
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
	realwindow = Curses::Window::newwin(bottom - top, right - left, top, left);
}

void Window::Draw(void)
{
	if (!realwindow)
		return;

	Container::Draw();

	/* copy the virtual window to a window, then display it
	 * on screen.
	 * */
	area->copyto(realwindow, copy_y, copy_x, 0, 0, copy_h, copy_w, 0);
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

Curses::Window *Window::GetSubPad(Widget *child, int nlines, int ncols, int begin_y, int begin_x)
{
	/* Return subpad inside the panel and shrink subpad if it would fall
	 * outside the panel. */

	// @todo review, maybe move to Container class instead..

	if (child == panel)
		return area->subpad(nlines, ncols, begin_y, begin_x);

	if (nlines > h - begin_y)
		nlines = h - begin_y;
	if (ncols > w - begin_x)
		ncols = w - begin_x;
	return area->subpad(nlines, ncols, begin_y + 1, begin_x + 1);
}
