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
 * AbstractListBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "AbstractListBox.h"

#include "Button.h"

AbstractListBox::AbstractListBox(int w, int h)
: ScrollPane(w, h, 0, 0)
{
}

Button *AbstractListBox::InsertItem(size_t pos, const gchar *title,
    sigc::slot<void, Button&> function)
{
  Button *b = new Button(Curses::onscreen_width(title), 1, title);
  b->signal_activate.connect(function);
  InsertWidget(pos, *b);
  return b;
}

Button *AbstractListBox::AppendItem(const gchar *title,
    sigc::slot<void, Button&> function)
{
  Button *b = new Button(Curses::onscreen_width(title), 1, title);
  b->signal_activate.connect(function);
  AppendWidget(*b);
  return b;
}

void AbstractListBox::AddWidget(Widget& widget, int x, int y)
{
  ScrollPane::AddWidget(widget, x, y);
}
