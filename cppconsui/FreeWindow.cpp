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
 * FreeWindow class implementation.
 *
 * @ingroup cppconsui
 */

#include "FreeWindow.h"

#include "CoreManager.h"

namespace CppConsUI
{

FreeWindow::FreeWindow(int x, int y, int w, int h, Type t)
: Container(w, h), win_x(x), win_y(y), win_w(w), win_h(h), copy_x(0)
, copy_y(0), copy_w(0), copy_h(0), realwindow(NULL), type(t)
{
  UpdateArea();

  COREMANAGER->signal_resize.connect(sigc::mem_fun(this,
        &FreeWindow::OnScreenResizedInternal));
  COREMANAGER->signal_resize.connect(sigc::mem_fun(this,
        &FreeWindow::OnScreenResized));

  DeclareBindables();
}

FreeWindow::~FreeWindow()
{
  Hide();

  if (realwindow)
    delete realwindow;
}

void FreeWindow::MoveResize(int newx, int newy, int neww, int newh)
{
  if (newx == win_x && newy == win_y && neww == win_w && newh == win_h)
    return;

  win_x = newx;
  win_y = newy;
  win_w = neww;
  win_h = newh;

  ResizeAndUpdateArea();
}

void FreeWindow::Draw()
{
  ProceedUpdateArea();

  if (!area || !realwindow)
    return;

  area->erase();

  Container::Draw();

  /* Reverse the top right corner of the window if there isn't any focused
   * widget and the window is the top window. This way the user knows which
   * window is on the top and can be closed using the Esc key. */
  if (!input_child && COREMANAGER->GetTopWindow() == this)
    area->mvchgat(win_w - 1, 0, 1, Curses::Attr::REVERSE, 0, NULL);

  // copy the virtual window to a window, then display it on screen
  area->copyto(realwindow, copy_x, copy_y, 0, 0, copy_w, copy_h, 0);

  // update virtual ncurses screen
  realwindow->touch();
  realwindow->noutrefresh();
}

void FreeWindow::SetVisibility(bool visible)
{
  visible ? Show() : Hide();
}

Point FreeWindow::GetAbsolutePosition()
{
  return Point(win_x, win_y);
}

void FreeWindow::SetWishSize(int neww, int newh)
{
  if (neww == wish_width && newh == wish_height)
    return;

  Container::SetWishSize(neww, newh);
  /* If either width or height is set to AUTOSIZE then this window has to be
   * resized accordingly. */
  if (win_w == AUTOSIZE || win_h == AUTOSIZE)
    ResizeAndUpdateArea();
}

bool FreeWindow::IsWidgetVisible(const Widget& child) const
{
  return true;
}

bool FreeWindow::SetFocusChild(Widget& child)
{
  CleanFocus();

  focus_child = &child;
  SetInputChild(child);

  if (COREMANAGER->GetTopWindow() != this)
    return false;

  return true;
}

void FreeWindow::Show()
{
  COREMANAGER->AddWindow(*this);
  visible = true;
  signal_show(*this);
}

void FreeWindow::Hide()
{
  if (COREMANAGER->HasWindow(*this)) {
    COREMANAGER->RemoveWindow(*this);
    visible = false;
    signal_hide(*this);
  }
}

void FreeWindow::Close()
{
  signal_close(*this);
  delete this;
}

void FreeWindow::SetParent(Container& parent)
{
  // Window can not have a parent
  g_assert_not_reached();
}

void FreeWindow::ProceedUpdateArea()
{
  if (!update_area)
    return;

  int maxx = Curses::getmaxx();
  int maxy = Curses::getmaxy();

  // update virtual area
  if (area)
    delete area;
  int realw = win_w;
  if (realw == AUTOSIZE) {
    realw = GetWishWidth();
    if (realw == AUTOSIZE)
      realw = Curses::getmaxx() - win_x;
  }
  int realh = win_h;
  if (realh == AUTOSIZE) {
    realh = GetWishHeight();
    if (realh == AUTOSIZE)
      realh = Curses::getmaxy() - win_y;
  }
  area = Curses::Window::newpad(realw, realh);

  // update real area
  int left = MAX(0, win_x);
  int top = MAX(0, win_y);
  int right = MIN(win_x + realw, maxx);
  int bottom = MIN(win_y + realh, maxy);

  copy_x = left - win_x;
  copy_y = top - win_y;
  copy_w = right - left - 1;
  copy_h = bottom - top - 1;

  if (realwindow)
    delete realwindow;
  // this could fail if the window falls outside the visible area
  realwindow = Curses::Window::newwin(left, top, right - left, bottom - top);

  update_area = false;
}

void FreeWindow::Redraw()
{
  COREMANAGER->Redraw();
}

void FreeWindow::OnScreenResizedInternal()
{
  ResizeAndUpdateArea();
}

void FreeWindow::ResizeAndUpdateArea()
{
  int realw = win_w;
  if (realw == AUTOSIZE) {
    realw = GetWishWidth();
    if (realw == AUTOSIZE)
      realw = Curses::getmaxx() - win_x;
  }
  int realh = win_h;
  if (realh == AUTOSIZE) {
    realh = GetWishHeight();
    if (realh == AUTOSIZE)
      realh = Curses::getmaxy() - win_y;
  }
  Container::MoveResize(0, 0, MAX(0, realw), MAX(0, realh));
  UpdateArea();
}

void FreeWindow::ActionClose()
{
  Close();
}

void FreeWindow::DeclareBindables()
{
  DeclareBindable("window", "close-window",
      sigc::mem_fun(this, &FreeWindow::ActionClose),
      InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
