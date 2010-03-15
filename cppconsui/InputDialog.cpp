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

#include "InputDialog.h"

#include "ConsuiCurses.h"

#include "gettext.h"

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
	seperator = new HorizontalLine(*this, 0, 1, width);
	entry = new TextEntry(*this, 0, 2, width, height - 4, defaultvalue);
	AddWidget(*label);
	AddWidget(*seperator);
	AddWidget(*entry);

	entry->GrabFocus();
}

InputDialog::~InputDialog()
{
}

const char* InputDialog::GetText(void)
{
	return entry->GetText();
}
