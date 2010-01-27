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

#include "Border.h"

#include "LineStyle.h"

//TODO CppConsUI namespace

Border::Border()
: LineStyle(LineStyleDefault())
, width(0)
, height(0)
{
}

Border::Border(LineStyle *linestyle, int width, int height)
: LineStyle(linestyle)
, width(width)
, height(height)
{
}

Border::~Border()
{
}

void Border::Resize(int newwidth, int newheight)
{
	width = newwidth;
	height = newheight;
}

void Border::Draw(Curses::Window *window)
{
	if (!window || width == 0 || height == 0)
		return; //TODO and throw an exception/log a warning?

	for (int i = 1; i < width-1; i++) {
		Curses::mvwadd_wch(window, 0, i, h);
		Curses::mvwadd_wch(window, height-1, i, h);
	}

	for (int i = 1; i < height-1; i++) {
		Curses::mvwadd_wch(window, i, 0, v);
		Curses::mvwadd_wch(window, i, width-1, v);
	}
	Curses::mvwadd_wch(window, 0, 0, corner_tl);
	Curses::mvwadd_wch(window, height-1, 0, corner_bl);
	Curses::mvwadd_wch(window, 0, width-1, corner_tr);
	Curses::mvwadd_wch(window, height-1, width-1, corner_br);
}
