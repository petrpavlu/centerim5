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

HorizontalListBox::HorizontalListBox(int x, int y, int w, int h)
: AbstractListBox(x, y, w, h)
{
	SetScrollSize(0, h);
}

void HorizontalListBox::AddSeparator()
{
	AddWidget(*(new VerticalLine(0, 0, 1)));
}

void HorizontalListBox::AddWidget(Widget& widget)
{
	int x = GetScrollWidth();
	widget.MoveResize(x, 0, widget.Width(), widget.Height());
	SetScrollWidth(x + widget.Width());
	AbstractListBox::AddWidget(widget);
}

void HorizontalListBox::RemoveWidget(Widget& widget)
{
	AbstractListBox::RemoveWidget(widget);

	int x = 0;
	for (Children::iterator i = ChildrenBegin(); i != ChildrenEnd(); i++) {
		Widget *w = i->widget;
		w->MoveResize(x, 0, w->Width(), w->Height());
		x += w->Width();
	}

	SetScrollWidth(x);
}
