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

/** @file HorizontalLine.cpp HorizontalLine class implementation
 * @ingroup cppconsui
 */

#include "ConsuiCurses.h"
#include "HorizontalLine.h"
#include "LineStyle.h"
#include "Widget.h"

HorizontalLine::HorizontalLine(Widget& parent, const int x, const int y, const int w)
: Widget(parent, x, y, w, 1)
, LineStyle(LineStyle::DEFAULT)
{
}

HorizontalLine::HorizontalLine(Widget& parent, LineStyle::Type ltype, const int x, const int y, const int w)
: Widget(parent, x, y, w, 1)
, LineStyle(ltype)
{
}

HorizontalLine::~HorizontalLine()
{
}

void HorizontalLine::Draw()
{
	if (!area || Width() == 0)
		return; /// @todo and throw an exception/log a warning?

	if (Width() <= 1) {
		area->mvaddstring(0, 0, 1, H());
	} else {
		area->mvaddstring(0, 0, 1, HBegin());
		// @todo optimize
		for (int i = 1; i < Width() - 1; i++) {
			area->mvaddstring(0, i, 1, H());
		}
		area->mvaddstring(0, Width() - 1, 1, HEnd());
	}
}
