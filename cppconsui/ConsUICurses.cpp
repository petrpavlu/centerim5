/*
 * Copyright (C) 2010-2013 by CenterIM developers
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

#include "ConsUICurses.h"

/* In order to get wide characters support we must define
 * _XOPEN_SOURCE_EXTENDED when using cursesw.h. */
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif

#define NCURSES_NOMACROS
#include <cursesw.h>

#include <algorithm>
#include <cassert>
#include <cstring>

namespace CppConsUI
{

namespace Curses
{

static Stats stats = {0, 0, 0};
bool ascii_mode = false;

ViewPort::ViewPort(int screen_x_, int screen_y_, int view_x_, int view_y_,
    int view_width_, int view_height_)
: screen_x(screen_x_), screen_y(screen_y_), view_x(view_x_), view_y(view_y_)
, view_width(view_width_), view_height(view_height_)
{
}

int ViewPort::addString(int x, int y, int w, const char *str, int *printed)
{
  assert(str);

  int res = OK;
  int p = 0;
  while (p < w && str && *str) {
    int out;
    if ((res = addChar(x + p, y, UTF8::getUniChar(str), &out)) == ERR)
      break;
    p += out;
    str = UTF8::getNextChar(str);
  }

  if (printed)
    *printed = p;

  return res;
}

int ViewPort::addString(int x, int y, const char *str, int *printed)
{
  assert(str);

  int res = OK;
  int p = 0;
  while (str && *str) {
    int out;
    if ((res = addChar(x + p, y, UTF8::getUniChar(str), &out)) == ERR)
      break;
    p += out;
    str = UTF8::getNextChar(str);
  }

  if (printed)
    *printed = p;

  return res;
}

int ViewPort::addString(int x, int y, int w, const char *str, const char *end,
    int *printed)
{
  assert(str);
  assert(end);

  int res = OK;
  int p = 0;
  while (p < w && str < end && str && *str) {
    int out;
    if ((res = addChar(x + p, y, UTF8::getUniChar(str), &out)) == ERR)
      break;
    p += out;
    str = UTF8::findNextChar(str, end);
  }

  if (printed)
    *printed = p;

  return res;
}

int ViewPort::addString(int x, int y, const char *str, const char *end,
    int *printed)
{
  assert(str);
  assert(end);

  int res = OK;
  int p = 0;
  while (str < end && str && *str) {
    int out;
    if ((res = addChar(x + p, y, UTF8::getUniChar(str), &out)) == ERR)
      break;
    p += out;
    str = UTF8::findNextChar(str, end);
  }

  if (printed)
    *printed = p;

  return res;
}

int ViewPort::addChar(int x, int y, UTF8::UniChar uc, int *printed)
{
  if (printed)
    *printed = 0;

  int draw_x = screen_x + (x - view_x);
  int draw_y = screen_y + (y - view_y);

  if (uc >= 0x7f && uc < 0xa0) {
    // filter out C1 (8-bit) control characters
    if (isInViewPort(x, y, 1))
      if (::mvaddch(draw_y, draw_x, '?') == ERR)
        return ERR;
    if (printed)
      *printed = 1;
    return OK;
  }

  // get a unicode character from the next few bytes
  wchar_t wch[2];

  wch[0] = uc;
  wch[1] = L'\0';

  // invalid utf-8 sequence
  if (wch[0] < 0)
    return ERR;

  // tab character
  if (wch[0] == '\t') {
    int w = onScreenWidth(wch[0]);
    for (int i = 0; i < w; ++i) {
      if (isInViewPort(x + i, y, 1))
        if (::mvaddch(draw_y, draw_x + i, ' ') == ERR)
          return ERR;
      if (printed)
        ++(*printed);
    }
    return OK;
  }

  // control char symbols
  if (wch[0] < 32)
    wch[0] = 0x2400 + wch[0];

  int w = onScreenWidth(wch[0]);
  if (isInViewPort(x, y, w)) {
    cchar_t cc;

    if (::setcchar(&cc, wch, A_NORMAL, 0, NULL) == ERR)
      return ERR;
    if (::mvadd_wch(draw_y, draw_x, &cc) == ERR)
      return ERR;
  }
  if (printed)
    *printed = w;
  return OK;
}

int ViewPort::addLineChar(int x, int y, LineChar c)
{
  if (!isInViewPort(x, y, 1))
    return OK;

  int draw_x = screen_x + (x - view_x);
  int draw_y = screen_y + (y - view_y);

  switch (c) {
    case LINE_HLINE:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? '-' : ACS_HLINE);
    case LINE_VLINE:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? '|' : ACS_VLINE);
    case LINE_LLCORNER:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? '+' : ACS_LLCORNER);
    case LINE_LRCORNER:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? '+' : ACS_LRCORNER);
    case LINE_ULCORNER:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? '+' : ACS_ULCORNER);
    case LINE_URCORNER:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? '+' : ACS_URCORNER);
    case LINE_BTEE:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? '+' : ACS_BTEE);
    case LINE_LTEE:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? '+' : ACS_LTEE);
    case LINE_RTEE:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? '+' : ACS_RTEE);
    case LINE_TTEE:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? '+' : ACS_TTEE);

    case LINE_DARROW:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? 'v' : ACS_DARROW);
    case LINE_LARROW:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? '<' : ACS_LARROW);
    case LINE_RARROW:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? '>' : ACS_RARROW);
    case LINE_UARROW:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? '^' : ACS_UARROW);
    case LINE_BULLET:
      return ::mvaddch(draw_y, draw_x, ascii_mode ? 'o' : ACS_BULLET);
  }
  return ERR;
}

