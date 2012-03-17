/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2012 by CenterIM developers
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
 * Label class implementation.
 *
 * @ingroup cppconsui
 */

#include "Label.h"

namespace CppConsUI
{

Label::Label(int w, int h, const char *text_)
: Widget(w, h)
, text(NULL)
{
  SetText(text_);
}

Label::Label(const char *text_)
: Widget(AUTOSIZE, AUTOSIZE)
, text(NULL)
{
  SetText(text_);
}

Label::~Label()
{
  if (text)
    g_free(text);
}

void Label::Draw()
{
  ProceedUpdateArea();

  if (!area || !text)
    return;

  int attrs = GetColorPair("label", "text");
  area->attron(attrs);

  int realw = area->getmaxx();
  int realh = area->getmaxy();

  // print text
  int y = 0;
  const char *start, *end;
  start = end = text;
  int p;
  while (*end) {
    if (*end == '\n') {
      if (y >= realh)
        break;

      p = area->mvaddstring(0, y, realw * (realh - y), start, end);
      y += (p / realw) + 1;
      start = end + 1;
    }
    end++;
  }
  if (y < realh)
    area->mvaddstring(0, y, realw * (realh - y), start, end);

  area->attroff(attrs);
}

void Label::SetText(const char *new_text)
{
  if (text)
    g_free(text);

  text = g_strdup(new_text);

  // update wish height
  int h = 1;
  if (text) {
    const char *start, *end;
    start = end = text;
    while (*end) {
      if (*end == '\n') {
        Curses::onscreen_width(start, end);
        h++;
        start = end + 1;
      }
      end++;
    }
    Curses::onscreen_width(start, end);
  }
  SetWishHeight(h);

  Redraw();
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
