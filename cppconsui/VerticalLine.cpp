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
 * VerticalLine class implementation.
 *
 * @ingroup cppconsui
 */

#include "VerticalLine.h"

#include "ColorScheme.h"

namespace CppConsUI {

VerticalLine::VerticalLine(int h) : AbstractLine(1, h)
{
}

int VerticalLine::draw(Curses::ViewPort area, Error &error)
{
  if (real_height == 0 || real_width != 1)
    return 0;

  int attrs;
  DRAW(getAttributes(ColorScheme::PROPERTY_VERTICALLINE_LINE, &attrs, error));
  DRAW(area.attrOn(attrs, error));

  for (int i = 0; i < real_height; i++)
    DRAW(area.addLineChar(0, i, Curses::LINE_VLINE, error));

  DRAW(area.attrOff(attrs, error));

  return 0;
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
