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

/**
 * @file ConsuiCurses.h Hidden implementation of curses specific functions
 * @todo make ConsuiCurses.h a full handler of all curses functions that could be easily changed
 * with a different implementation.
 */

#ifndef __CONSUICURSES_H__
#define __CONSUICURSES_H__

#include <glib.h>

class Curses
{
	public:
		class Window
		{
			public:
				virtual ~Window();

				// these tree functions returns NULL if such pad/window can
				// not be created
				static Window *newpad(int cols, int nlines);
				static Window *newwin(int begin_x, int begin_y, int ncols, int nlines);

				Window *subpad(int begin_x, int begin_y, int ncols, int nlines);

				/**
				 * This function takes a formatted string and draws it on the
				 * screen. The formatting of the string happens when a '\v' is
				 * encountered. After the '\v' is a char, a switch in the code
				 * figures out what to do based on this. Based on giFTcurs
				 * drawing function.
				 */
				void mvaddstring(int x, int y, int w, const gchar *str);

				// @todo remove
				int mvaddstr(int x, int y, const char *str);
				int mvaddnstr(int x, int y, const char *str, int n);

				int attron(int attrs);
				int attroff(int attrs);
				int mvchgat(int x, int y, int n, /* attr_t */ int attr, short color, const void *opts);

				int erase();
				int clrtoeol();
				int clrtobot();

				int noutrefresh();

				int touch();

				int copyto(Window *dstwin, int smincol, int sminrow,
						int dmincol, int dminrow, int dmaxcol, int dmaxrow,
						int overlay);

				int getmaxx();
				int getmaxy();

				int keypad(bool bf);

			protected:
				struct WindowInternals;
				WindowInternals *p;

			private:
				Window();
				Window(const Window &other);
				Window &operator=(const Window &other);
		};

		struct Attr
		{
			static int NORMAL;
			static int REVERSE;
			static int DIM;
		};

		static int erase();
		static int doupdate();

		static int beep();

		// @todo add noraw to raw, nl to nonl etc
		static int initscr();
		static int endwin();
		static int start_color();
		static int curs_set(int visibility);
		static int nonl();
		static int raw();

		// stdscr
		static int noutrefresh();
		static int keypad(bool bf);
		static int getmaxx();
		static int getmaxy();

		static int resizeterm(int lines, int columns);

		static int C_OK;
		static int C_ERR;

	protected:

	private:
};

#endif /* __CONSUICURSES_H__ */
