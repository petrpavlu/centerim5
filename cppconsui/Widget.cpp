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

/** @file Widget.cpp Widget class implementation
 * @ingroup cppconsui
 */

#include "ConsuiCurses.h"
//include "ConsuiLog.h"
#include "Widget.h"

#include <string>

Widget::Widget(Widget& parent, int x, int y, int w, int h)
: x(x)
, y(y)
, w(w)
, h(h)
, can_focus(false)
, has_focus(false)
, focus_child(NULL)
, parent(&parent)
, colorscheme(NULL)
{
	area = new Curses::Window;
	UpdateArea(); /// @todo It should be called whenever the parent decides, and not here in constructor.
	colorscheme = ColorScheme::ColorSchemeNormal();
}

Widget::~Widget()
{
	/// @todo should also delete @ref Widget::area
	Curses::delwin(area);
}

void Widget::Move(int newx, int newy)
{
	Point oldpos(x,y), newpos(newx, newy);

	x = newx;
	y = newy;

	UpdateArea();

	signal_move(this, oldpos, newpos);
}

void Widget::Resize(int neww, int newh)
{
	Rect oldsize(x, y, w, h), newsize(x, y, neww, newh);

	w = neww;
	h = newh;

	UpdateArea();

	signal_resize(this, oldsize, newsize);
}

void Widget::MoveResize(int newx, int newy, int neww, int newh)
{
	Rect oldsize(x, y, w, h), newsize(newx, newy, neww, newh);

	x = newx;
	y = newy;
	w = neww;
	h = newh;

	UpdateArea();

	signal_move(this, oldsize, newsize);
	signal_resize(this, oldsize, newsize);
}

void Widget::UpdateArea()
{
	//LOG("/tmp/ua.log",">Widget::UpdateArea (%d,%d,%d,%d) parent: %x this: %x areaw: %x \n", x,y,w,h, this->parent, this, area->w);	
	if (area)
		Curses::delwin(area);

	// @todo use GetSubPad?
	area = Curses::subpad(parent->area, h, w, y, x);

	//LOG("/tmp/ua.log","<Widget::UpdateArea (%d,%d,%d,%d) parent: %x this: %x areaw: %x \n", x,y,w,h, this->parent, this, area->w);	
	/*
	if (area == NULL)
		{}//TODO throw an exception
		//actually, dont!
		//after a container resize, all widgets are updatearea()'d
		//which will probably result (unless resizing to bigger) in
		//area == null because no pad can be made
		*/
}

void Widget::Draw(void)
{
}

void Widget::Redraw(void)
{
	signal_redraw(this);
}

bool Widget::SetFocusChild(Widget* child)
{
	g_assert(child != NULL);

	if (focus_child == child)
		/* The focus child is already correct. */
		return true;

	if (focus_child != NULL) {
		/* The currently focussed widget is in a different branch
		 * of the widget tree, so unfocus that widget first.
		 * */
		if (!focus_child->StealFocus())
			return false;
	}

	if (parent == this) {
		/* Current widget is a window. */
		/// @todo window should try to become topmost.
		focus_child = child;
		SetInputChild(child);
		return true;
	}

	if (parent->SetFocusChild(this)) {
		/* Only if we can grab the focus all the way up the widget
		 * tree do we set the focus child.
		 * */
		focus_child = child;
		SetInputChild(child);
		return true;
	}

	return false;
}

bool Widget::StealFocus(void)
{
	/* If has_focus is true, then this is the widget with focus. */
	if (has_focus) {
		has_focus = false;
		signal_focus(this, false);
		return true;
	}

	if (!focus_child) {
		/* Apparently there is no widget with focus because
		 * the chain ends here. */
		return true;
	}

	/* First propagate focus stealing to the widget with focus.
	 * If theft is successful, then unset focus_child. */
	if (focus_child->StealFocus()) {
		focus_child = NULL;
		SetInputChild(NULL);
		return true;
	}

	return false;
}

/// @todo move to window and use getfocuswidget??
void Widget::RestoreFocus(void)
{
	if (focus_child == NULL) {
		if (can_focus) {
			has_focus = true;
			Redraw();
		}
	} else {
		focus_child->RestoreFocus();
	}
}

Widget* Widget::GetFocusWidget(void)
{
	if (focus_child == NULL) {
		if (can_focus) {
			return this;
		} else {
			return NULL;
		}
	} else {
		return focus_child->GetFocusWidget();
	}

}

bool Widget::GrabFocus(void)
{
	if (can_focus && parent != NULL && parent->SetFocusChild(this)) {
		//TODO only set if window has focus.
		has_focus = true;
		signal_focus(this, true);
		Redraw();
		return true;
	}

	return false;
}

void Widget::UngrabFocus(void)
{
	has_focus = false;
}

void Widget::MoveFocus(FocusDirection direction)
{
	/* Make sure we always start at the root
	 * of the widget tree. */
	if (parent) {
		parent->MoveFocus(direction);
	}
}

Curses::Window *Widget::GetSubPad(int nlines, int ncols, int begin_y, int begin_x)
{
	return Curses::subpad(area, nlines, ncols, begin_y, begin_x);
}
