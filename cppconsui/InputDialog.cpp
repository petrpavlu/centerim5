/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 by CenterIM developers
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
 */

/**
 * @file
 * InputDialog class implementation.
 *
 * @ingroup cppconsui
 */

#include "InputDialog.h"

#include "gettext.h"

namespace CppConsUI {

InputDialog::InputDialog(const char *title, const char *defaultvalue)
  : AbstractDialog(title)
{
  addButton(OK_BUTTON_TEXT, RESPONSE_OK);

  entry = new TextEntry(AUTOSIZE, AUTOSIZE, defaultvalue);
  layout->insertWidget(0, *entry);

  entry->grabFocus();
}

void InputDialog::emitResponse(ResponseType response)
{
  signal_response(*this, response);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
