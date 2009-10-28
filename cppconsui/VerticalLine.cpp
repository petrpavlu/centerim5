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

#include "ConsuiCurses.h"
#include "VerticalLine.h"
#include "LineStyle.h"
#include "Widget.h"

VerticalLine::VerticalLine(Widget& parent, const int x, const int y, const int w)
: Widget(parent, x, y, w, 1)
, LineStyle(LineStyleDefault())
{
}

VerticalLine::VerticalLine(Widget& parent, LineStyle *linestyle, const int x, const int y, const int w)
: Widget(parent, x, y, w, 1)
, LineStyle(linestyle)
{
}

VerticalLine::~VerticalLine()
{
}

void VerticalLine::Draw()
{
	if (!area->w || Height() == 0)
		return; //TODO and throw an exception/log a warning?

	if (Height() <= 1) {
		mvwadd_wch(area->w, 0, 0, V());
	} else {
		mvwadd_wch(area->w, 0, 0, VBegin());
		for (int i = 1; i < Height()-1; i++) {
			mvwadd_wch(area->w, 0, i, V());
		}
		mvwadd_wch(area->w, 0, Width()-1, VEnd());
	}
}
