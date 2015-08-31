// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
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
/// Widget class implementation.
///
/// @ingroup cppconsui

#include "Widget.h"

#include "ColorScheme.h"
#include "CoreManager.h"

#include <cassert>
#include <cstring>

namespace CppConsUI {

Widget::Widget(int w, int h)
  : xpos_(UNSETPOS), ypos_(UNSETPOS), width_(w), height_(h),
    wish_width_(AUTOSIZE), wish_height_(AUTOSIZE), real_xpos_(UNSETPOS),
    real_ypos_(UNSETPOS), real_width_(0), real_height_(0), can_focus_(false),
    has_focus_(false), visible_(true), parent_(nullptr), color_scheme_(0),
    absolute_position_listeners_(0)
{
}

Widget::~Widget()
{
  setVisibility(false);

  if (parent_ != nullptr && absolute_position_listeners_.size() > 0)
    parent_->unregisterAbsolutePositionListener(*this);
}

void Widget::moveResize(int newx, int newy, int neww, int newh)
{
  if (newx == xpos_ && newy == ypos_ && neww == width_ && newh == height_)
    return;

  Rect oldsize(xpos_, ypos_, width_, height_);
  Rect newsize(newx, newy, neww, newh);

  xpos_ = newx;
  ypos_ = newy;
  width_ = neww;
  height_ = newh;

  signalMoveResize(oldsize, newsize);
}

Widget *Widget::getFocusWidget()
{
  if (can_focus_)
    return this;
  return nullptr;
}

void Widget::cleanFocus()
{
  if (!has_focus_)
    return;

  has_focus_ = false;
  signal_focus(*this, false);
  redraw();
}

bool Widget::restoreFocus()
{
  return grabFocus();
}

void Widget::ungrabFocus()
{
  if (parent_ == nullptr || !has_focus_)
    return;

  has_focus_ = false;
  signal_focus(*this, false);
  redraw();
}

bool Widget::grabFocus()
{
  if (parent_ == nullptr || has_focus_)
    return false;

  if (can_focus_ && isVisibleRecursive()) {
    if (parent_->setFocusChild(*this)) {
      has_focus_ = true;
      signal_focus(*this, true);
      redraw();
    }
    return true;
  }

  return false;
}

void Widget::setVisibility(bool new_visible)
{
  if (new_visible == visible_)
    return;

  visible_ = new_visible;

  if (parent_ != nullptr) {
    parent_->updateFocusChain();

    Container *t = getTopContainer();
    if (visible_) {
      if (!t->getFocusWidget()) {
        // There is no focused widget, try if this or a widget that was revealed
        // can grab it.
        t->moveFocus(Container::FOCUS_DOWN);
      }
    }
    else {
      Widget *focus = t->getFocusWidget();
      if (focus && !focus->isVisibleRecursive()) {
        // Focused widget was hidden, move the focus.
        t->moveFocus(Container::FOCUS_DOWN);
      }
    }

    signalVisible(visible_);
  }

  signal_visible(*this, visible_);
  redraw();
}

bool Widget::isVisibleRecursive() const
{
  if (parent_ == nullptr || !visible_)
    return false;

  return parent_->isWidgetVisible(*this);
}

void Widget::setParent(Container &new_parent)
{
  // Changing parent widget is not supported.
  assert(parent_ == nullptr);

  parent_ = &new_parent;

  // Register this widget as an absolute position listener to its parent in case
  // some other widget is interested in a position of this widget.
  if (absolute_position_listeners_.size() > 0)
    parent_->registerAbsolutePositionListener(*this);

  parent_->updateFocusChain();

  Container *t = getTopContainer();
  if (!t->getFocusWidget()) {
    // There is no focused widget, try if this or a child widget (in case of
    // Container) can grab it.
    Widget *w = getFocusWidget();
    if (w != nullptr)
      w->grabFocus();
  }
  else {
    // Clean focus if this widget/container was added to a container that
    // already has a focused widget.
    cleanFocus();
  }
}

// All following moveResize() wrappers should always call member methods to get
// xpos_, ypos_, width_, height_ and never use member variables directly. See
// getLeft(), getTop(), getWidth(), getHeight() in the Window class.
void Widget::move(int newx, int newy)
{
  moveResize(newx, newy, getWidth(), getHeight());
}

void Widget::resize(int neww, int newh)
{
  moveResize(getLeft(), getTop(), neww, newh);
}

void Widget::setLeft(int newx)
{
  moveResize(newx, getTop(), getWidth(), getHeight());
}

void Widget::setTop(int newy)
{
  moveResize(getLeft(), newy, getWidth(), getHeight());
}

void Widget::setWidth(int neww)
{
  moveResize(getLeft(), getTop(), neww, getHeight());
}

void Widget::setHeight(int newh)
{
  moveResize(getLeft(), getTop(), getWidth(), newh);
}

Point Widget::getAbsolutePosition() const
{
  if (parent_ == nullptr)
    return Point(0, 0);

  return parent_->getAbsolutePosition(*this);
}

Point Widget::getRelativePosition(const Container &ref) const
{
  if (parent_ == nullptr)
    return Point(0, 0);

  return parent_->getRelativePosition(ref, *this);
}

void Widget::setRealPosition(int newx, int newy)
{
  if (newx == real_xpos_ && newy == real_ypos_)
    return;

  real_xpos_ = newx;
  real_ypos_ = newy;

  signalAbsolutePositionChange();
}

void Widget::setRealSize(int neww, int newh)
{
  if (neww == real_width_ && newh == real_height_)
    return;

  real_width_ = neww;
  real_height_ = newh;

  updateArea();
}

void Widget::setColorScheme(int new_color_scheme)
{
  if (new_color_scheme == color_scheme_)
    return;

  color_scheme_ = new_color_scheme;
  redraw();
}

int Widget::getColorScheme() const
{
  if (color_scheme_ != 0)
    return color_scheme_;
  else if (parent_ != nullptr)
    return parent_->getColorScheme();

  return 0;
}

void Widget::registerAbsolutePositionListener(Widget &widget)
{
  absolute_position_listeners_.push_back(&widget);
  if (parent_ != nullptr && absolute_position_listeners_.size() == 1)
    parent_->registerAbsolutePositionListener(*this);
}

void Widget::unregisterAbsolutePositionListener(Widget &widget)
{
  Widgets::iterator i = std::find(absolute_position_listeners_.begin(),
    absolute_position_listeners_.end(), &widget);
  assert(i != absolute_position_listeners_.end());

  absolute_position_listeners_.erase(i);
  if (parent_ != nullptr && absolute_position_listeners_.size() == 0)
    parent_->unregisterAbsolutePositionListener(*this);
}

void Widget::onAbsolutePositionChange(Widget & /*widget*/)
{
  // Propagate the event to the listeners.
  signalAbsolutePositionChange();
}

void Widget::signalMoveResize(const Rect &oldsize, const Rect &newsize)
{
  if (parent_ == nullptr)
    return;

  // Tell the parent about the new size so it can position the widget correctly.
  parent_->onChildMoveResize(*this, oldsize, newsize);
}

void Widget::signalWishSizeChange(const Size &oldsize, const Size &newsize)
{
  if (parent_ == nullptr)
    return;

  // Tell the parent about the new wish size so it can position the widget
  // correctly.
  parent_->onChildWishSizeChange(*this, oldsize, newsize);
}

void Widget::signalVisible(bool visible)
{
  if (parent_ == nullptr)
    return;

  // Tell the parent about the new visibility status so it can reposition its
  // child widgets as appropriate.
  parent_->onChildVisible(*this, visible);
}

void Widget::signalAbsolutePositionChange()
{
  for (Widget *widget : absolute_position_listeners_)
    widget->onAbsolutePositionChange(*this);
}

void Widget::updateArea()
{
  // Empty for Widget.
}

void Widget::redraw()
{
  if (parent_ == nullptr)
    return;

  parent_->redraw();
}

void Widget::setWishSize(int neww, int newh)
{
  if (neww == wish_width_ && newh == wish_height_)
    return;

  Size oldsize(wish_width_, wish_height_);
  Size newsize(neww, newh);

  wish_width_ = neww;
  wish_height_ = newh;

  signalWishSizeChange(oldsize, newsize);
}

int Widget::getAttributes(int property, int *attrs, Error &error) const
{
  return getAttributes(property, 0, attrs, error);
}

int Widget::getAttributes(
  int property, int subproperty, int *attrs, Error &error) const
{
  return COLORSCHEME->getAttributes(
    getColorScheme(), property, subproperty, attrs, error);
}

Container *Widget::getTopContainer()
{
  if (parent_ != nullptr)
    return parent_->getTopContainer();
  return dynamic_cast<Container *>(this);
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
