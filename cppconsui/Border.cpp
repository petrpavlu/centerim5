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
: LineStyle(LineStyle::DEFAULT)
, width(0)
, height(0)
{
}

Border::Border(LineStyle::Type ltype, int width, int height)
: LineStyle(ltype)
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

	// @todo optimize
	for (int i = 1; i < width - 1; i++) {
		Curses::mvwaddstring(window, 0, i, 1, H());
		Curses::mvwaddstring(window, height - 1, i, 1, H());
	}

	for (int i = 1; i < height-1; i++) {
		Curses::mvwaddstring(window, i, 0, 1, V());
		Curses::mvwaddstring(window, i, width - 1, 1, V());
	}
	Curses::mvwaddstring(window, 0, 0, 1, CornerTL());
	Curses::mvwaddstring(window, height - 1, 0, 1, CornerBL());
	Curses::mvwaddstring(window, 0, width - 1, 1, CornerTR());
	Curses::mvwaddstring(window, height - 1, width - 1, 1, CornerBR());
}
