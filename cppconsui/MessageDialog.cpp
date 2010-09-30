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
 * MessageDialog class implementation.
 *
 * @ingroup cppconsui
 */

#include "MessageDialog.h"

#include "gettext.h"

MessageDialog::MessageDialog(const gchar *title, const gchar *text)
: Dialog(title)
{
	AddButton(_(OK_BUTTON_TEXT), Dialog::RESPONSE_OK);

	label = new Label(AUTOSIZE, AUTOSIZE, text);
	layout->InsertWidget(0, *label);
}
