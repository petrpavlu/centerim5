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
				Window();
				virtual ~Window();

				// @todo make private
				struct WindowInternals;
				WindowInternals *p;

			protected:

			private:
				Window(const Window &);
				Window &operator=(const Window &);
		};

		struct Attr
		{
			// @todo rename when we'll get rid of curses.h from all files
			static int A_NORMAL_rename();
			static int A_REVERSE_rename();
			static int A_DIM_rename();
		};

		/**
		 * @todo we should later create a window class and remove these static
		 * functions...
		 */

		/**
		 * This function takes a formatted string and draws it on the screen.
		 * The formatting of the string happens when a '\v' is encountered.
		 * After the '\v' is a char, a switch in the code figures out what to
		 * do based on this. Based on giFTcurs drawing function.
		 */
		static void mvwaddstring(Window *area, int y, int x, int w, const gchar *str);
		static void mvwaddstringf(Window *area, int y, int x, int w, const gchar *fmt, ...);

		static int mvwaddstr(Window *area, int y, int x, const char *str);
		static int mvwaddnstr(Window *area, int y, int x, const char *str, int n);

		static int wattron(Window *area, int attrs);
		static int wattroff(Window *area, int attrs);
		static int mvwchgat(Window *area, int y, int x, int n, /* attr_t */ int attr, short color, const void *opts);

		static int erase();
		static int werase(Window *area);
		static int wclrtoeol(Window *area);
		static int wclrtobot(Window *area);

		static int wnoutrefresh(Window *area);
		static int doupdate();

		static int touchwin(Window *area);

		static int beep();

		static Window *newpad(int nlines, int ncols);
		static Window *subpad(Window *orig, int nlines, int ncols, int begin_y, int begin_x);
		static Window *newwin(int nlines, int ncols, int begin_y, int begin_x);
		static int copywin(const Window *srcwin, Window *dstwin, int sminrow,
				int smincol, int dminrow, int dmincol, int dmaxrow,
				int dmaxcol, int overlay);
		static int delwin(Window *area);

		// @todo add noraw to raw, nl to nonl etc
		static Window *initscr();
		static int endwin();
		static int start_color();
		static int curs_set(int visibility);
		static int keypad(Window *area, bool bf);
		static int nonl();
		static int raw();

		static void ngetmaxyx(Window *area, int *y, int *x);
		static int getmaxx(Window *area);
		static int getmaxy(Window *area);

		static int resizeterm(int lines, int columns);

		static int C_OK();
		static int C_ERR();

	protected:

	private:
};

#endif /* __CONSUICURSES_H__ */
