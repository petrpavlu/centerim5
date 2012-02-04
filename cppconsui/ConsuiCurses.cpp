/*
 * Copyright (C) 2010-2012 by CenterIM developers
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
 */

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

namespace CppConsUI
{

namespace Curses
{

static Stats stats = {};

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

  int printed = 0;
  while (printed < w && str && *str) {
    printed += PrintChar(g_utf8_get_char(str));
    str = g_utf8_find_next_char(str, NULL);
  }
  return printed;
}

int Window::mvaddstring(int x, int y, const char *str)
{
  g_assert(str);

  wmove(p->win, y, x);

  int printed = 0;
  while (str && *str) {
    printed += PrintChar(g_utf8_get_char(str));
    str = g_utf8_find_next_char(str, NULL);
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
  while (printed < w && str < end && str && *str) {
    printed += PrintChar(g_utf8_get_char(str));
    str = g_utf8_find_next_char(str, end);
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
  while (str < end && str && *str) {
    printed += PrintChar(g_utf8_get_char(str));
    str = g_utf8_find_next_char(str, end);
  }
  return printed;
}

int Window::mvaddchar(int x, int y, gunichar uc)
{
  wmove(p->win, y, x);
  return PrintChar(uc);
}

int Window::mvaddlinechar(int x, int y, LineChar c)
{
  switch (c) {
    case LINE_HLINE:
      return mvwaddch(p->win, y, x, ACS_HLINE);
    case LINE_VLINE:
      return mvwaddch(p->win, y, x, ACS_VLINE);
    case LINE_LLCORNER:
      return mvwaddch(p->win, y, x, ACS_LLCORNER);
    case LINE_LRCORNER:
      return mvwaddch(p->win, y, x, ACS_LRCORNER);
    case LINE_ULCORNER:
      return mvwaddch(p->win, y, x, ACS_ULCORNER);
    case LINE_URCORNER:
      return mvwaddch(p->win, y, x, ACS_URCORNER);
    case LINE_BTEE:
      return mvwaddch(p->win, y, x, ACS_BTEE);
    case LINE_LTEE:
      return mvwaddch(p->win, y, x, ACS_LTEE);
    case LINE_RTEE:
      return mvwaddch(p->win, y, x, ACS_RTEE);
    case LINE_TTEE:
      return mvwaddch(p->win, y, x, ACS_TTEE);

    case LINE_DARROW:
      return mvwaddch(p->win, y, x, ACS_DARROW);
    case LINE_LARROW:
      return mvwaddch(p->win, y, x, ACS_LARROW);
    case LINE_RARROW:
      return mvwaddch(p->win, y, x, ACS_RARROW);
    case LINE_UARROW:
      return mvwaddch(p->win, y, x, ACS_UARROW);
    case LINE_BULLET:
      return mvwaddch(p->win, y, x, ACS_BULLET);
  }
  return ERR;
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
  attr_t battrs;
  short pair;

  if (attr_get(&battrs, &pair, NULL) == ERR)
    return ERR;

  if (attron(attrs) == ERR)
    return ERR;

  int realw = getmaxx();
  int realh = getmaxy();

  for (int i = 0; i < realw; i++)
    for (int j = 0; j < realh; j++) {
      /* Note: mvwaddch() returns ERR here when i = realw - 1 and
       * j = realh - 1 because the cursor can't be wrapped to the next line.
       * */
      mvwaddch(p->win, j, i, ' ');
    }

  if (attr_set(battrs, pair, NULL) == ERR)
    return ERR;

  return OK;
}

int Window::fill(int attrs, int x, int y, int w, int h)
{
  attr_t battrs;
  short pair;

  if (attr_get(&battrs, &pair, NULL) == ERR)
    return ERR;

  if (attron(attrs) == ERR)
    return ERR;

  int realw = getmaxx();
  int realh = getmaxy();

  for (int i = x; i < realw && i < x + w; i++)
    for (int j = y; j < realh && j < y + h; j++)
      mvwaddch(p->win, j, i, ' ');

  if (attr_set(battrs, pair, NULL) == ERR)
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

int Window::PrintChar(gunichar uc)
{
  /**
   * @todo Error checking (setcchar).
   */

  if (uc >= 0x7f && uc < 0xa0) {
    // filter out C1 (8-bit) control characters
    waddch(p->win, '?');
    return 1;
  }

  // get a unicode character from the next few bytes
  wchar_t wch[2];
  cchar_t cc;

  wch[0] = uc;
  wch[1] = L'\0';

  // invalid utf-8 sequence
  if (wch[0] < 0)
    return 0;

  // tab character
  if (wch[0] == '\t') {
    int w = onscreen_width(wch[0]);
    for (int i = 0; i < w; i++)
      waddch(p->win, ' ');
    return w;
  }

  // control char symbols
  if (wch[0] < 32)
    wch[0] = 0x2400 + wch[0];

  setcchar(&cc, wch, A_NORMAL, 0, NULL);
  wadd_wch(p->win, &cc);
  return onscreen_width(wch[0]);
}

Window::Window()
: p(new WindowInternals)
{
}

const int Color::DEFAULT = -1;
const int Color::BLACK = COLOR_BLACK;
const int Color::RED = COLOR_RED;
const int Color::GREEN = COLOR_GREEN;
const int Color::YELLOW = COLOR_YELLOW;
const int Color::BLUE = COLOR_BLUE;
const int Color::MAGENTA = COLOR_MAGENTA;
const int Color::CYAN = COLOR_CYAN;
const int Color::WHITE = COLOR_WHITE;

const int Attr::NORMAL = A_NORMAL;
const int Attr::STANDOUT = A_STANDOUT;
const int Attr::REVERSE = A_REVERSE;
const int Attr::BLINK = A_BLINK;
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
    if (::use_default_colors() == ERR)
      return ERR;
  if (::curs_set(0) == ERR)
    return ERR;
  if (::nonl() == ERR)
    return ERR;
  if (::raw() == ERR)
    return ERR;
  return OK;
}

bool init_colorpair(int idx, int fg, int bg, int *res)
{
  bool success;

  success = (init_pair(idx, fg, bg) != ERR);

  if (success)
    *res = COLOR_PAIR(idx);

  return success;
}

int screen_finalize()
{
  return ::endwin();
}

int nrcolors()
{
  return COLORS;
}

int nrcolorpairs()
{
#ifndef NCURSES_EXT_COLORS
  /* ncurses reports more than 256 color pairs, even
   * when compiled without ext-color.
   */
  return MIN(COLOR_PAIRS, 256);
#else
  return COLOR_PAIRS;
#endif
}

#ifdef DEBUG
bool colorpair_content(int colorpair, int *fg, int *bg)
{
  short sfg, sbg;
  int ret;

  ret = pair_content(PAIR_NUMBER(colorpair), &sfg, &sbg);

  *fg = sfg;
  *bg = sbg;

  return ret;
}
#endif

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

  if (!start)
    return 0;

  if (!end)
    end = start + strlen(start);

  while (start < end) {
    width += onscreen_width(g_utf8_get_char(start));
    start = g_utf8_next_char(start);
  }
  return width;
}

int onscreen_width(gunichar uc, int w)
{
  if (uc == '\t')
    return 8 - w % 8;
  return g_unichar_iswide(uc) ? 2 : 1;
}

} // namespace Curses

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 tw=78 expandtab : */
