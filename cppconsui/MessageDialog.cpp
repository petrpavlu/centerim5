/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2013 by CenterIM developers
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
 * MessageDialog class implementation.
 *
 * @ingroup cppconsui
 */

#include "MessageDialog.h"

#include "gettext.h"

namespace CppConsUI
{

MessageDialog::MessageDialog(const char *title, const char *text)
: AbstractDialog(title)
{
  addButton(OK_BUTTON_TEXT, RESPONSE_OK);

  // never give focus to the textview
  buttons->setFocusCycle(FOCUS_CYCLE_LOCAL);

  textview = new TextView(AUTOSIZE, AUTOSIZE);
  textview->append(text);
  layout->insertWidget(0, *textview);
}

void MessageDialog::emitResponse(ResponseType response)
{
  signal_response(*this, response);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
