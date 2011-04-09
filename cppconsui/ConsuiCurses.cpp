/*
 * Copyright (C) 2010-2011 by CenterIM developers
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
 * Hidden implementation of curses specific functions.
 *
 * @ingroup cppconsui
 */

#include "ConsuiCurses.h"

/* In order to get wide characters support we must define
 * _XOPEN_SOURCE_EXTENDED when using cursesw.h. */
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif

#define NCURSES_NOMACROS
#include <cursesw.h>

#include <map>
#include "gettext.h"

namespace Curses
{

static Stats stats = {0};

const Stats *GetStats()
{
  return &stats;
}

void ResetStats()
{
  memset(&stats, 0, sizeof(stats));
}

struct Window::WindowInternals
{
  WINDOW *win;
  WindowInternals(WINDOW *w = NULL) : win(w) {}
};

Window *Window::newpad(int ncols, int nlines)
{
  stats.newpad_calls++;

  WINDOW *win;

  if (!(win = ::newpad(nlines, ncols)))
    return NULL;

  Window *a = new Window;
  a->p->win = win;
  return a;
}

Window *Window::newwin(int begin_x, int begin_y, int ncols, int nlines)
{
  stats.newwin_calls++;

  WINDOW *win;

  if (!(win = ::newwin(nlines, ncols, begin_y, begin_x)))
    return NULL;

  Window *a = new Window;
  a->p->win = win;
  return a;
}

Window *Window::subpad(int begin_x, int begin_y, int ncols, int nlines)
{
  stats.subpad_calls++;

  WINDOW *win;

  if (!(win = ::subpad(p->win, nlines, ncols, begin_y, begin_x)))
    return NULL;

  Window *a = new Window;
  a->p->win = win;
  return a;
}

Window::~Window()
{
  delwin(p->win);
  delete p;
}

int Window::mvaddstring(int x, int y, int w, const char *str)
{
  g_assert(str);

  wmove(p->win, y, x);
  //attrset(selection_color(selected, COLOR_STANDARD));

  int printed = 0;
  int p;
  while (printed < w && str && *str) {
    str = PrintChar(str, &p);
    printed += p;
  }
  return printed;
}

int Window::mvaddstring(int x, int y, const char *str)
{
  g_assert(str);

  wmove(p->win, y, x);

  int printed = 0;
  int p;
  while (str && *str) {
    str = PrintChar(str, &p);
    printed += p;
  }
  return printed;
}

int Window::mvaddstring(int x, int y, int w, const char *str, const char *end)
{
  g_assert(str);
  g_assert(end);

  if (str >= end)
    return 0;

  wmove(p->win, y, x);

  int printed = 0;
  int p;
  while (printed < w && str < end && str && *str) {
    str = PrintChar(str, &p, end);
    printed += p;
  }
  return printed;
}

int Window::mvaddstring(int x, int y, const char *str, const char *end)
{
  g_assert(str);
  g_assert(end);

  if (str >= end)
    return 0;

  wmove(p->win, y, x);

  int printed = 0;
  int p;
  while (str < end && str && *str) {
    str = PrintChar(str, &p, end);
    printed += p;
  }
  return printed;
}

int Window::attron(int attrs)
{
  return wattron(p->win, attrs);
}

int Window::attroff(int attrs)
{
  return wattroff(p->win, attrs);
}

int Window::mvchgat(int x, int y, int n, /* attr_t */ int attr, short color,
    const void *opts)
{
  return mvwchgat(p->win, y, x, n, attr, color, opts);
}

int Window::fill(int attrs)
{
  if (attron(attrs) == ERR)
    return ERR;

  int w = getmaxx();
  int h = getmaxy();

  for (int i = 0; i < w; i++)
    for (int j = 0; j < h; j++) {
      /* Note: mvwaddch() returns ERR here when i = w - 1 and j = h - 1
       * because the cursor can't be wrapped to the next line. */
      mvwaddch(p->win, j, i, ' ');
    }

  if (attroff(attrs) == ERR)
    return ERR;

  return OK;
}

int Window::erase()
{
  return werase(p->win);
}

int Window::noutrefresh()
{
  return wnoutrefresh(p->win);
}

int Window::touch()
{
  return touchwin(p->win);
}

int Window::copyto(Window *dstwin, int smincol, int sminrow,
    int dmincol, int dminrow, int dmaxcol, int dmaxrow,
    int overlay)
{
  return copywin(p->win, dstwin->p->win, sminrow, smincol, dminrow, dmincol,
      dmaxrow, dmaxcol, overlay);
}

int Window::getmaxx()
{
  return ::getmaxx(p->win);
}

int Window::getmaxy()
{
  return ::getmaxy(p->win);
}

const char *Window::PrintChar(const char *ch, int *printed, const char *end)
{
  /**
   * @todo Error checking (setcchar).
   */

  g_assert(ch);
  g_assert(*ch);
  g_assert(printed);

  *printed = 0;

  if (((unsigned char) *ch >= 0x7f && (unsigned char) *ch < 0xa0)) {
    // filter out C1 (8-bit) control characters
    waddch(p->win, '?');
    *printed = 1;
    return ch + 1;
  }

  // get a unicode character from the next few bytes
  wchar_t wch[2];
  cchar_t cc;

  wch[0] = g_utf8_get_char(ch);
  wch[1] = L'\0';

  // invalid utf-8 sequence
  if (wch[0] < 0)
    return ch + 1;

  // control char symbols
  if (wch[0] < 32)
    wch[0] = 0x2400 + wch[0];

  setcchar(&cc, wch, A_NORMAL, 0, NULL);
  wadd_wch(p->win, &cc);
  *printed = onscreen_width(wch[0]);
  return g_utf8_find_next_char(ch, end);
}

Window::Window()
: p(new WindowInternals)
{
}

const int Color::BLACK = COLOR_BLACK;
const int Color::RED = COLOR_RED;
const int Color::GREEN = COLOR_GREEN;
const int Color::YELLOW = COLOR_YELLOW;
const int Color::BLUE = COLOR_BLUE;
const int Color::MAGENTA = COLOR_MAGENTA;
const int Color::CYAN = COLOR_CYAN;
const int Color::WHITE = COLOR_WHITE;

const int Attr::NORMAL = A_NORMAL;
const int Attr::REVERSE = A_REVERSE;
const int Attr::DIM = A_DIM;
const int Attr::BOLD = A_BOLD;

const int C_OK = OK;
const int C_ERR = ERR;

int screen_init()
{
  if (!::initscr())
    return ERR;

  if (::has_colors())
    if (::start_color() == ERR)
      return ERR;
  if (::curs_set(0) == ERR)
    return ERR;
  if (::nonl() == ERR)
    return ERR;
  if (::raw() == ERR)
    return ERR;
  return OK;
}

int screen_finalize()
{
  return ::endwin();
}

int getcolorpair(int fg, int bg)
{
  typedef std::map<std::pair<int, int>, int> Colors;
  static Colors c;

  Colors::const_iterator i;
  if ((i = c.find(std::make_pair(fg, bg))) != c.end())
    return i->second;

  if ((int) c.size() >= COLOR_PAIRS) {
    g_warning(_("Color pairs limit exceeded.\n"));
    return 0;
  }

  if (init_pair(c.size() + 1, fg, bg) == ERR)
    return 0;
  int res = COLOR_PAIR(c.size() + 1);
  c[std::make_pair(fg, bg)] = res;
  return res;
}

int erase()
{
  return ::erase();
}

int clear()
{
  return ::clear();
}

int doupdate()
{
  return ::doupdate();
}

int beep()
{
  return ::beep();
}

int noutrefresh()
{
  return ::wnoutrefresh(stdscr);
}

int getmaxx()
{
  return ::getmaxx(stdscr);
}

int getmaxy()
{
  return ::getmaxy(stdscr);
}

int resizeterm(int lines, int columns)
{
  return ::resizeterm(lines, columns);
}

/// @todo should g_unichar_iszerowidth be used?
int onscreen_width(const char *start, const char *end)
{
  int width = 0;

  if (start == NULL)
    return 0;

  if (end == NULL)
    end = start + strlen(start);

  while (start < end) {
    width += g_unichar_iswide(g_utf8_get_char(start)) ? 2 : 1;
    start = g_utf8_next_char(start);
  }
  return width;
}

int onscreen_width(gunichar uc)
{
  return g_unichar_iswide(uc) ? 2 : 1;
}

} // namespace Curses
