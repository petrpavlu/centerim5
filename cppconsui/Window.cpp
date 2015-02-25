/*
 * Copyright (C) 2010-2015 by CenterIM developers
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

#include "CoreManager.h"

#include <algorithm>

namespace CppConsUI
{

Window::Window(int x, int y, int w, int h, Type t, bool decorated_)
: Container(w, h), type(t), decorated(decorated_), closable(true)
{
  initWindow(x, y, NULL);
}

Window::Window(int x, int y, int w, int h, const char *title, Type t,
    bool decorated_)
: Container(w, h), type(t), decorated(decorated_), closable(true)
{
  initWindow(x, y, title);
}

Window::~Window()
{
  hide();
  COREMANAGER->removeWindow(*this);
  delete panel;
}

void Window::draw(Curses::ViewPort area)
{
  area.erase();

  Container::draw(area);
  if (decorated)
    panel->draw(area);

  /* Reverse the top right corner of the window if there isn't any focused
   * widget and the window is the top window. This way the user knows which
   * window is on the top and can be closed using the Esc key. */
  if (!input_child && COREMANAGER->getTopWindow() == this)
    area.changeAt(real_width - 1, 0, 1, Curses::Attr::REVERSE, 0, NULL);
}

void Window::setVisibility(bool visible)
{
  visible ? show() : hide();
}

Point Window::getAbsolutePosition()
{
  return Point(real_xpos, real_ypos);
}

bool Window::isWidgetVisible(const Widget & /*child*/) const
{
  return true;
}

bool Window::setFocusChild(Widget &child)
{
  cleanFocus();

  focus_child = &child;
  setInputChild(child);

  if (COREMANAGER->getTopWindow() != this)
    return false;

  return true;
}

Point Window::getAbsolutePosition(const Widget &child) const
{
  assert(child.getParent() == this);

  int child_x = child.getRealLeft();
  int child_y = child.getRealTop();

  if (child_x != UNSETPOS && child_y != UNSETPOS) {
    child_x -= scroll_xpos;
    child_y -= scroll_ypos;

    if (real_xpos != UNSETPOS && real_ypos != UNSETPOS)
      return Point(real_xpos + child_x, real_ypos + child_y);
  }

  return Point(UNSETPOS, UNSETPOS);
}

void Window::show()
{
  visible = true;
  COREMANAGER->topWindow(*this);
  signal_show(*this);
}

void Window::hide()
{
  visible = false;
  signal_hide(*this);
}

void Window::close()
{
  signal_close(*this);
  delete this;
}

void Window::setClosable(bool new_closable)
{
  closable = new_closable;
}

void Window::signalMoveResize(const Rect &oldsize, const Rect &newsize)
{
  COREMANAGER->onWindowMoveResize(*this, oldsize, newsize);
  Container::signalMoveResize(oldsize, newsize);
}

void Window::signalWishSizeChange(const Size &oldsize, const Size &newsize)
{
  COREMANAGER->onWindowWishSizeChange(*this, oldsize, newsize);
  Container::signalWishSizeChange(oldsize, newsize);
}

void Window::updateArea()
{
  panel->setRealSize(real_width, real_height);
  Container::updateArea();
}

void Window::redraw()
{
  COREMANAGER->redraw();
}

void Window::initWindow(int x, int y, const char *title)
{
  xpos = x;
  ypos = y;
  visible = false;
  if (decorated)
    border = 1;

  panel = new Panel(AUTOSIZE, AUTOSIZE, title);
  panel->setParent(*this);
  panel->setRealPosition(0, 0);

  COREMANAGER->registerWindow(*this);

  declareBindables();
}

void Window::actionClose()
{
  if (closable)
    close();
}

void Window::declareBindables()
{
  declareBindable("window", "close-window",
      sigc::mem_fun(this, &Window::actionClose),
      InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
