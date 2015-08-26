// Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
//
// This file is part of CenterIM.
//
// CenterIM is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// CenterIM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// Window class implementation.
///
/// @ingroup cppconsui

#include "Window.h"

#include "CoreManager.h"

#include <algorithm>

namespace CppConsUI {

Window::Window(int x, int y, int w, int h, Type t, bool decorated)
  : Container(w, h), type_(t), decorated_(decorated), closable_(true)
{
  initWindow(x, y, NULL);
}

Window::Window(
  int x, int y, int w, int h, const char *title, Type t, bool decorated)
  : Container(w, h), type_(t), decorated_(decorated), closable_(true)
{
  initWindow(x, y, title);
}

Window::~Window()
{
  hide();
  COREMANAGER->removeWindow(*this);
  delete panel_;
}

int Window::draw(Curses::ViewPort area, Error &error)
{
  DRAW(area.erase(error));

  DRAW(Container::draw(area, error));
  if (decorated_)
    DRAW(panel_->draw(area, error));

  // Reverse the top right corner of the window if there is not any focused
  // widget and the window is the top window. This way the user knows which
  // window is on the top and can be closed using the Esc key.
  if (input_child_ == NULL && COREMANAGER->getTopWindow() == this)
    DRAW(area.changeAt(real_width_ - 1, 0, 1, Curses::Attr::REVERSE, 0, error));

  return 0;
}

void Window::setVisibility(bool visible)
{
  visible ? show() : hide();
}

Point Window::getAbsolutePosition()
{
  return Point(real_xpos_, real_ypos_);
}

bool Window::isWidgetVisible(const Widget & /*child*/) const
{
  return true;
}

bool Window::setFocusChild(Widget &child)
{
  cleanFocus();

  focus_child_ = &child;
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
    child_x -= scroll_xpos_;
    child_y -= scroll_ypos_;

    if (real_xpos_ != UNSETPOS && real_ypos_ != UNSETPOS)
      return Point(real_xpos_ + child_x, real_ypos_ + child_y);
  }

  return Point(UNSETPOS, UNSETPOS);
}

void Window::show()
{
  visible_ = true;
  COREMANAGER->topWindow(*this);
  signal_show(*this);
}

void Window::hide()
{
  visible_ = false;
  signal_hide(*this);
}

void Window::close()
{
  signal_close(*this);
  delete this;
}

void Window::setClosable(bool new_closable)
{
  closable_ = new_closable;
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
  panel_->setRealSize(real_width_, real_height_);
  Container::updateArea();
}

void Window::redraw()
{
  COREMANAGER->redraw();
}

void Window::initWindow(int x, int y, const char *title)
{
  xpos_ = x;
  ypos_ = y;
  visible_ = false;
  if (decorated_)
    border_ = 1;

  panel_ = new Panel(AUTOSIZE, AUTOSIZE, title);
  panel_->setParent(*this);
  panel_->setRealPosition(0, 0);

  COREMANAGER->registerWindow(*this);

  declareBindables();
}

void Window::actionClose()
{
  if (closable_)
    close();
}

void Window::declareBindables()
{
  declareBindable("window", "close-window",
    sigc::mem_fun(this, &Window::actionClose), InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
