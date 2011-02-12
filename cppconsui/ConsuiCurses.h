/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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

/**
 * @file
 * Wrapper for curses functions.
 *
 * @ingroup cppconsui
 *
 * @todo Make ConsuiCurses.h a full handler of all curses functions that could
 * be easily changed with a different implementation.
 * @todo Documentation. ;)
 */

#ifndef __CONSUICURSES_H__
#define __CONSUICURSES_H__

#include <glib.h>

namespace Curses
{
struct Stats {
  unsigned newpad_calls;
  unsigned newwin_calls;
  unsigned subpad_calls;
};

const Stats *GetStats();
void ResetStats();

class Window
{
public:
  // these tree functions returns NULL if such pad/window can not be created
  static Window *newpad(int cols, int nlines);
  static Window *newwin(int begin_x, int begin_y, int ncols, int nlines);

  Window *subpad(int begin_x, int begin_y, int ncols, int nlines);

  virtual ~Window();

  /**
   * This functions take a formatted string and draws it on the screen. The
   * formatting of the string happens when a '\\v' is encountered. After the
   * '\\v' is a char, a switch in the code figures out what to do based on
   * this. Based on giFTcurs drawing function.
   */
  int mvaddstring(int x, int y, int w, const gchar *str);
  int mvaddstring(int x, int y, const gchar *str);
  int mvaddstring(int x, int y, int w, const gchar *str, const gchar *end);
  int mvaddstring(int x, int y, const gchar *str, const gchar *end);

  // @todo remove
  int mvaddstr(int x, int y, const char *str);
  int mvaddnstr(int x, int y, const char *str, int n);

  int attron(int attrs);
  int attroff(int attrs);
  int mvchgat(int x, int y, int n, /* attr_t */ int attr, short color, const
      void *opts);

  int fill(int attrs);
  int erase();
  int clrtoeol();
  int clrtobot();

  int noutrefresh();

  int touch();

  int copyto(Window *dstwin, int smincol, int sminrow, int dmincol,
      int dminrow, int dmaxcol, int dmaxrow, int overlay);

  int getmaxx();
  int getmaxy();

protected:
  const gchar *PrintChar(const gchar *ch, int *printed,
      const gchar *end = NULL);

private:
  struct WindowInternals;
  WindowInternals *p;

  Window();
  Window(const Window &other);
  Window &operator=(const Window &other);
};

struct Color
{
  const static int MIN;
  const static int BLACK;
  const static int RED;
  const static int GREEN;
  const static int YELLOW;
  const static int BLUE;
  const static int MAGENTA;
  const static int CYAN;
  const static int WHITE;
  const static int MAX;
};

struct Attr
{
  const static int NORMAL;
  const static int REVERSE;
  const static int DIM;
  const static int BOLD;
};

extern const int C_OK;
extern const int C_ERR;

int getcolorpair(int fg, int bg);

int erase();
int doupdate();

int beep();

/// @todo Add noraw to raw, nl to nonl etc.
int initscr();
int endwin();
bool has_colors();
int start_color();
int curs_set(int visibility);
int nonl();
int raw();

// stdscr
int noutrefresh();
int getmaxx();
int getmaxy();

int resizeterm(int lines, int columns);

int onscreen_width(const char *start, const char *end = NULL);
int onscreen_width(gunichar uc);

} // namespace Curses

#endif // __CONSUICURSES_H__
