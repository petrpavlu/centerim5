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
#include "Keys.h"

#include "gettext.h"

FreeWindow::FreeWindow(int x, int y, int w, int h, Type t)
: Container(w, h), win_x(x), win_y(y), win_w(w), win_h(h), realwindow(NULL)
, type(t)
{
  UpdateArea();

  COREMANAGER->signal_resize.connect(sigc::mem_fun(this,
        &FreeWindow::ScreenResized));

  DeclareBindables();
}

FreeWindow::~FreeWindow()
{
  Hide();

  if (realwindow)
    delete realwindow;
}

void FreeWindow::DeclareBindables()
{
  DeclareBindable("window", "close-window",
      sigc::mem_fun(this, &FreeWindow::ActionClose),
      InputProcessor::BINDABLE_NORMAL);
}

void FreeWindow::MoveResize(int newx, int newy, int neww, int newh)
{
  if (newx == win_x && newy == win_y && neww == win_w && newh == win_h)
    return;

  win_x = newx;
  win_y = newy;
  win_w = neww;
  win_h = newh;

  Container::MoveResize(0, 0, win_w, win_h);
  UpdateArea();
}

void FreeWindow::Draw()
{
  RealUpdateArea();

  if (!area || !realwindow)
    return;

  area->erase();

  Container::Draw();

  // copy the virtual window to a window, then display it on screen
  area->copyto(realwindow, 0, 0, 0, 0, copy_w, copy_h, 0);

  // update virtual ncurses screen
  realwindow->touch();
  realwindow->noutrefresh();
}

void FreeWindow::SetVisibility(bool visible)
{
  visible ? Show() : Hide();
}

void FreeWindow::SetParent(Container& parent)
{
  // Window can not have a parent
  g_assert_not_reached();
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

void FreeWindow::ScreenResized()
{
}

void FreeWindow::RealUpdateArea()
{
  if (update_area) {
    // update virtual area
    if (area)
      delete area;
    area = Curses::Window::newpad(win_w, win_h);

    // update real area
    int maxx = Curses::getmaxx();
    int maxy = Curses::getmaxy();

    int left = win_x < 0 ? 0 : win_x;
    int top = win_y < 0 ? 0 : win_y;
    int right = win_x + win_w >= maxx ? maxx : win_x + win_w;
    int bottom = win_y + win_h >= maxy ? maxy : win_y + win_h;

    copy_w = right - left - 1;
    copy_h = bottom - top - 1;

    if (realwindow)
      delete realwindow;
    // this could fail if the window falls outside the visible area
    realwindow = Curses::Window::newwin(left, top, right - left,
        bottom - top);

    update_area = false;
  }
}

void FreeWindow::Redraw()
{
  COREMANAGER->Redraw();
}

void FreeWindow::ActionClose()
{
  Close();
}
