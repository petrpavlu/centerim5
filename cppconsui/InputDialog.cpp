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

#include "InputDialog.h"

#include <glibmm/ustring.h>

InputDialog::InputDialog(const gchar* text, const gchar* defaultvalue)
: Dialog()
, label(NULL)
, entry(NULL)
, seperator(NULL)
{
	AddButton(_("Ok"), sigc::mem_fun(this, &Window::Close));

	label = new Label(*this, 1, 1, text);
	seperator = new HorizontalLine(*this, 1, 2, w-2);
	entry = new TextEntry(*this, 1, 3, w-2, h-5, defaultvalue);
	AddWidget(label);
	AddWidget(seperator);
	AddWidget(entry);

	SetInputChild(entry);
	entry->GrabFocus();
}

InputDialog::~InputDialog()
{
}

void InputDialog::Resize(int neww, int newh)
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
