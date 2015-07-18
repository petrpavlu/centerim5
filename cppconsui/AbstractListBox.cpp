/*
 * Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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
 * AbstractListBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "AbstractListBox.h"

namespace CppConsUI {

AbstractListBox::AbstractListBox(int w, int h) : Container(w, h)
{
}

Button *AbstractListBox::insertItem(
  size_t pos, const char *title, const sigc::slot<void, Button &> &callback)
{
  Button *b = new Button(Curses::onScreenWidth(title), 1, title);
  b->signal_activate.connect(callback);
  insertWidget(pos, *b);
  return b;
}

Button *AbstractListBox::appendItem(
  const char *title, const sigc::slot<void, Button &> &callback)
{
  Button *b = new Button(Curses::onScreenWidth(title), 1, title);
  b->signal_activate.connect(callback);
  appendWidget(*b);
  return b;
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
