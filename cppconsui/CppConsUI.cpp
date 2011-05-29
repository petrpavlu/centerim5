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
 * General classes, functions and enumerations.
 *
 * @ingroup cppconsui
 */

#include "CppConsUI.h"

namespace CppConsUI
{

Point::Point()
: x(0)
, y(0)
{
}

Point::Point(int x, int y)
: x(x)
, y(y)
{
}

Rect::Rect()
: Point(0, 0)
, width(0)
, height(0)
{
}

Rect::Rect(int x, int y, int w, int h)
: Point(x, y)
, width(w)
, height(h)
{
}

} // namespace CppConsUI
