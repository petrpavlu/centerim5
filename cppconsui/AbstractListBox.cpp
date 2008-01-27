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
#include "Keys.h"

#include "ScrollPane.h"

AbstractListBox::AbstractListBox(Widget& parent, int x, int y, int w, int h)
: ScrollPane(parent, x, y, w, h, w, h)
, maxheight(0)
, maxwidth(0)
{
	const gchar *context = "listbox";
	DeclareBindable(context, "focus-previous", sigc::mem_fun(this, &AbstractListBox::ActionFocusPrevious),
		_("Focusses the previous item in the list"), InputProcessor::Bindable_Normal);
	DeclareBindable(context, "focus-next", sigc::mem_fun(this, &AbstractListBox::ActionFocusNext),
		_("Focusses the next item in the list"), InputProcessor::Bindable_Normal);
}

AbstractListBox::~AbstractListBox()
{
}

void AbstractListBox::AddWidget(Widget *widget)
{
	if (widget->Height() > maxheight)
		maxheight = widget->Height();
	
	if (widget->Height() > maxwidth)
		maxwidth = widget->Height();

	Container::AddWidget(widget);
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
