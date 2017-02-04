// Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
//
// This file is part of CenterIM.
//
// CenterIM is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// CenterIM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// Hidden implementation of curses specific functions.
///
/// @ingroup cppconsui

#include "ConsUICurses.h"

// Define _XOPEN_SOURCE_EXTENDED to get wide character support.
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif

#define NCURSES_NOMACROS
#include <cursesw.h>

#include "gettext.h"
#include <algorithm>
#include <cassert>
#include <cstring>

namespace CppConsUI {

namespace Curses {

namespace {

SCREEN *screen = nullptr;
int screen_width = 0;
int screen_height = 0;
bool ascii_mode = false;

void updateScreenSize()
{
  screen_width = ::getmaxx(stdscr);
  assert(screen_width != ERR);
  screen_height = ::getmaxy(stdscr);
  assert(screen_height != ERR);
}

} // anonymous namespace

ViewPort::ViewPort(int screen_x, int screen_y, int view_x, int view_y,
  int view_width, int view_height)
  : screen_x_(screen_x), screen_y_(screen_y), view_x_(view_x), view_y_(view_y),
    view_width_(view_width), view_height_(view_height)
{
}

int ViewPort::addString(
  int x, int y, int w, const char *str, Error &error, int *printed)
{
  assert(str != nullptr);

  int res = 0;
  int p = 0;
  while (p < w && str != nullptr && *str != '\0') {
    int out;
    if ((res = addChar(x + p, y, UTF8::getUniChar(str), error, &out)) != 0)
      break;
    p += out;
    str = UTF8::getNextChar(str);
  }

  if (printed != nullptr)
    *printed = p;

  return res;
}

int ViewPort::addString(
  int x, int y, const char *str, Error &error, int *printed)
{
  assert(str != nullptr);

  int res = 0;
  int p = 0;
  while (str != nullptr && *str != '\0') {
    int out;
    if ((res = addChar(x + p, y, UTF8::getUniChar(str), error, &out)) != 0)
      break;
    p += out;
    str = UTF8::getNextChar(str);
  }

  if (printed != nullptr)
    *printed = p;

  return res;
}

int ViewPort::addString(int x, int y, int w, const char *str, const char *end,
  Error &error, int *printed)
{
  assert(str != nullptr);
  assert(end != nullptr);

  int res = 0;
  int p = 0;
  while (p < w && str != nullptr && str < end && *str != '\0') {
    int out;
    if ((res = addChar(x + p, y, UTF8::getUniChar(str), error, &out)) != 0)
      break;
    p += out;
    str = UTF8::findNextChar(str, end);
  }

  if (printed != nullptr)
    *printed = p;

  return res;
}

int ViewPort::addString(
  int x, int y, const char *str, const char *end, Error &error, int *printed)
{
  assert(str != nullptr);
  assert(end != nullptr);

  int res = 0;
  int p = 0;
  while (str != nullptr && str < end && *str != '\0') {
    int out;
    if ((res = addChar(x + p, y, UTF8::getUniChar(str), error, &out)) != 0)
      break;
    p += out;
    str = UTF8::findNextChar(str, end);
  }

  if (printed != nullptr)
    *printed = p;

  return res;
}

int ViewPort::addChar(
  int x, int y, UTF8::UniChar uc, Error &error, int *printed)
{
  if (printed)
    *printed = 0;

  int draw_x = screen_x_ + (x - view_x_);
  int draw_y = screen_y_ + (y - view_y_);
  chtype ch;

  // Filter out C1 (8-bit) control characters.
  if (uc >= 0x7f && uc < 0xa0) {
    if (isInViewPort(x, y, 1)) {
      ch = '?';
      if (::mvaddchnstr(draw_y, draw_x, &ch, 1) == ERR) {
        error = Error(ERROR_CURSES_ADD_CHARACTER);
        error.setFormattedString(
          _("Adding character '?' on screen at position (x=%d, y=%d) failed."),
          draw_x, draw_y);
        return error.getCode();
      }
    }
    if (printed != nullptr)
      *printed = 1;
    return 0;
  }

  // Handle tab characters.
  if (uc == '\t') {
    int w = onScreenWidth(uc);
    for (int i = 0; i < w; ++i) {
      if (isInViewPort(x + i, y, 1)) {
        ch = ' ';
        if (::mvaddchnstr(draw_y, draw_x + i, &ch, 1) == ERR) {
          error = Error(ERROR_CURSES_ADD_CHARACTER);
          error.setFormattedString(
            _("Adding character ' ' on screen at position (x=%d, y=%d) "
              "failed."),
            draw_x, draw_y);
          return error.getCode();
        }
      }
      if (printed != nullptr)
        ++(*printed);
    }
    return 0;
  }

  // Make control chars printable.
  if (uc < 32)
    uc += 0x2400;

  // Create a wide-character string accepted by ncurses.
  wchar_t wch[2];
  wch[0] = uc;
  wch[1] = '\0';

  int w = onScreenWidth(uc);
  if (isInViewPort(x, y, w)) {
    cchar_t cc;

    if (::setcchar(&cc, wch, A_NORMAL, 0, nullptr) == ERR) {
      error = Error(ERROR_CURSES_ADD_CHARACTER);
      error.setFormattedString(
        _("Setting complex character from Unicode character "
          "#%" UNICHAR_FORMAT "failed."),
        uc);
      return error.getCode();
    }
    if (::mvadd_wchnstr(draw_y, draw_x, &cc, 1) == ERR) {
      error.setFormattedString(
        _("Adding Unicode character #%" UNICHAR_FORMAT " on screen at position "
          "(x=%d, y=%d) failed."),
        uc, draw_x, draw_y);
      return error.getCode();
    }
  }
  if (printed != nullptr)
    *printed = w;
  return 0;
}

int ViewPort::addLineChar(int x, int y, LineChar c, Error &error)
{
  if (!isInViewPort(x, y, 1))
    return 0;

  cchar_t cc;
  cchar_t *ccp = &cc;

  if (ascii_mode) {
    char ch;

    switch (c) {
    case LINE_HLINE:
      ch = '-';
      break;
    case LINE_VLINE:
      ch = '|';
      break;
    case LINE_LLCORNER:
    case LINE_LRCORNER:
    case LINE_ULCORNER:
    case LINE_URCORNER:
    case LINE_BTEE:
    case LINE_LTEE:
    case LINE_RTEE:
    case LINE_TTEE:
      ch = '+';
      break;
    case LINE_DARROW:
      ch = 'v';
      break;
    case LINE_LARROW:
      ch = '<';
      break;
    case LINE_RARROW:
      ch = '>';
      break;
    case LINE_UARROW:
      ch = '^';
      break;
    case LINE_BULLET:
      ch = 'o';
      break;
    default:
      assert(0);
    }

    wchar_t wch[2];
    wch[0] = ch;
    wch[1] = '\0';

    if (::setcchar(&cc, wch, A_NORMAL, 0, nullptr) == ERR) {
      error = Error(ERROR_CURSES_ADD_CHARACTER);
      error.setFormattedString(
        _("Setting complex character from character '%c' failed."), ch);
      return error.getCode();
    }
  }
  else {
    // Non-ASCII mode.
    switch (c) {
    case LINE_HLINE:
      ccp = WACS_HLINE;
      break;
    case LINE_VLINE:
      ccp = WACS_VLINE;
      break;
    case LINE_LLCORNER:
      ccp = WACS_LLCORNER;
      break;
    case LINE_LRCORNER:
      ccp = WACS_LRCORNER;
      break;
    case LINE_ULCORNER:
      ccp = WACS_ULCORNER;
      break;
    case LINE_URCORNER:
      ccp = WACS_URCORNER;
      break;
    case LINE_BTEE:
      ccp = WACS_BTEE;
      break;
    case LINE_LTEE:
      ccp = WACS_LTEE;
      break;
    case LINE_RTEE:
      ccp = WACS_RTEE;
      break;
    case LINE_TTEE:
      ccp = WACS_TTEE;
      break;
    case LINE_DARROW:
      ccp = WACS_DARROW;
      break;
    case LINE_LARROW:
      ccp = WACS_LARROW;
      break;
    case LINE_RARROW:
      ccp = WACS_RARROW;
      break;
    case LINE_UARROW:
      ccp = WACS_UARROW;
      break;
    case LINE_BULLET:
      ccp = WACS_BULLET;
      break;
    default:
      assert(0);
    }
  }

  int draw_x = screen_x_ + (x - view_x_);
  int draw_y = screen_y_ + (y - view_y_);

  if (::mvadd_wchnstr(draw_y, draw_x, ccp, 1) == OK)
    return 0;

  const char *name;
  switch (c) {
  case LINE_HLINE:
    name = "HLINE";
    break;
  case LINE_VLINE:
    name = "VLINE";
    break;
  case LINE_LLCORNER:
    name = "LLCORNER";
    break;
  case LINE_LRCORNER:
    name = "LRCORNER";
    break;
  case LINE_ULCORNER:
    name = "ULCORNER";
    break;
  case LINE_URCORNER:
    name = "URCORNER";
    break;
  case LINE_BTEE:
    name = "BTEE";
    break;
  case LINE_LTEE:
    name = "LTEE";
    break;
  case LINE_RTEE:
    name = "RTEE";
    break;
  case LINE_TTEE:
    name = "TTEE";
    break;
  case LINE_DARROW:
    name = "DARROW";
    break;
  case LINE_LARROW:
    name = "LARROW";
    break;
  case LINE_RARROW:
    name = "RARROW";
    break;
  case LINE_UARROW:
    name = "UARROW";
    break;
  case LINE_BULLET:
    name = "BULLET";
    break;
  default:
    assert(0);
  }

  error = Error(ERROR_CURSES_ADD_CHARACTER);
  error.setFormattedString(
    _("Adding line character %s on screen at position (x=%d, y=%d) failed."),
    name, draw_x, draw_y);
  return error.getCode();
}

int ViewPort::attrOn(int attrs, Error &error)
{
  if (::attron(attrs) == OK)
    return 0;

  error = Error(ERROR_CURSES_ATTR);
  error.setFormattedString(
    _("Turning on window attributes '%#x' failed."), attrs);
  return error.getCode();
}

int ViewPort::attrOff(int attrs, Error &error)
{
  if (::attroff(attrs) == OK)
    return 0;

  error = Error(ERROR_CURSES_ATTR);
  error.setFormattedString(
    _("Turning off window attributes '%#x' failed."), attrs);
  return error.getCode();
}

int ViewPort::changeAt(int x, int y, int n, /* attr_t */ unsigned long attr,
  short color, Error &error)
{
  for (int i = 0; i < n; ++i) {
    if (!isInViewPort(x + i, y, 1))
      continue;

    int draw_x = screen_x_ + (x + i - view_x_);
    int draw_y = screen_y_ + (y - view_y_);
    if (::mvchgat(draw_y, draw_x, 1, attr, color, nullptr) == ERR) {
      error = Error(ERROR_CURSES_ATTR);
      error.setFormattedString(
        _("Changing window attributes to '%#lx' and color pair to '%d' on "
          "screen at position (x=%d, y=%d) failed."),
        attr, color, draw_x, draw_y);
      return error.getCode();
    }
  }
  return 0;
}

int ViewPort::fill(int attrs, Error &error)
{
  return fill(attrs, 0, 0, view_width_, view_height_, error);
}

int ViewPort::fill(int attrs, int x, int y, int w, int h, Error &error)
{
  attr_t battrs;
  short pair;

  if (::attr_get(&battrs, &pair, nullptr) == ERR) {
    error = Error(ERROR_CURSES_ATTR, _("Obtaining window attributes failed."));
    return error.getCode();
  }

  if (attrOn(attrs, error) != 0)
    return error.getCode();

  for (int i = 0; i < h; ++i)
    for (int j = 0; j < w; ++j)
      if (addChar(x + j, y + i, ' ', error) != 0)
        return error.getCode();

  if (::attr_set(battrs, pair, nullptr) == ERR) {
    error = Error(ERROR_CURSES_ATTR);
    error.setFormattedString(
      _("Setting window attributes to '%#lx' and color pair to '%d' failed."),
      static_cast<unsigned long>(battrs), pair);
    return error.getCode();
  }

  return 0;
}

int ViewPort::erase(Error &error)
{
  return fill(0, error);
}

void ViewPort::scroll(int scroll_x, int scroll_y)
{
  view_x_ += scroll_x;
  view_y_ += scroll_y;
}

bool ViewPort::isInViewPort(int x, int y, int w)
{
  // Check that the given area fits in the view port.
  return x >= view_x_ && y >= view_y_ && x + w <= view_x_ + view_width_ &&
    y < view_y_ + view_height_;
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

int initScreen(Error &error)
{
  assert(screen == nullptr);

  screen = ::newterm(nullptr, stdout, stdin);
  if (screen == nullptr) {
    error = Error(ERROR_CURSES_INITIALIZATION,
      _("Initialization of the terminal for Curses session failed."));
    return error.getCode();
  }

  if (::has_colors()) {
    if (::start_color() == ERR) {
      error = Error(ERROR_CURSES_INITIALIZATION,
        _("Initialization of color support failed."));
      goto error_out;
    }
    if (::use_default_colors() == ERR) {
      error = Error(ERROR_CURSES_INITIALIZATION,
        _("Initialization of default colors failed."));
      goto error_out;
    }
  }
  if (::curs_set(0) == ERR) {
    error = Error(ERROR_CURSES_INITIALIZATION, _("Hiding the cursor failed."));
    goto error_out;
  }
  if (::nonl() == ERR) {
    error = Error(
      ERROR_CURSES_INITIALIZATION, _("Disabling newline translation failed."));
    goto error_out;
  }
  if (::raw() == ERR) {
    error = Error(ERROR_CURSES_INITIALIZATION,
      _("Placing the terminal into raw mode failed."));
    goto error_out;
  }

  updateScreenSize();

  return 0;

error_out:
  // Try to destroy the already created screen.
  ::endwin();
  ::delscreen(screen);
  screen = nullptr;

  return error.getCode();
}

int finalizeScreen(Error &error)
{
  assert(screen != nullptr);

  // Note: This function can fail in three places: clear(), refresh() and
  // endwin(). The first two are non-critical and the function proceeds even if
  // they occur. Error in endwin() is potentially serious and should always
  // override any error from clear() or refresh().

  bool has_error;

  // Clear the screen.
  if (clear(error) != 0)
    has_error = true;
  if (refresh(error) != 0)
    has_error = true;

  if (::endwin() == ERR) {
    error = Error(
      ERROR_CURSES_FINALIZATION, _("Finalization of Curses session failed."));
    has_error = true;
  }

  ::delscreen(screen);
  screen = nullptr;

  return has_error ? error.getCode() : 0;
}

void setAsciiMode(bool enabled)
{
  ascii_mode = enabled;
}

bool getAsciiMode()
{
  return ascii_mode;
}

bool initColorPair(int idx, int fg, int bg, int *res, Error &error)
{
  assert(res != nullptr);

  int color_pair_count = Curses::getColorPairCount();
  if (idx > color_pair_count) {
    error = Error(ERROR_CURSES_COLOR_LIMIT);
    error.setFormattedString(
      _("Adding of color pair '%d' (foreground=%d, background=%d) failed "
        "because color pair limit of '%d' was exceeded."),
      idx, fg, bg, color_pair_count);
    return error.getCode();
  }

  if (::init_pair(idx, fg, bg) == ERR) {
    error = Error(ERROR_CURSES_COLOR_INIT);
    error.setFormattedString(
      _("Initialization of color pair '%d' to (foreground=%d, background=%d) "
        "failed."),
      idx, fg, bg);
    return error.getCode();
  }

  *res = COLOR_PAIR(idx);
  return 0;
}

int getColorCount()
{
  return COLORS;
}

int getColorPairCount()
{
#ifndef NCURSES_EXT_COLORS
  // Ncurses reports more than 256 color pairs, even when compiled without
  // ext-color.
  return std::min(COLOR_PAIRS, 256);
#else
  return COLOR_PAIRS;
#endif
}

int erase(Error &error)
{
  if (::erase() == ERR) {
    error = Error(ERROR_CURSES_CLEAR, _("Erasing the screen failed."));
    return error.getCode();
  }
  return 0;
}

int clear(Error &error)
{
  if (::clear() == ERR) {
    error = Error(ERROR_CURSES_CLEAR, _("Clearing the screen failed."));
    return error.getCode();
  }
  return 0;
}

int refresh(Error &error)
{
  if (::refresh() == ERR) {
    error = Error(ERROR_CURSES_REFRESH, _("Refreshing the screen failed."));
    return error.getCode();
  }
  return 0;
}

int beep(Error &error)
{
  if (::beep() == ERR) {
    error = Error(ERROR_CURSES_BEEP, _("Producing beep alert failed."));
    return error.getCode();
  }
  return 0;
}

int getWidth()
{
  return screen_width;
}

int getHeight()
{
  return screen_height;
}

int resizeTerm(int width, int height, Error &error)
{
  if (::resizeterm(height, width) == ERR) {
    error = Error(ERROR_CURSES_RESIZE);
    error.setFormattedString(
      _("Changing the Curses terminal size to (width=%d, height=%d) failed."),
      width, height);
    return error.getCode();
  }

  updateScreenSize();

  return 0;
}

int onScreenWidth(const char *start, const char *end)
{
  int width = 0;

  if (start == nullptr)
    return 0;

  if (end == nullptr)
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

} // namespace Curses

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
