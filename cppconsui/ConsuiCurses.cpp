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

int Curses::Attr::NORMAL = A_NORMAL;
int Curses::Attr::REVERSE = A_REVERSE;
int Curses::Attr::DIM = A_DIM;

int Curses::C_OK = OK;
int Curses::C_ERR = ERR;

struct Curses::Window::WindowInternals
{
	WINDOW *win;
	WindowInternals(WINDOW *w = NULL) : win(w) {}
};

Curses::Window::Window()
: p(new WindowInternals)
{
}

Curses::Window::~Window()
{
	::delwin(p->win);
	delete p;
}

Curses::Window *Curses::Window::newpad(int nlines, int ncols)
{
	Window *a = new Window;
	a->p->win = ::newpad(nlines, ncols);
	return a;
}

Curses::Window *Curses::Window::newwin(int nlines, int ncols, int begin_y, int begin_x)
{
	Window *a = new Window;
	a->p->win = ::newwin(nlines, ncols, begin_y, begin_x);
	return a;
}

Curses::Window *Curses::Window::subpad(int nlines, int ncols, int begin_y, int begin_x)
{
	Window *a = new Window;
	a->p->win = ::subpad(p->win, nlines, ncols, begin_y, begin_x);
	return a;
}

void Curses::Window::mvaddstring(int y, int x, int w, const gchar *str)
{
	// @todo `\v' switch is not implemented yet
	// @todo optimizations (don't translate to cchar if we have got utf8
	// terminal, etc.)
	// @todo error checking (setcchar)

	int printed = 0;
	const gchar *u;

	::wmove(p->win, y, x);
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
			::waddch(p->win, '?');
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
		::wadd_wch(p->win, &cc);
		printed += g_unichar_iswide(wch[0]) ? 2 : 1;
		u = g_utf8_next_char(u) - 1;
	}
	if (w != -1)
		::whline(p->win, ' ', w - printed);
}

void Curses::Window::mvaddstringf(int y, int x, int w, const gchar *fmt, ...)
{
	char *s;
	va_list ap;

	va_start(ap, fmt);
	s = g_strdup_vprintf(fmt, ap);
	va_end(ap);
	mvaddstring(y, x, w, s);
	g_free(s);
}

int Curses::Window::mvaddstr(int y, int x, const char *str)
{
	return ::mvwaddstr(p->win, y, x, str);
}

int Curses::Window::mvaddnstr(int y, int x, const char *str, int n)
{
	return ::mvwaddnstr(p->win, y, x, str, n);
}

int Curses::Window::attron(int attrs)
{
	return ::wattron(p->win, attrs);
}

int Curses::Window::attroff(int attrs)
{
	return ::wattroff(p->win, attrs);
}

int Curses::Window::mvchgat(int y, int x, int n, /* attr_t */ int attr, short color, const void *opts)
{
	return ::mvwchgat(p->win, y, x, n, attr, color, opts);
}

int Curses::Window::erase()
{
	return ::werase(p->win);
}

int Curses::Window::clrtoeol()
{
	return ::wclrtoeol(p->win);
}

int Curses::Window::clrtobot()
{
	return ::wclrtobot(p->win);
}

int Curses::Window::noutrefresh()
{
	return ::wnoutrefresh(p->win);
}

int Curses::Window::touch()
{
	return ::touchwin(p->win);
}

int Curses::Window::copyto(Window *dstwin, int sminrow, int smincol,
		int dminrow, int dmincol, int dmaxrow, int dmaxcol,
		int overlay)
{
	if (!p->win || !dstwin->p->win)
		return OK;

	return ::copywin(p->win, dstwin->p->win, sminrow, smincol, dminrow, dmincol,
			dmaxrow, dmaxcol, overlay);
}

int Curses::Window::getmaxx()
{
	return ::getmaxx(p->win);
}

int Curses::Window::getmaxy()
{
	return ::getmaxy(p->win);
}

int Curses::Window::keypad(bool bf)
{
	return ::keypad(p->win, bf);
}

int Curses::erase()
{
	return ::erase();
}

int Curses::doupdate()
{
	return ::doupdate();
}

int Curses::beep()
{
	return ::beep();
}

int Curses::initscr()
{
	if (::initscr())
		return OK;
	return ERR;
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

int Curses::nonl()
{
	return ::nonl();
}

int Curses::raw()
{
	return ::raw();
}

int Curses::noutrefresh()
{
	return ::wnoutrefresh(stdscr);
}

int Curses::keypad(bool bf)
{
	return ::keypad(stdscr, bf);
}

int Curses::getmaxx()
{
	return ::getmaxx(stdscr);
}

int Curses::getmaxy()
{
	return ::getmaxy(stdscr);
}

int Curses::resizeterm(int lines, int columns)
{
	return ::resizeterm(lines, columns);
}
