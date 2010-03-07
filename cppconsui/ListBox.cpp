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

ListBox::ListBox(Widget& parent, int x, int y, int w, int h)
: AbstractListBox(parent, x, y, w, h)
{
	SetScrollSize(w, 0);
}

void ListBox::AddSeparator()
{
	AddWidget(*(new HorizontalLine(*this, 0, 0, Width())));
}

void ListBox::AddWidget(Widget& widget)
{
	int y = GetScrollHeight();
	widget.MoveResize(0, y, widget.Width(), widget.Height());
	SetScrollHeight(y + widget.Height());
	AbstractListBox::AddWidget(widget);
}

void ListBox::RemoveWidget(Widget& widget)
{
	AbstractListBox::RemoveWidget(widget);

	// recalculate height
	int y = 0;
	for (Children::iterator i = ChildrenBegin(); i != ChildrenEnd(); i++) {
		Widget *w = i->widget;
		w->MoveResize(0, y, w->Width(), w->Height());
		y += w->Height();
	}

	SetScrollHeight(y);
}
