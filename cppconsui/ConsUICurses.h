// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// Wrapper for curses functions.
///
/// @ingroup cppconsui

#ifndef __CONSUICURSES_H__
#define __CONSUICURSES_H__

#include "CppConsUI.h"

namespace CppConsUI {

namespace Curses {
struct Stats {
  unsigned newpad_calls;
  unsigned newwin_calls;
  unsigned subpad_calls;
};

enum LineChar {
  LINE_HLINE,
  LINE_VLINE,
  LINE_LLCORNER,
  LINE_LRCORNER,
  LINE_ULCORNER,
  LINE_URCORNER,
  LINE_BTEE,
  LINE_LTEE,
  LINE_RTEE,
  LINE_TTEE,
  LINE_DARROW,
  LINE_LARROW,
  LINE_RARROW,
  LINE_UARROW,
  LINE_BULLET,
};

class ViewPort {
public:
  ViewPort(int screen_x, int screen_y, int view_x, int view_y, int view_width,
    int view_height);
  virtual ~ViewPort() {}

  /// Adds a string to the screen.
  ///
  /// First two variants require NUL-terminated strings.
  int addString(
    int x, int y, int w, const char *str, Error &error, int *printed = NULL);
  int addString(
    int x, int y, const char *str, Error &error, int *printed = NULL);
  int addString(int x, int y, int w, const char *str, const char *end,
    Error &error, int *printed = NULL);
  int addString(int x, int y, const char *str, const char *end, Error &error,
    int *printed = NULL);

  int addChar(
    int x, int y, UTF8::UniChar uc, Error &error, int *printed = NULL);
  int addLineChar(int x, int y, LineChar c, Error &error);

  int attrOn(int attrs, Error &error);
  int attrOff(int attrs, Error &error);
  int changeAt(int x, int y, int n, /* attr_t */ unsigned long attr,
    short color, Error &error);

  int fill(int attrs, Error &error);
  int fill(int attrs, int x, int y, int w, int h, Error &error);
  int erase(Error &error);

  void scroll(int scroll_x, int scroll_y);

  int getScreenLeft() const { return screen_x_; }
  int getScreenTop() const { return screen_y_; }
  int getViewLeft() const { return view_x_; }
  int getViewTop() const { return view_y_; }
  int getViewWidth() const { return view_width_; }
  int getViewHeight() const { return view_height_; }

protected:
  int screen_x_, screen_y_;
  int view_x_, view_y_, view_width_, view_height_;

  bool isInViewPort(int x, int y, int w);

private:
  // CONSUI_DISABLE_COPY(ViewPort);
};

struct Color {
  const static int DEFAULT;
  const static int BLACK;
  const static int RED;
  const static int GREEN;
  const static int YELLOW;
  const static int BLUE;
  const static int MAGENTA;
  const static int CYAN;
  const static int WHITE;
};

struct Attr {
  const static int NORMAL;
  const static int STANDOUT;
  const static int REVERSE;
  const static int BLINK;
  const static int DIM;
  const static int BOLD;
};

const int NUM_DEFAULT_COLORS = 16;

int initScreen(Error &error);
int finalizeScreen(Error &error);
void setAsciiMode(bool enabled);
bool getAsciiMode();

bool initColorPair(int idx, int fg, int bg, int *res, Error &error);
int getColorCount();
int getColorPairCount();

int erase(Error &error);
int clear(Error &error);
int refresh(Error &error);

int beep(Error &error);

// stdscr
int getWidth();
int getHeight();

int resizeTerm(int width, int height, Error &error);

int onScreenWidth(const char *start, const char *end = NULL);
int onScreenWidth(UTF8::UniChar uc, int w = 0);

const Stats *getStats();
void resetStats();

} // namespace Curses

} // namespace CppConsUI

#endif // __CONSUICURSES_H__

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
