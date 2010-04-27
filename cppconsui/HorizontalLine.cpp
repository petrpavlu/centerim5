/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010 by CenterIM developers
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
 * HorizontalLine class implementation
 *
 * @ingroup cppconsui
 */

#include "HorizontalLine.h"

#include "ConsuiCurses.h"
#include "ColorScheme.h"

HorizontalLine::HorizontalLine(int x, int y, int w, LineStyle::Type ltype)
: AbstractLine(x, y, w, 1, ltype)
{
}

void HorizontalLine::Draw()
{
	int realw;

	if (!area || (realw = area->getmaxx()) == 0 || area->getmaxy() != 1)
		return;

	int attrs = COLORSCHEME->GetColorPair(GetColorScheme(), "horizontalline", "line");
	area->attron(attrs);
	if (realw <= 1) {
		area->mvaddstring(0, 0, linestyle.H());
	} else {
		area->mvaddstring(0, 0, linestyle.HBegin());
		for (int i = 1; i < realw - 1; i++) {
			area->mvaddstring(i, 0, linestyle.H());
		}
		area->mvaddstring(realw - 1, 0, linestyle.HEnd());
	}
	area->attroff(attrs);
}
