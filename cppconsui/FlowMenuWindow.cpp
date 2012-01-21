/*
 * Copyright (C) 2012 by CenterIM developers
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
 * FlowMenuWindow class implementation.
 *
 * @ingroup cppconsui
 */

#include "FlowMenuWindow.h"

namespace CppConsUI
{

FlowMenuWindow::FlowMenuWindow(Widget& ref_, int w, int h, const char *title)
: MenuWindow(0, 0, w, h, title), ref(&ref_), xshift(0), yshift(0)
{
  ref_visible_conn = ref->signal_visible.connect(sigc::mem_fun(this,
        &FlowMenuWindow::OnRefVisible));
}

void FlowMenuWindow::Draw()
{
  UpdateSmartPositionAndSize();

  MenuWindow::Draw();
}

void FlowMenuWindow::SetLeftShift(int x)
{
  if (xshift == x)
    return;

  xshift = x;
  Redraw();
}

void FlowMenuWindow::SetTopShift(int y)
{
  if (yshift == y)
    return;

  yshift = y;
  Redraw();
}

void FlowMenuWindow::UpdateSmartPositionAndSize()
{
  /* This code is called when position or size of this window should be
   * updated.
   *
   * This can happen when:
   * - the screen is resized,
   * - listbox wish height changed,
   * - reference widget changed its position on the screen.
   *
   * Unfortunately the position of the reference widget isn't known until the
   * widget is drawn on the screen. This happens just before
   * FlowMenuWindow::Draw() is called thus this method has to be ultimately
   * called from that method.
   *
   * Note that none of the below called methods (Move(), SetWishHeight())
   * doesn't trigger update-area procedure if it isn't really necessary.
   */
  Point p = ref->GetAbsolutePosition();
  int x = p.GetX() + xshift;
  int y = p.GetY() + yshift;
  int above = y;
  int below = Curses::getmaxy() - y - 1;
  int req_h;
  if (win_h == AUTOSIZE)
    req_h = listbox->GetChildrenHeight() + 2;
  else
    req_h = win_h;

  if (below > req_h) {
    // draw the window under the combobox
    Move(x, y + 1);
    SetWishHeight(req_h);
  }
  else if (above > req_h) {
    // draw the window above the combobox
    Move(x, y - req_h);
    SetWishHeight(req_h);
  }
  else if (win_h == AUTOSIZE) {
    if (below >= above) {
      Move(x, y + 1);
      SetWishHeight(below);
    }
    else {
      Move(x, 0);
      SetWishHeight(above);
    }
  }
}

void FlowMenuWindow::OnRefVisible(Widget& activator, bool visible)
{
  if (visible)
    return;

  // hide window if the reference widget is hidden
  Close();
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab */
