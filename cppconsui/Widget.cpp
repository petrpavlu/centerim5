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

Widget::Widget(int w, int h)
: xpos(0)
, ypos(0)
, width(w)
, height(h)
, can_focus(false)
, has_focus(false)
, focus_child(NULL)
, area(NULL)
, parent(NULL)
, color_scheme(NULL)
{
}

Widget::~Widget()
{
	if (area)
		delete area;
	if (color_scheme)
		g_free(color_scheme);
}

void Widget::MoveResize(int newx, int newy, int neww, int newh)
{
	Rect oldsize(xpos, ypos, width, height), newsize(newx, newy, neww, newh);

	xpos = newx;
	ypos = newy;
	width = neww;
	height = newh;

	if (parent)
		UpdateArea();

	signal_moveresize(*this, oldsize, newsize);
}

void Widget::MoveResize()
{
	MoveResize(xpos, ypos, width, height);
}

void Widget::UpdateArea()
{
	g_assert(parent);

	if (area)
		delete area;
	area = parent->GetSubPad(*this, xpos, ypos, width, height);
}

bool Widget::SetFocusChild(Widget &child)
{
	// focus cannot be set for widget without a parent
	if (!parent)
		return false;

	if (focus_child == &child)
		/* The focus child is already correct. */
		return true;

	if (focus_child != NULL) {
		/* The currently focussed widget is in a different branch
		 * of the widget tree, so unfocus that widget first.
		 * */
		if (!focus_child->StealFocus())
			return false;
	}

	if (parent->SetFocusChild(*this)) {
		/* Only if we can grab the focus all the way up the widget
		 * tree do we set the focus child.
		 * */
		focus_child = &child;
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
		signal_focus(*this, false);
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
		ClearInputChild();
		return true;
	}

	return false;
}

/// @todo move to window and use getfocuswidget??
void Widget::RestoreFocus()
{
	if (!focus_child) {
		if (can_focus) {
			has_focus = true;
			signal_redraw(*this);
		}
	} else {
		focus_child->RestoreFocus();
	}
}

Widget* Widget::GetFocusWidget()
{
	if (!focus_child) {
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
	if (can_focus && parent && parent->SetFocusChild(*this)) {
		//TODO only set if window has focus.
		has_focus = true;
		signal_focus(*this, true);
		signal_redraw(*this);
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

void Widget::SetParent(Widget& parent)
{
	// we don't support parent change
	g_assert(!this->parent);

	this->parent = &parent;
	UpdateArea();
}

Curses::Window *Widget::GetSubPad(const Widget& child, int begin_x, int begin_y, int ncols, int nlines)
{
	if (!area)
		return NULL;

	int realw = area->getmaxx();
	int realh = area->getmaxy();

	/* Extend requested subpad to whole parent area or shrink requested area
	 * if necessary. */
	if (nlines < 0 || nlines > realh - begin_y)
		nlines = realh - begin_y;

	if (ncols < 0 || ncols > realw - begin_x)
		ncols = realw - begin_x;

	return area->subpad(begin_x, begin_y, ncols, nlines);
}

void Widget::SetColorScheme(const char *scheme)
{
	if (color_scheme)
		g_free(color_scheme);

	if (scheme)
		color_scheme = g_strdup(scheme);
	else
		color_scheme = NULL;
}

const char *Widget::GetColorScheme()
{
	if (color_scheme)
		return color_scheme;
	else if (parent)
		return parent->GetColorScheme();

	return NULL;
}
