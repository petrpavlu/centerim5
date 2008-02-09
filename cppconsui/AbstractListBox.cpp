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

#include "AbstractListBox.h"
#include "HorizontalLine.h"
#include "Label.h"
#include "Keys.h"

#include "ScrollPane.h"

AbstractListBox::AbstractListBox(Widget& parent, int x, int y, int w, int h)
: ScrollPane(parent, x, y, w, h, 0, 0)
, maxheight(0)
, maxwidth(0)
{
}

AbstractListBox::~AbstractListBox()
{
}

void AbstractListBox::AddItem(const char *text, sigc::slot<void> function)
{
	Label *label;

	label = new Label(*this, 0, 0, text);
	label->SetCanFocus(true);
	AddWidget(label);
}

void AbstractListBox::AddWidget(Widget *widget)
{
	if (widget->Height() > maxheight)
		maxheight = widget->Height();
	
	if (widget->Width() > maxwidth)
		maxwidth = widget->Width();

	ScrollPane::AddWidget(widget);
}

void AbstractListBox::RemoveWidget(Widget *widget)
{
	Children::iterator i;
	Widget *w = NULL;

	g_return_if_fail(widget != NULL);

	ScrollPane::RemoveWidget(widget);

	maxwidth = maxheight = 0;
	for (i = ChildrenBegin(); i != ChildrenEnd(); i++) {
		w = (*i).first;

		if (w->Height() > maxheight) maxheight = w->Height();
		if (w->Height() > maxwidth) maxwidth = w->Height();
	}
}
