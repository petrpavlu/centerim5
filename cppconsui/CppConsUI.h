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

/// @file CppConsUI.h
/// @todo CppConsUI namespace

#ifndef __CPPCONSUI_H__
#define __CPPCONSUI_H__

#include <glibmm/ustring.h>
#include <wchar.h>
#include <glib.h>

#ifdef ENABLE_NLS

#include <libintl.h>
#define _(s)	gettext(s)

#else

#define _(s)	(s)

#endif


#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
//#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>
#else
#include <curses.h>
#endif

int RealScreenWidth(void);
int RealScreenHeight(void);

Glib::ustring::size_type width(const Glib::ustring &string);
Glib::ustring::size_type width(const char *start, const char *end);

/// @todo perhaps move to its own files?

class Point
{
	public:
		Point();
		Point(int x, int y);

		int X(void) { return x; }
		int Y(void) { return y; }

		int x, y;
	protected:

	private:
};

class Rect: public Point
{
	public:
		Rect();
		Rect(int x, int y, int w, int h);

		int Width(void) { return width; }
		int Height(void) { return height; }
		int Left(void) { return x; }
		int Top(void) { return y; }
		int Right(void) { return x+width; }
		int Bottom(void) { return y+height; }

		int width, height;

	protected:

	private:
};

#endif /* __CPPCONSUI_H__ */
