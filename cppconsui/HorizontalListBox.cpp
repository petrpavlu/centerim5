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
/// HorizontalListBox class implementation.
///
/// @ingroup cppconsui

#include "HorizontalListBox.h"

#include <algorithm>
#include <cassert>

namespace CppConsUI {

HorizontalListBox::HorizontalListBox(int w, int h)
  : AbstractListBox(w, h), children_width_(0), autosize_children_count_(0)
{
}

VerticalLine *HorizontalListBox::insertSeparator(std::size_t pos)
{
  auto l = new VerticalLine(AUTOSIZE);
  insertWidget(pos, *l);
  return l;
}

VerticalLine *HorizontalListBox::appendSeparator()
{
  auto l = new VerticalLine(AUTOSIZE);
  appendWidget(*l);
  return l;
}

void HorizontalListBox::insertWidget(std::size_t pos, Widget &widget)
{
  Container::insertWidget(pos, widget, UNSETPOS, UNSETPOS);

  if (!widget.isVisible())
    return;

  // Calculate the expected width by the widget.
  int w = widget.getWidth();
  int autosize_change = 0;
  if (w == AUTOSIZE) {
    w = widget.getWishWidth();
    if (w == AUTOSIZE) {
      w = 1;
      autosize_change = 1;
    }
  }

  updateChildren(w, autosize_change);
}

void HorizontalListBox::appendWidget(Widget &widget)
{
  insertWidget(children_.size(), widget);
}

void HorizontalListBox::updateArea()
{
  int autosize_width = 1;
  int autosize_width_extra = 0;
  if (autosize_children_count_ && children_width_ < real_width_) {
    int space = real_width_ - (children_width_ - autosize_children_count_);
    autosize_width = space / autosize_children_count_;
    autosize_width_extra = space % autosize_children_count_;
  }

  int x = 0;
  for (Widget *widget : children_) {
    bool is_visible = widget->isVisible();

    // Position the widget correctly.
    widget->setRealPosition(x, 0);

    // Calculate the real width.
    int w = widget->getWidth();
    if (w == AUTOSIZE) {
      w = widget->getWishWidth();
      if (w == AUTOSIZE) {
        w = autosize_width;
        if (is_visible && autosize_width_extra > 0) {
          --autosize_width_extra;
          ++w;
        }
      }
    }

    // Calculate the real height.
    int h = widget->getHeight();
    if (h == AUTOSIZE) {
      h = widget->getWishHeight();
      if (h == AUTOSIZE)
        h = real_height_;
    }
    if (h > real_height_)
      h = real_height_;

    widget->setRealSize(w, h);

    if (is_visible)
      x += w;
  }

  // Make sure that the currently focused widget is visible.
  updateScroll();
}

void HorizontalListBox::onChildMoveResize(
  Widget &activator, const Rect &oldsize, const Rect &newsize)
{
  // Sanity check.
  assert(newsize.getLeft() == UNSETPOS && newsize.getTop() == UNSETPOS);

  if (!activator.isVisible())
    return;

  int old_width = oldsize.getWidth();
  int new_width = newsize.getWidth();

  if (old_width == new_width)
    return;

  int autosize_change = 0;
  if (old_width == AUTOSIZE) {
    old_width = activator.getWishWidth();
    if (old_width == AUTOSIZE) {
      old_width = 1;
      --autosize_change;
    }
  }
  if (new_width == AUTOSIZE) {
    new_width = activator.getWishWidth();
    if (new_width == AUTOSIZE) {
      new_width = 1;
      ++autosize_change;
    }
  }

  updateChildren(new_width - old_width, autosize_change);
}

void HorizontalListBox::onChildWishSizeChange(
  Widget &activator, const Size &oldsize, const Size &newsize)
{
  if (!activator.isVisible() || activator.getWidth() != AUTOSIZE)
    return;

  // The widget is visible and is autosized.
  int old_width = oldsize.getWidth();
  int new_width = newsize.getWidth();

  if (old_width == new_width)
    return;

  updateChildren(new_width - old_width, 0);
}

void HorizontalListBox::onChildVisible(Widget &activator, bool visible)
{
  // The widget is being hidden or deleted.
  int w = activator.getWidth();
  int sign = visible ? 1 : -1;
  int autosize_change = 0;
  if (w == AUTOSIZE) {
    w = activator.getWishWidth();
    if (w == AUTOSIZE) {
      w = 1;
      autosize_change = sign;
    }
  }
  updateChildren(sign * w, autosize_change);
}

void HorizontalListBox::updateChildren(
  int children_width_change, int autosize_children_count_change)
{
  // Set new children data.
  children_width_ += children_width_change;
  assert(children_width_ >= 0);
  autosize_children_count_ += autosize_children_count_change;
  assert(autosize_children_count_ >= 0);

  // Reposition all child widgets.
  updateArea();
  signal_children_width_change(*this, children_width_);
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
