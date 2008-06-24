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

#include "Curses.h"

#include "Dialog.h"

#include "Panel.h"

Dialog::Dialog(int x, int y, int w, int h, Border *border)
: Window(x, y, w, h, border)
{
	AddWidgets();
}

Dialog::Dialog()
: Window(0, 0, 1, 1, NULL)
{
	//TODO set correct position.
	MoveResize(10, 10, 60, 12);

	AddWidgets();
}

Dialog::~Dialog()
{
}

void Dialog::AddWidgets(void)
{
	buttons = new HorizontalListBox(*this, 1, h-2, w-2, 1);
	seperator = new HorizontalLine(*this, 1, h-3, w-2);
	AddWidget(buttons);
	AddWidget(seperator);
	AddWidget(new Panel(*this, 0, 0, w, h, LineStyle::LineStyleDefault()));
}

void Dialog::Resize(int neww, int newh)
{
	/* Let parent's Resize() renew data structures (including
	 * the area's of child widgets which will thus be done
	 * twice)
	 * */
	Window::Resize(neww, newh);

	/* resize all our widgets, in this case its only one widget
	 * here, w and h are the size of the container, which is 
	 * what we want. in most cases you would need to recalculate
	 * widget sizes based on window and/or container size.
	 * */
	//browser->Resize(w-4, h-2);
}

void Dialog::AddButton(const gchar *text, sigc::slot<void> function)
{
	buttons->AddItem(text, function);
}
