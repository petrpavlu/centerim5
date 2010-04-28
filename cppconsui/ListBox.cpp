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

void ListBox::AppendSeparator()
{
	AppendWidget(*(new HorizontalLine(Width())));
}

void ListBox::AppendWidget(Widget& widget)
{
	int h = GetScrollHeight();
	SetScrollHeight(h + widget.Height());
	AddWidget(widget, 0, h);
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
