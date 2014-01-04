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
 * Panel class implementation.
 *
 * @ingroup cppconsui
 */

#include "Panel.h"

#include <algorithm>
#include <cstring>

namespace CppConsUI
{

Panel::Panel(int w, int h, const char *text)
: Widget(w, h), title(NULL), title_width(0)
{
  setTitle(text);
}

Panel::~Panel()
{
  delete [] title;
}

void Panel::draw()
{
  proceedUpdateArea();

  if (!area)
    return;

  int realw = area->getmaxx();
  int realh = area->getmaxy();
  int attrs, i;

  // calc title width
  int draw_title_width = 0;
  if (realw > 4)
    draw_title_width = realw - 4;
  draw_title_width = std::min(draw_title_width, title_width);

  // calc horizontal line length (one segment width)
  int hline_len = 0;
  int extra = draw_title_width ? 4 : 2;
  if (realw > draw_title_width + extra)
    hline_len = (realw - draw_title_width - extra) / 2;

  if (draw_title_width) {
    // draw title
    attrs = getColorPair("panel", "title");
    area->attron(attrs);
    area->mvaddstring(2 + hline_len, 0, draw_title_width, title);
    area->attroff(attrs);
  }

  // draw lines
  attrs = getColorPair("panel", "line");
  area->attron(attrs);

  int wa = (realw >= width || width == AUTOSIZE) && realw > 1 ? 1 : 0;
  int ha = (realh >= height || height == AUTOSIZE) && realh > 1 ? 1 : 0;

  // draw top horizontal line
  for (i = 1; i < 1 + hline_len; i++)
    area->mvaddlinechar(i, 0, Curses::LINE_HLINE);
  for (i = 1 + hline_len + extra - 2 + draw_title_width; i < realw - 1 * wa;
      i++)
    area->mvaddlinechar(i, 0, Curses::LINE_HLINE);

  // draw bottom horizontal line
  if (ha)
    for (i = 1; i < realw - 1 * wa; i++)
      area->mvaddlinechar(i, realh - 1, Curses::LINE_HLINE);

  // draw left and right vertical line
  for (i = 1; i < realh - 1 * ha; i++)
    area->mvaddlinechar(0, i, Curses::LINE_VLINE);
  if (wa)
    for (i = 1; i < realh - 1 * ha; i++)
      area->mvaddlinechar(realw - 1, i, Curses::LINE_VLINE);

  // draw corners
  area->mvaddlinechar(0, 0, Curses::LINE_ULCORNER);
  if (wa)
    area->mvaddlinechar(realw - 1, 0, Curses::LINE_URCORNER);
  if (ha)
    area->mvaddlinechar(0, realh - 1, Curses::LINE_LLCORNER);
  if (wa && ha)
    area->mvaddlinechar(realw - 1, realh - 1, Curses::LINE_LRCORNER);

  area->attroff(attrs);
}

void Panel::setTitle(const char *new_title)
{
  delete [] title;

  size_t size = 1;
  if (new_title)
    size += std::strlen(new_title);
  title = new char[size];
  if (new_title)
    std::strcpy(title, new_title);
  else
    title[0] = '\0';
  title_width = Curses::onscreen_width(title);

  redraw();
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
