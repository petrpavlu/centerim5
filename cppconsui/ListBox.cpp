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

#include "ListBox.h"
#include "Keys.h"
#include "HorizontalLine.h"

#include "ScrollPane.h"

ListBox::ListBox(Widget& parent, int x, int y, int w, int h)
: AbstractListBox(parent, x, y, w, h)
, movingwidget(false)
{
	SetScrollSize(w, 0);
}

ListBox::~ListBox()
{
}

void ListBox::AddSeparator()
{
	HorizontalLine *line;

	line = new HorizontalLine(*this, 0, 0, Width());
	AddWidget(line);
}

void ListBox::AddWidget(Widget *widget)
{
	int y;

	y = GetScrollHeight();
	
	movingwidget = true;
	widget->MoveResize(0, y, widget->Width(), widget->Height());
	movingwidget = false;
	SetScrollHeight(y + widget->Height());
	AbstractListBox::AddWidget(widget);
}

void ListBox::RemoveWidget(Widget *widget)
{
	Children::iterator i;
	Widget *w = NULL;
	int y;

	g_return_if_fail(widget != NULL);

	AbstractListBox::RemoveWidget(widget);

	y = 0;
	for (i = ChildrenBegin(); i != ChildrenEnd(); i++) {
		w = (*i).widget;
		w->MoveResize(0, y, w->Width(), w->Height());
		y += w->Height();
	}

	SetScrollHeight(y);
}
