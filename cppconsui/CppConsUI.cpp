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

#include "CppConsUI.h"

#include <glibmm/ustring.h>
#include <wchar.h>

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif

//TODO getmaxx(stdscreen) just must have less overhead
int RealScreenWidth(void)
{
	int x, y;
	getmaxyx(stdscr, y, x);
	return x;
}

int RealScreenHeight(void)
{
	int x, y;
	getmaxyx(stdscr, y, x);
	return y;
}

Glib::ustring::size_type width(const Glib::ustring &string)
{
	return width(string.data(), string.data()+string.bytes());
}

//NOTE copied from libgnt/gntutils.c
//TODO should g_unichar_iszerowidth be used?
//TODO write a wrapper string class
//if so, then also include drawing functions and a way to store colours
//for a string.
Glib::ustring::size_type width(const char *start, const char *end)
{
        Glib::ustring::size_type width = 0;

        if (end == NULL)
                end = start + strlen(start);

        while (start < end) {
                width += g_unichar_iswide(g_utf8_get_char(start)) ? 2 : 1;
                start = g_utf8_next_char(start);
        }
        return width;
}

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
