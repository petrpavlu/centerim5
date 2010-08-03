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
 * HorizontalListBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "HorizontalListBox.h"

#include "VerticalLine.h"

HorizontalListBox::HorizontalListBox(int w, int h)
: AbstractListBox(w, h)
{
	SetScrollSize(0, h);
}

void HorizontalListBox::MoveResize(int newx, int newy, int neww, int newh)
{
	SetScrollHeight(newh);

	AbstractListBox::MoveResize(newx, newy, neww, newh);
}

void HorizontalListBox::Draw()
{
	if (!area) {
		// scrollpane will clear the scroll (real) area
		AbstractListBox::Draw();
		return;
	}

	area->erase();

	int x = 0;
	for (Children::iterator i = children.begin(); i != children.end(); i++) {
		Widget *widget = i->widget;
		if (widget->IsVisible()) {
			widget->MoveResize(x, 0, widget->Width(), widget->Height());
			widget->Draw();

			int w = widget->Width();
			w = w < 0 ? 1 : w;
			x += w;
		}
	}

	// make sure that currently focused widget is visible
	if (focus_child)
		MakeVisible(focus_child->Left(), focus_child->Top());

	AbstractListBox::Draw();
}

void HorizontalListBox::AppendSeparator()
{
	AppendWidget(*(new VerticalLine(-1)));
}

void HorizontalListBox::AppendWidget(Widget& widget)
{
	int w = widget.Width();
	w = w < 0 ? 1 : w;
	int sw = GetScrollWidth();
	SetScrollWidth(sw + w);
	AddWidget(widget, sw, 0);
}

void HorizontalListBox::RemoveWidget(Widget& widget)
{
	int w = widget.Width();
	w = w < 0 ? 1 : w;
	SetScrollWidth(GetScrollWidth() - w);

	AbstractListBox::RemoveWidget(widget);
}

Curses::Window *HorizontalListBox::GetSubPad(const Widget& child, int begin_x,
		int begin_y, int ncols, int nlines)
{
	// if width is set to negative number (autosize) then return width `1'
	if (ncols < 0)
		ncols = 1;

	return AbstractListBox::GetSubPad(child, begin_x, begin_y, ncols, nlines);
}

void HorizontalListBox::OnChildMoveResize(Widget& widget, Rect &oldsize, Rect &newsize)
{
	int old_width = oldsize.Width();
	int new_width = newsize.Width();
	if (old_width != new_width) {
		old_width = old_width < 0 ? 1 : old_width;
		new_width = new_width < 0 ? 1 : new_width;

		SetScrollWidth(GetScrollWidth() - old_width + new_width);
	}
}
