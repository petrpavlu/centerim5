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
 * HorizontalLine class implementation
 *
 * @ingroup cppconsui
 */

#include "HorizontalLine.h"

namespace CppConsUI
{

HorizontalLine::HorizontalLine(int w)
: AbstractLine(w, 1)
{
}

void HorizontalLine::Draw()
{
  ProceedUpdateArea();

  int realw;

  if (!area || (realw = area->getmaxx()) == 0 || area->getmaxy() != 1)
    return;

  int attrs = GetColorPair("horizontalline", "line");
  area->attron(attrs);
  for (int i = 0; i < realw; i++)
    area->mvaddlinechar(i, 0, Curses::LINE_HLINE);
  area->attroff(attrs);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 tw=78 expandtab : */
