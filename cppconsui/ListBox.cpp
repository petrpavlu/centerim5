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
 * ListBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "ListBox.h"

#include "HorizontalLine.h"

ListBox::ListBox(int w, int h)
: AbstractListBox(w, h)
{
	SetScrollSize(w, 0);
}

void ListBox::MoveResize(int newx, int newy, int neww, int newh)
{
	SetScrollWidth(neww);

	AbstractListBox::MoveResize(newx, newy, neww, newh);
}

void ListBox::Draw()
{
	if (!area) {
		// scrollpane will clear the scroll (real) area
		AbstractListBox::Draw();
		return;
	}

	area->erase();

	int y = 0;
	for (Children::iterator i = children.begin(); i != children.end(); i++) {
		Widget *widget = i->widget;
		if (widget->IsVisible()) {
			widget->MoveResize(0, y, widget->Width(), widget->Height());
			widget->Draw();

			int h = widget->Height();
			h = h < 0 ? 1 : h;
			y += h;
		}
	}

	// make sure that currently focused widget is visible
	if (focus_child)
		MakeVisible(focus_child->Left(), focus_child->Top());

	AbstractListBox::Draw();
}

void ListBox::AppendSeparator()
{
	AppendWidget(*(new HorizontalLine(-1)));
}

void ListBox::AppendWidget(Widget& widget)
{
	int h = widget.Height();
	h = h < 0 ? 1 : h;
	int sh = GetScrollHeight();
	SetScrollHeight(sh + h);
	AddWidget(widget, 0, sh);
}

void ListBox::RemoveWidget(Widget& widget)
{
	int h = widget.Height();
	h = h < 0 ? 1 : h;
	SetScrollHeight(GetScrollHeight() - h);

	AbstractListBox::RemoveWidget(widget);
}

Curses::Window *ListBox::GetSubPad(const Widget& child, int begin_x,
		int begin_y, int ncols, int nlines)
{
	// if height is set to negative number (autosize) then return height `1'
	if (nlines < 0)
		nlines = 1;

	return AbstractListBox::GetSubPad(child, begin_x, begin_y, ncols, nlines);
}

void ListBox::OnChildMoveResize(Widget& widget, Rect &oldsize, Rect &newsize)
{
	int old_height = oldsize.Height();
	int new_height = newsize.Height();
	if (old_height != new_height) {
		old_height = old_height < 0 ? 1 : old_height;
		new_height = new_height < 0 ? 1 : new_height;

		SetScrollHeight(GetScrollHeight() - old_height + new_height);
	}
}