int ViewPort::attrOn(int attrs)
{
  return ::attron(attrs);
}

int ViewPort::attrOff(int attrs)
{
  return ::attroff(attrs);
}

int ViewPort::changeAt(int x, int y, int n, /* attr_t */ int attr,
    short color, const void *opts)
{
  int res = OK;
  int draw_x, draw_y;
  for (int i = 0; i < n; i++) {
    if (!isInViewPort(x + i, y, 1))
      continue;

    draw_x = screen_x + (x + i - view_x);
    draw_y = screen_y + (y - view_y);
    if ((res = ::mvchgat(draw_y, draw_x, 1, attr, color, opts)) == ERR)
      break;
  }
  return res;
}

int ViewPort::fill(int attrs)
{
  attr_t battrs;
  short pair;

  if (::attr_get(&battrs, &pair, NULL) == ERR)
    return ERR;

  if (::attron(attrs) == ERR)
    return ERR;

  for (int i = 0; i < view_height; ++i)
    for (int j = 0; j < view_width; ++j) {
      ///< @todo Implement correct error checking.
      addChar(j, i, ' ');
    }

  if (attr_set(battrs, pair, NULL) == ERR)
    return ERR;

  return OK;
}

int ViewPort::fill(int attrs, int x, int y, int w, int h)
{
  attr_t battrs;
  short pair;

  if (attr_get(&battrs, &pair, NULL) == ERR)
    return ERR;

  if (attron(attrs) == ERR)
    return ERR;

  for (int i = 0; i < h; ++i)
    for (int j = 0; j < w; ++j) {
      ///< @todo Implement correct error checking.
      addChar(x + j, y + i, ' ');
    }

  if (attr_set(battrs, pair, NULL) == ERR)
    return ERR;

  return OK;
}

int ViewPort::erase()
{
  return fill(0);
}

void ViewPort::scroll(int scroll_x, int scroll_y)
{
  view_x += scroll_x;
  view_y += scroll_y;
}

bool ViewPort::isInViewPort(int x, int y, int w)
{
  return x >= view_x && y >= view_y
    && x + w <= view_x + view_width
    && y < view_y + view_height;
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

int initScreen()
{
  if (!::initscr())
    return ERR;

  if (::has_colors()) {
    if (::start_color() == ERR)
      return ERR;
    if (::use_default_colors() == ERR)
      return ERR;
  }
  if (::curs_set(0) == ERR)
    return ERR;
  if (::nonl() == ERR)
    return ERR;
  if (::raw() == ERR)
    return ERR;
  return OK;
}

int finalizeScreen()
{
  return ::endwin();
}

void setAsciiMode(bool enabled)
{
  ascii_mode = enabled;
}

bool getAsciiMode()
{
  return ascii_mode;
}

bool initColorPair(int idx, int fg, int bg, int *res)
{
  assert(res);

  bool success = (init_pair(idx, fg, bg) == OK);

  if (success)
    *res = COLOR_PAIR(idx);

  return success;
}

int getColorCount()
{
  return COLORS;
}

int getColorPairCount()
{
#ifndef NCURSES_EXT_COLORS
  /* Ncurses reports more than 256 color pairs, even when compiled without
   * ext-color. */
  return std::min(COLOR_PAIRS, 256);
#else
  return COLOR_PAIRS;
#endif
}

#ifdef DEBUG
bool getColorPair(int colorpair, int *fg, int *bg)
{
  short sfg, sbg;

  int ret = pair_content(PAIR_NUMBER(colorpair), &sfg, &sbg);

  *fg = sfg;
  *bg = sbg;

  return ret;
}
#endif // DEBUG

int erase()
{
  return ::erase();
}

int clear()
{
  return ::clear();
}

int refresh()
{
  return ::refresh();
}

int beep()
{
  return ::beep();
}

int getWidth()
{
  return ::getmaxx(stdscr);
}

int getHeight()
{
  return ::getmaxy(stdscr);
}

int resizeTerm(int width, int height)
{
  return ::resizeterm(height, width);
}

/// @todo Should be g_unichar_iszerowidth() used?
int onScreenWidth(const char *start, const char *end)
{
  int width = 0;

  if (!start)
    return 0;

  if (!end)
    end = start + std::strlen(start);

  while (start < end) {
    width += onScreenWidth(UTF8::getUniChar(start));
    start = UTF8::getNextChar(start);
  }
  return width;
}

int onScreenWidth(UTF8::UniChar uc, int w)
{
  if (uc == '\t')
    return 8 - w % 8;
  return UTF8::isUniCharWide(uc) ? 2 : 1;
}

const Stats *getStats()
{
  return &stats;
}

void resetStats()
{
  memset(&stats, 0, sizeof(stats));
}

} // namespace Curses

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
