/*
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

#include "ConsuiCurses.h"

// @todo Window instances created here aren't usually deleted anywhere!!

//#define _XOPEN_SOURCE_EXTENDED
#define NCURSES_NOMACROS

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif

struct Curses::Window::WindowInternals
{
	WINDOW *win;
	WindowInternals(WINDOW *w = NULL) : win(w) {}
};

Curses::Window::Window()
{
	p = new WindowInternals();
}

Curses::Window::~Window()
{
	delete p;
}

int Curses::Attr::A_NORMAL_rename()
{
	return A_NORMAL;
}

int Curses::Attr::A_REVERSE_rename()
{
	return A_REVERSE;
}

int Curses::Attr::A_DIM_rename()
{
	return A_DIM;
}

void Curses::mvwaddstring(Window *area, int y, int x, int w, /* gboolean selected, */ const gchar *str)
{
	// @todo `\v' switch is not implemented yet
	// @todo optimizations (don't translate to cchar if we have got utf8
	// terminal, etc.)
	// @todo error checking (setcchar)

	int printed = 0;
	const gchar *u;

	::wmove(area->p->win, y, x);
	//attrset(selection_color(selected, COLOR_STANDARD));

	for (u = str; *u && (w == -1 || printed < w); u++) {
		/*
		if (*u == COLOR_SELECT_CHAR) {
			u++;
			if (*u & COLOR_SPECIAL) {
				attrset(selection_color_special(selected, *u & COLOR_MASK));
			} else if (*u & COLOR_BOLD) {
				attrset(selection_color(selected, *u & COLOR_MASK));
				attron(A_BOLD);
			} else {
				attrset(selection_color(selected, *u & COLOR_MASK));
			}
			continue;
		}
		*/

		if (((unsigned char) *u >= 0x7f && (unsigned char) *u < 0xa0)) {
			// filter out C1 (8-bit) control characters
			::waddch(area->p->win, '?');
			printed++;
			continue;
		}

		// get a unicode character from the next few bytes
		wchar_t wch[2];
		cchar_t cc;

		wch[0] = g_utf8_get_char_validated(u, -1);
		wch[1] = L'\0';

		// invalid utf-8 sequence
		if (wch[0] < 0)
			continue;

		// control char symbols
		if (wch[0] < 32)
			wch[0] = 0x2400 + wch[0];

		::setcchar(&cc, wch, A_NORMAL, 0, NULL);
		::wadd_wch(area->p->win, &cc);
		printed += g_unichar_iswide(wch[0]) ? 2 : 1;
		u = g_utf8_next_char(u) - 1;
	}
	if (w != -1)
		::whline(area->p->win, ' ', w - printed);
}

void Curses::mvwaddstringf(Window *area, int y, int x, int w, const gchar *fmt, ...)
{
	char *s;
	va_list ap;

	va_start(ap, fmt);
	s = g_strdup_vprintf(fmt, ap);
	va_end(ap);
	mvwaddstring(area, y, x, w, s);
	g_free(s);
}

int Curses::mvwaddstr(Window *area, int y, int x, const char *str)
{
	return ::mvwaddstr(area->p->win, y, x, str);
}

int Curses::mvwaddnstr(Window *area, int y, int x, const char *str, int n)
{
	return ::mvwaddnstr(area->p->win, y, x, str, n);
}

int Curses::wattron(Window *area, int attrs)
{
	return ::wattron(area->p->win, attrs);
}

int Curses::wattroff(Window *area, int attrs)
{
	return ::wattroff(area->p->win, attrs);
}

int Curses::mvwchgat(Window *area, int y, int x, int n, /* attr_t */ int attr, short color, const void *opts)
{
	return ::mvwchgat(area->p->win, y, x, n, attr, color, opts);
}

int Curses::erase()
{
	return ::erase();
}

int Curses::werase(Window *area)
{
	return ::werase(area->p->win);
}

int Curses::wclrtoeol(Window *area)
{
	return ::wclrtoeol(area->p->win);
}

int Curses::wclrtobot(Window *area)
{
	return ::wclrtobot(area->p->win);
}

int Curses::wnoutrefresh(Window *area)
{
	if (area == NULL)
		return ::wnoutrefresh(stdscr);

	return ::wnoutrefresh(area->p->win);
}

int Curses::doupdate()
{
	return ::doupdate();
}

int Curses::touchwin(Window *area)
{
	return ::touchwin(area->p->win);
}

int Curses::beep()
{
	return ::beep();
}

Curses::Window *Curses::newpad(int nlines, int ncols)
{
	Window *a = new Window;
	a->p->win = ::newpad(nlines, ncols);
	return a;
}

Curses::Window *Curses::subpad(Window *orig, int nlines, int ncols, int begin_y, int begin_x)
{
	Window *a = new Window;
	a->p->win = ::subpad(orig->p->win, nlines, ncols, begin_y, begin_x);
	return a;
}

Curses::Window *Curses::newwin(int nlines, int ncols, int begin_y, int begin_x)
{
	Window *a = new Window;
	a->p->win = ::newwin(nlines, ncols, begin_y, begin_x);
	return a;
}

int Curses::copywin(const Window *srcwin, Window *dstwin, int sminrow,
		int smincol, int dminrow, int dmincol, int dmaxrow,
		int dmaxcol, int overlay)
{
	if (!srcwin->p->win || !dstwin->p->win)
		return OK;

	return ::copywin(srcwin->p->win, dstwin->p->win, sminrow, smincol, dminrow, dmincol,
			dmaxrow, dmaxcol, overlay);
}

int Curses::delwin(Window *area)
{
	int res;

	if (area == NULL)
		return OK;

	// @todo review
	res = ::delwin(area->p->win);
	delete area;
	area = NULL;

	return res;
}

Curses::Window *Curses::initscr()
{
	Window *a = new Window;
	a->p->win = ::initscr();
	return a;
}

int Curses::endwin()
{
	return ::endwin();
}

int Curses::start_color()
{
	return ::start_color();
}

int Curses::curs_set(int visibility)
{
	return ::curs_set(visibility);
}

int Curses::keypad(Window *area, bool bf)
{
	if (area == NULL)
		return ::keypad(stdscr, bf);

	return ::keypad(area->p->win, bf);
}

int Curses::nonl()
{
	return ::nonl();
}

int Curses::raw()
{
	return ::raw();
}

void Curses::ngetmaxyx(Window *area, int *y, int *x)
{
	if (area == NULL) {
		*y = ::getmaxy(stdscr);
		*x = ::getmaxx(stdscr);
	}
	else {
		*y = ::getmaxy(area->p->win);
		*x = ::getmaxx(area->p->win);
	}
}

int Curses::getmaxx(Window *area)
{
	if (area == NULL)
		return ::getmaxx(stdscr);

	return ::getmaxx(area->p->win);
}

int Curses::getmaxy(Window *area)
{
	if (area == NULL)
		return ::getmaxy(stdscr);

	return ::getmaxy(area->p->win);
}

int Curses::resizeterm(int lines, int columns)
{
	return ::resizeterm(lines, columns);
}

int Curses::C_OK()
{
	return OK;
}

int Curses::C_ERR()
{
	return ERR;
}
