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
 * MenuWindow class implementation.
 *
 * @ingroup cppconsui
 */

#include "MenuWindow.h"

namespace CppConsUI
{

MenuWindow::MenuWindow(int x, int y, int w, int h, const char *title)
: Window(x, y, w, h, title, TYPE_TOP)
, wish_height(3), ref(NULL), xshift(0), yshift(0), flags(0)
{
  wish_width = MENU_WINDOW_WISH_WIDTH;

  listbox = new ListBox(AUTOSIZE, AUTOSIZE);
  listbox->signal_children_height_change.connect(sigc::mem_fun(this,
        &MenuWindow::OnChildrenHeightChange));
  Window::AddWidget(*listbox, 0, 0);
}

MenuWindow::MenuWindow(Widget& ref_, int w, int h, const char *title)
: Window(0, 0, w, h, title, TYPE_TOP)
, wish_height(3), ref(&ref_), xshift(0), yshift(0), flags(0)
{
  wish_width = MENU_WINDOW_WISH_WIDTH;

  listbox = new ListBox(AUTOSIZE, AUTOSIZE);
  listbox->signal_children_height_change.connect(sigc::mem_fun(this,
        &MenuWindow::OnChildrenHeightChange));
  Window::AddWidget(*listbox, 0, 0);

  SetRef(ref);
}

void MenuWindow::Draw()
{
  UpdateSmartPositionAndSize();

  Window::Draw();
}

void MenuWindow::Close()
{
  if (flags & FLAG_HIDE_ON_CLOSE) {
    Hide();
  } else {
    Window::Close();
  }
}

void MenuWindow::SetFlags(int new_flags)
{
  if (flags == new_flags)
    return;

  flags = new_flags;
  Redraw();
}

void MenuWindow::SetRef(Widget *ref_)
{
  /* Disconnect old ref_visible_conn signal. */
  if (ref) {
    ref_visible_conn.disconnect();
  }

  ref = ref_;

  /* Connect new ref_visible_conn, if any. */
  if (ref) {
    ref_visible_conn = ref->signal_visible.connect(sigc::mem_fun(this,
        &MenuWindow::OnRefVisible));
  }
}

void MenuWindow::SetLeftShift(int x)
{
  if (xshift == x)
    return;

  xshift = x;
  Redraw();
}

void MenuWindow::SetTopShift(int y)
{
  if (yshift == y)
    return;

  yshift = y;
  Redraw();
}

Button* MenuWindow::AppendSubMenu(const char *title,
    MenuWindow& submenu)
{
  Button *button = new Button (title);

  button->signal_activate.connect(sigc::hide(sigc::mem_fun(submenu,
        &MenuWindow::Show)));

  signal_hide.connect(sigc::hide(sigc::mem_fun(submenu,
        &MenuWindow::Hide)));

  submenu.SetRef (button);
  submenu.SetFlags (FLAG_HIDE_ON_CLOSE);

  listbox->AppendWidget (*button);

  return button;
}

void MenuWindow::AddWidget(Widget& widget, int x, int y)
{
  Window::AddWidget(widget, x, y);
}

void MenuWindow::OnScreenResizedInternal()
{
  UpdateSmartPositionAndSize();
  Window::OnScreenResizedInternal();
}

void MenuWindow::UpdateSmartPositionAndSize()
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
  int x, y;

  if (ref) {
    Point p = ref->GetAbsolutePosition();
    x = p.GetX() + xshift;
    y = p.GetY() + yshift;
  } else {
    x = win_x;
    y = win_y;
  }

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
void MenuWindow::OnChildrenHeightChange(ListBox& activator, int new_height)
{
  if (win_h != AUTOSIZE)
    return;

  UpdateSmartPositionAndSize();
}

//TODO actualy hide, not close(), also monitor show, etc
void MenuWindow::OnRefVisible(Widget& activator, bool visible)
{
  if (visible)
    return;

  // hide window if the reference widget is hidden
  Close();
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
