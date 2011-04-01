/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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
 * Window class implementation.
 *
 * @ingroup cppconsui
 */

#include "Window.h"

Window::Window(int x, int y, int w, int h, const char *title, Type t,
    LineStyle::Type ltype)
: FreeWindow(x, y, w, h, t)
{
  panel = new Panel(win_w, win_h, title, ltype);
  AddWidget(*panel, 0, 0);
}

void Window::MoveResize(int newx, int newy, int neww, int newh)
{
  if (newx == win_x && newy == win_y && neww == win_w && newh == win_h)
    return;

  win_x = newx;
  win_y = newy;
  win_w = neww;
  win_h = newh;

  panel->MoveResize(0, 0, win_w, win_h);

  Container::MoveResize(1, 1, win_w < 2 ? 0 : win_w - 2,
      win_h < 2 ? 0 : win_h - 2);
  UpdateArea();
}

Point Window::GetAbsolutePosition(const Container& ref,
    const Widget& child) const
{
  g_assert(child.GetParent() == this);

  if (this == &ref) {
    if (&child == panel)
      return Point(0, 0);

    return Point(child.GetLeft() + 1, child.GetTop() + 1);
  }

  if (&child == panel)
    return Point(win_x, win_y);

  return Point(win_x + child.GetLeft() + 1, win_y + child.GetTop() + 1);
}

Point Window::GetAbsolutePosition(const Widget& child) const
{
  g_assert(child.GetParent() == this);

  if (&child == panel)
    return Point(win_x, win_y);

  return Point(win_x + child.GetLeft() + 1, win_y + child.GetTop() + 1);
}

Curses::Window *Window::GetSubPad(const Widget &child, int begin_x,
    int begin_y, int ncols, int nlines)
{
  if (!area)
    return NULL;

  // handle panel child specially
  if (&child == panel)
    return area->subpad(begin_x, begin_y, ncols, nlines);

  int realw = area->getmaxx() - 2;
  int realh = area->getmaxy() - 2;

  /* Extend requested subpad to whole panel area or shrink requested area if
   * necessary. */
  if (nlines == AUTOSIZE || nlines > realh - begin_y)
    nlines = realh - begin_y;

  if (ncols == AUTOSIZE || ncols > realw - begin_x)
    ncols = realw - begin_x;

  // add `+1' offset to normal childs so they can not overwrite the panel
  return area->subpad(begin_x + 1, begin_y + 1, ncols, nlines);
}

void Window::SetBorderStyle(LineStyle::Type ltype)
{
  panel->SetBorderStyle(ltype);
}

LineStyle::Type Window::GetBorderStyle() const
{
  return panel->GetBorderStyle();
}
