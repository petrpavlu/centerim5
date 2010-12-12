/*
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
 * FreeWindow class implementation.
 *
 * @ingroup cppconsui
 */

#include "FreeWindow.h"

#include "CoreManager.h"
#include "Keys.h"

#include "gettext.h"

#define CONTEXT_WINDOW "window"

FreeWindow::FreeWindow(int x, int y, int w, int h, Type t)
: Container(w, h)
, win_x(x)
, win_y(y)
, win_w(w)
, win_h(h)
, realwindow(NULL)
, type(t)
{
  MakeRealWindow();
  UpdateArea();

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
  DeclareBindable(CONTEXT_WINDOW, "close-window",
      sigc::mem_fun(this, &FreeWindow::ActionClose),
      InputProcessor::BINDABLE_NORMAL);
}

DEFINE_SIG_REGISTERKEYS(FreeWindow, RegisterKeys);
bool FreeWindow::RegisterKeys()
{
  RegisterKeyDef(CONTEXT_WINDOW, "close-window", _("Close the window."),
      Keys::SymbolTermKey(TERMKEY_SYM_ESCAPE));
  return true;
}

void FreeWindow::MoveResize(int newx, int newy, int neww, int newh)
{
  win_x = newx;
  win_y = newy;
  win_w = neww;
  win_h = newh;

  MakeRealWindow();
  UpdateArea();

  Container::MoveResize(0, 0, win_w, win_h);
}

void FreeWindow::UpdateArea()
{
  if (area)
    delete area;
  area = Curses::Window::newpad(win_w, win_h);
  signal_redraw(*this);
}

void FreeWindow::Draw()
{
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

bool FreeWindow::IsWidgetVisible(const Widget& child) const
{
  return true;
}

bool FreeWindow::SetFocusChild(Widget& child)
{
  if (focus_child)
    focus_child->CleanFocus();

  focus_child = &child;
  SetInputChild(child);
  if (!signal_focus_child_last) {
    signal_focus_child(*this, true);
    signal_focus_child_last = true;
  }

  if (COREMANAGER->GetTopWindow() != this)
    return false;

  return true;
}

void FreeWindow::Show()
{
  COREMANAGER->AddWindow(*this);
  signal_show(*this);
}

void FreeWindow::Hide()
{
  if (COREMANAGER->HasWindow(*this)) {
    COREMANAGER->RemoveWindow(*this);
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

void FreeWindow::MakeRealWindow()
{
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
  realwindow = Curses::Window::newwin(left, top, right - left, bottom - top);
}

void FreeWindow::ActionClose()
{
  Close();
}
