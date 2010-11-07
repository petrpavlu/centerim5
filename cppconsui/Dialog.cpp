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
 * Dialog class implementation.
 *
 * @ingroup cppconsui
 */

#include "Dialog.h"

Dialog::Dialog(int x, int y, int w, int h, const gchar *title,
    LineStyle::Type ltype)
: Window(x, y, w, h, title, TYPE_TOP, ltype)
{
  InitLayout();
}

Dialog::Dialog(const gchar *title, LineStyle::Type ltype)
: Window(10, 10, 60, 12, title, TYPE_TOP, ltype)
{
  /// @todo Set correct position.

  InitLayout();
}

void Dialog::Close()
{
  Response(RESPONSE_CANCEL);
}

void Dialog::AddButton(const gchar *text, Dialog::ResponseType response)
{
  buttons->AppendItem(text, sigc::bind(sigc::mem_fun(this,
          &Dialog::OnButtonResponse), response));
}

void Dialog::AddSeparator()
{
  buttons->AppendSeparator();
}

void Dialog::Response(Dialog::ResponseType response)
{
  signal_response(*this, response);

  Window::Close();
}

void Dialog::InitLayout()
{
  layout = new ListBox(AUTOSIZE, AUTOSIZE);
  AddWidget(*layout, 0, 0);

  separator = new HorizontalLine(AUTOSIZE);
  layout->AppendWidget(*separator);
  buttons = new HorizontalListBox(AUTOSIZE, 1);
  layout->AppendWidget(*buttons);
}

void Dialog::OnButtonResponse(Button& activator, ResponseType response)
{
  Response(response);
}
