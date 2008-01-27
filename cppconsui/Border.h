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

#ifndef __BORDER_H__
#define __BORDER_H__

#include "LineStyle.h"

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
//#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>
#else
#include <curses.h>
#endif

//TODO borders with colors are nice
class Border
: public LineStyle
{
	public:
		Border(void);
		Border(LineStyle *linestyle, int width, int height);

		void Resize(int newwidth, int newhheight);
		void Draw(WINDOW *window);

	protected:
		int width, height;

	private:
		Border& operator=(const Border&);
};

#endif /* __BORDER_H__ */
