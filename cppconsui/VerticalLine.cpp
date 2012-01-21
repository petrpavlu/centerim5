/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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
 * VerticalLine class implementation.
 *
 * @ingroup cppconsui
 */

#include "VerticalLine.h"

namespace CppConsUI
{

VerticalLine::VerticalLine(int h)
: AbstractLine(1, h)
{
}

void VerticalLine::Draw()
{
  ProceedUpdateArea();

  int realh;

  if (!area || (realh = area->getmaxy()) == 0 || area->getmaxx() != 1)
    return;

  int attrs = GetColorPair("verticalline", "line");
  area->attron(attrs);
  for (int i = 0; i < realh; i++)
    area->mvaddlinechar(i, 0, Curses::LINE_VLINE);
  area->attroff(attrs);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab */
