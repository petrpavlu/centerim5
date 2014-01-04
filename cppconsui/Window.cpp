/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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
 * Window class implementation.
 *
 * @ingroup cppconsui
 */

#include "Window.h"

#include <algorithm>
#include <cassert>

namespace CppConsUI
{

Window::Window(int x, int y, int w, int h, const char *title, Type t)
: FreeWindow(x, y, w, h, t)
{
  panel = new Panel(win_w, win_h, title);
  addWidget(*panel, 0, 0);
}

void Window::moveResize(int newx, int newy, int neww, int newh)
{
  if (newx == win_x && newy == win_y && neww == win_w && newh == win_h)
    return;

  win_x = newx;
  win_y = newy;
  win_w = neww;
  win_h = newh;

  resizeAndUpdateArea();
}

Point Window::getAbsolutePosition(const Container& ref,
    const Widget& child) const
{
  assert(child.getParent() == this);

  if (this == &ref) {
    if (&child == panel)
      return Point(0, 0);

    return Point(child.getLeft() + 1, child.getTop() + 1);
  }

  if (&child == panel)
    return Point(win_x, win_y);

  return Point(win_x + child.getLeft() + 1, win_y + child.getTop() + 1);
}

Point Window::getAbsolutePosition(const Widget& child) const
{
  assert(child.getParent() == this);

  if (&child == panel)
    return Point(win_x, win_y);

  return Point(win_x + child.getLeft() + 1, win_y + child.getTop() + 1);
}

Curses::Window *Window::getSubPad(const Widget &child, int begin_x,
    int begin_y, int ncols, int nlines)
{
  if (!area)
    return NULL;

  // handle panel child specially
  if (&child == panel)
    return area->subpad(begin_x, begin_y, ncols, nlines);

  int realw = area->getmaxx() - 2;
  int realh = area->getmaxy() - 2;

  if (nlines == AUTOSIZE)
    nlines = child.getWishHeight();
  if (ncols == AUTOSIZE)
    ncols = child.getWishWidth();

  /* Extend requested subpad to whole panel area or shrink requested area if
   * necessary. */
  if (nlines == AUTOSIZE || nlines > realh - begin_y)
    nlines = realh - begin_y;

  if (ncols == AUTOSIZE || ncols > realw - begin_x)
    ncols = realw - begin_x;

  if (nlines <= 0 || ncols <= 0)
    return NULL;

  // add '+1' offset to normal childs so they can not overwrite the panel
  return area->subpad(begin_x + 1, begin_y + 1, ncols, nlines);
}

void Window::resizeAndUpdateArea()
{
  int realw = win_w;
  if (realw == AUTOSIZE) {
    realw = getWishWidth();
    if (realw == AUTOSIZE)
      realw = Curses::getmaxx() - win_x;
  }
  int realh = win_h;
  if (realh == AUTOSIZE) {
    realh = getWishHeight();
    if (realh == AUTOSIZE)
      realh = Curses::getmaxy() - win_y;
  }

  panel->moveResize(0, 0, realw, realh);

  Container::moveResize(1, 1, std::max(0, realw - 2), std::max(0, realh - 2));
  updateArea();
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
