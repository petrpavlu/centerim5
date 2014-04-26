/*
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
 * FreeWindow class implementation.
 *
 * @ingroup cppconsui
 */

#include "FreeWindow.h"

#include "CoreManager.h"

#include <algorithm>

namespace CppConsUI
{

FreeWindow::FreeWindow(int x, int y, int w, int h, Type t)
: Container(w, h), win_x(x), win_y(y), win_w(w), win_h(h), copy_x(0)
, copy_y(0), copy_w(0), copy_h(0), realwindow(NULL), type(t), closable(true)
{
  COREMANAGER->signal_resize.connect(sigc::mem_fun(this,
        &FreeWindow::onScreenResizedInternal));
  COREMANAGER->signal_resize.connect(sigc::mem_fun(this,
        &FreeWindow::onScreenResized));

  declareBindables();
}

FreeWindow::~FreeWindow()
{
  hide();

  delete realwindow;
}

void FreeWindow::moveResize(int newx, int newy, int neww, int newh)
{
  if (newx == win_x && newy == win_y && neww == win_w && newh == win_h)
    return;

  win_x = newx;
  win_y = newy;
  win_w = neww;
  win_h = newh;

  updateArea();
}

void FreeWindow::updateArea()
{
  int maxx = Curses::getmaxx();
  int maxy = Curses::getmaxy();

  // update virtual area
  delete area;
  int realw = win_w;
  if (realw == AUTOSIZE) {
    realw = getWishWidth();
    if (realw == AUTOSIZE)
      realw = maxx - win_x;
  }
  int realh = win_h;
  if (realh == AUTOSIZE) {
    realh = getWishHeight();
    if (realh == AUTOSIZE)
      realh = maxy - win_y;
  }
  area = Curses::Window::newpad(realw, realh);

  // inform the container about the new area
  updateContainer(realw, realh);

  // update real area
  int left = std::max(0, win_x);
  int top = std::max(0, win_y);
  int right = std::min(win_x + realw, maxx);
  int bottom = std::min(win_y + realh, maxy);

  copy_x = left - win_x;
  copy_y = top - win_y;
  copy_w = right - left - 1;
  copy_h = bottom - top - 1;

  delete realwindow;
  // this could fail if the window falls outside the visible area
  realwindow = Curses::Window::newwin(left, top, right - left, bottom - top);
}

void FreeWindow::draw()
{
  if (!area || !realwindow)
    return;

  area->erase();

  Container::draw();

  /* Reverse the top right corner of the window if there isn't any focused
   * widget and the window is the top window. This way the user knows which
   * window is on the top and can be closed using the Esc key. */
  if (!input_child && COREMANAGER->getTopWindow() == this)
    area->mvchgat(win_w - 1, 0, 1, Curses::Attr::REVERSE, 0, NULL);

  // copy the virtual window to a window, then display it on screen
  area->copyto(realwindow, copy_x, copy_y, 0, 0, copy_w, copy_h, 0);

  // update virtual ncurses screen
  realwindow->touch();
  realwindow->noutrefresh();
}

void FreeWindow::setVisibility(bool visible)
{
  visible ? show() : hide();
}

Point FreeWindow::getAbsolutePosition()
{
  return Point(win_x, win_y);
}

void FreeWindow::setWishSize(int neww, int newh)
{
  if (neww == wish_width && newh == wish_height)
    return;

  Container::setWishSize(neww, newh);
  updateArea();
}

bool FreeWindow::isWidgetVisible(const Widget& /*child*/) const
{
  return true;
}

bool FreeWindow::setFocusChild(Widget& child)
{
  cleanFocus();

  focus_child = &child;
  setInputChild(child);

  if (COREMANAGER->getTopWindow() != this)
    return false;

  return true;
}

void FreeWindow::show()
{
  COREMANAGER->addWindow(*this);
  visible = true;
  signal_show(*this);
}

void FreeWindow::hide()
{
  if (!COREMANAGER->hasWindow(*this))
    return;

  COREMANAGER->removeWindow(*this);
  visible = false;
  signal_hide(*this);
}

void FreeWindow::close()
{
  signal_close(*this);
  delete this;
}

void FreeWindow::setClosable(bool new_closable)
{
  closable = new_closable;
}

void FreeWindow::redraw()
{
  COREMANAGER->redraw();
}

void FreeWindow::updateContainer(int realw, int realh)
{
  Container::moveResize(0, 0, std::max(0, realw), std::max(0, realh));
  Container::updateArea();
}

void FreeWindow::onScreenResizedInternal()
{
  updateArea();
}

void FreeWindow::actionClose()
{
  if (closable)
    close();
}

void FreeWindow::declareBindables()
{
  declareBindable("window", "close-window",
      sigc::mem_fun(this, &FreeWindow::actionClose),
      InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
