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

#include "HorizontalListBox.h"
#include "VerticalLine.h"
#include "Keys.h"

#include "ScrollPane.h"

HorizontalListBox::HorizontalListBox(Widget& parent, int x, int y, int w, int h)
: AbstractListBox(parent, x, y, w, h)
, movingwidget(false)
{
	SetScrollSize(0, h);
}

HorizontalListBox::~HorizontalListBox()
{
}

void HorizontalListBox::AddSeperator()
{
	VerticalLine *line;

	line = new VerticalLine(*this, 0, 0, 1);
	AddWidget(line);
}

void HorizontalListBox::AddWidget(Widget *widget)
{
	int x;

	x = GetScrollWidth();
	
	movingwidget = true;
	widget->Move(x, 0);
	movingwidget = false;
	SetScrollHeight(x + widget->Width());
	AbstractListBox::AddWidget(widget);
}

void HorizontalListBox::RemoveWidget(Widget *widget)
{
	Children::iterator i;
	Widget *w = NULL;
	int x;

	g_return_if_fail(widget != NULL);

	AbstractListBox::RemoveWidget(widget);

	x = 0;
	for (i = ChildrenBegin(); i != ChildrenEnd(); i++) {
		w = (*i).first;
		widget->Move(x, 0);
		x += widget->Width();
	}

	SetScrollWidth(x);
}
