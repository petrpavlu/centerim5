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

ListBox::ListBox(int w, int h)
: AbstractListBox(w, h)
, children_height(0)
, autosize_children(0)
, autosize_height(0)
{
}

void ListBox::UpdateArea()
{
	AbstractListBox::UpdateArea();

	// set virtual scroll area width
	if (scrollarea)
		SetScrollWidth(scrollarea->getmaxx());
	UpdateScrollHeight();
}

void ListBox::Draw()
{
	if (!area) {
		// scrollpane will clear the scroll (real) area
		AbstractListBox::Draw();
		return;
	}

	autosize_height = 1;
	int autosize_height_extra = 0;
	int realh = area->getmaxy();
	if (autosize_children && children_height < realh) {
		int space = realh - (children_height - autosize_children);
		autosize_height = space / autosize_children;
		autosize_height_extra = space % autosize_children;
	}
	autosize_extra.clear();

	int y = 0;
	for (Children::iterator i = children.begin(); i != children.end(); i++) {
		Widget *widget = i->widget;
		if (widget->IsVisible()) {
			int h = widget->Height();
			if (h == AUTOSIZE) {
				h = autosize_height;
				if (autosize_height_extra) {
					autosize_extra.insert(widget);
					autosize_height_extra--;
					h++;
				}
			}

			widget->MoveResize(0, y, widget->Width(), widget->Height());
			y += h;
		}
	}

	// make sure that currently focused widget is visible
	if (focus_child)
		MakeVisible(focus_child->Left(), focus_child->Top());

	AbstractListBox::Draw();
}

HorizontalLine *ListBox::InsertSeparator(size_t pos)
{
	HorizontalLine *l = new HorizontalLine(AUTOSIZE);
	InsertWidget(pos, *l);
	return l;
}

HorizontalLine *ListBox::AppendSeparator()
{
	HorizontalLine *l = new HorizontalLine(AUTOSIZE);
	AppendWidget(*l);
	return l;
}

void ListBox::InsertWidget(size_t pos, Widget& widget)
{
	if (widget.IsVisible()) {
		int h = widget.Height();
		if (h == AUTOSIZE) {
			h = 1;
			autosize_children++;
		}
		children_height += h;
		UpdateScrollHeight();
	}

	// note: widget is moved to a correct position in Draw() method
	ScrollPane::InsertWidget(pos, widget, 0, 0);
}

void ListBox::AppendWidget(Widget& widget)
{
	InsertWidget(children.size(), widget);
}

Curses::Window *ListBox::GetSubPad(const Widget& child, int begin_x,
		int begin_y, int ncols, int nlines)
{
	// autosize
	if (nlines == AUTOSIZE) {
		nlines = autosize_height;
		if (autosize_extra.find(&child) != autosize_extra.end())
			nlines++;
	}

	return AbstractListBox::GetSubPad(child, begin_x, begin_y, ncols, nlines);
}

void ListBox::OnChildMoveResize(Widget& widget, Rect& oldsize, Rect& newsize)
{
	int old_height = oldsize.Height();
	int new_height = newsize.Height();
	if (old_height != new_height) {
		if (old_height == AUTOSIZE) {
			old_height = 1;
			autosize_children--;
		}
		if (new_height == AUTOSIZE) {
			new_height = 1;
			autosize_children++;
		}
		children_height += new_height - old_height;
		UpdateScrollHeight();
	}
}

void ListBox::OnChildVisible(Widget& widget, bool visible)
{
	// the widget is being hidden or deleted

	int height = widget.Height();

	int sign = 1;
	if (!visible)
		sign = -1;

	if (height == AUTOSIZE) {
		autosize_children += sign;
		height = 1;
	}

	children_height += sign * height;
	UpdateScrollHeight();
}

void ListBox::UpdateScrollHeight()
{
	int realh = 0;
	if (scrollarea)
		realh = scrollarea->getmaxy();

	SetScrollHeight(MAX(realh, children_height));
}
