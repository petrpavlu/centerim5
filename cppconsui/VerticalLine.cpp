/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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

/** @file VerticalLine.cpp VerticalLine class implementation
 * @ingroup cppconsui
 */

#include "ConsuiCurses.h"
#include "VerticalLine.h"
#include "LineStyle.h"
#include "Widget.h"

VerticalLine::VerticalLine(Widget& parent, const int x, const int y, const int w)
: Widget(parent, x, y, w, 1)
, LineStyle(LineStyle::DEFAULT)
{
}

VerticalLine::VerticalLine(Widget& parent, LineStyle::Type ltype, const int x, const int y, const int w)
: Widget(parent, x, y, w, 1)
, LineStyle(ltype)
{
}

VerticalLine::~VerticalLine()
{
}

void VerticalLine::Draw()
{
	int realh;

	if (!area || (realh = area->getmaxy()) == 0)
		return;

	if (realh <= 1) {
		area->mvaddstring(0, 0, 1, V());
	} else {
		area->mvaddstring(0, 0, 1, VBegin());
		for (int i = 1; i < realh - 1; i++) {
			area->mvaddstring(i, 0, 1, V());
		}
		area->mvaddstring(realh - 1, 0, 1, VEnd());
	}
}
