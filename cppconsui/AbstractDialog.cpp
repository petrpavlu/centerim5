/*
 * Copyright (C) 2011-2012 by CenterIM developers
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
 * AbstractDialog class implementation.
 *
 * @ingroup cppconsui
 */

#include "AbstractDialog.h"

namespace CppConsUI
{

AbstractDialog::AbstractDialog(int x, int y, int w, int h, const char *title)
: Window(x, y, w, h, title, TYPE_TOP)
{
  InitLayout();
}

AbstractDialog::AbstractDialog(const char *title)
: Window(10, 10, 60, 12, title, TYPE_TOP)
{
  /// @todo Set correct position.

  InitLayout();
}

void AbstractDialog::Close()
{
  Response(RESPONSE_CANCEL);
}

void AbstractDialog::AddButton(const char *text,
    AbstractDialog::ResponseType response)
{
  buttons->AppendItem(text, sigc::bind(sigc::mem_fun(this,
          &AbstractDialog::OnButtonResponse), response));
}

void AbstractDialog::AddSeparator()
{
  buttons->AppendSeparator();
}

void AbstractDialog::Response(ResponseType response)
{
  EmitResponse(response);
  Window::Close();
}

void AbstractDialog::InitLayout()
{
  layout = new ListBox(AUTOSIZE, AUTOSIZE);
  AddWidget(*layout, 0, 0);

  separator = new HorizontalLine(AUTOSIZE);
  layout->AppendWidget(*separator);
  buttons = new HorizontalListBox(AUTOSIZE, 1);
  layout->AppendWidget(*buttons);
}

void AbstractDialog::OnButtonResponse(Button& /*activator*/,
    ResponseType response)
{
  Response(response);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
