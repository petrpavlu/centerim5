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

#include "ConsuiCurses.h"

#include "InputDialog.h"

#include <glibmm/ustring.h>

InputDialog::InputDialog(const gchar* text, const gchar* defaultvalue)
: Dialog()
, label(NULL)
, entry(NULL)
, seperator(NULL)
{
	//TODO add a way such that each dialog uses the same strings
	//for default buttons
	AddButton(_("Ok"), InputDialog::ResponseOK);

	label = new Label(*this, 0, 0, text);
	seperator = new HorizontalLine(*this, 0, 1, w);
	entry = new TextEntry(*this, 0, 2, w, h - 4, defaultvalue);
	AddWidget(label);
	AddWidget(seperator);
	AddWidget(entry);

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

const char* InputDialog::GetText(void)
{
	return entry->GetText();
}
