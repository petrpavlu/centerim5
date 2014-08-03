/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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
 * ListBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "ListBox.h"

#include <algorithm>
#include <cassert>

namespace CppConsUI
{

ListBox::ListBox(int w, int h)
: AbstractListBox(w, h), children_height(0), autosize_children_count(0)
{
  // allow fast focus changing (paging) using PageUp/PageDown keys
  page_focus = true;
}

HorizontalLine *ListBox::insertSeparator(size_t pos)
{
  HorizontalLine *l = new HorizontalLine(AUTOSIZE);
  insertWidget(pos, *l);
  return l;
}

HorizontalLine *ListBox::appendSeparator()
{
  HorizontalLine *l = new HorizontalLine(AUTOSIZE);
  appendWidget(*l);
  return l;
}

void ListBox::insertWidget(size_t pos, Widget& widget)
{
  Container::insertWidget(pos, widget, UNSETPOS, UNSETPOS);

  if (!widget.isVisible())
    return;

  // calculate the expected height by the widget
  int h = widget.getHeight();
  int autosize_change = 0;
  if (h == AUTOSIZE) {
    h = widget.getWishHeight();
    if (h == AUTOSIZE) {
      h = 1;
      autosize_change = 1;
    }
  }

  updateChildren(h, autosize_change);
}

void ListBox::appendWidget(Widget& widget)
{
  insertWidget(children.size(), widget);
}

void ListBox::updateArea()
{
  int autosize_height = 1;
  int autosize_height_extra = 0;
  if (autosize_children_count && children_height < real_height) {
    int space = real_height - (children_height - autosize_children_count);
    autosize_height = space / autosize_children_count;
    autosize_height_extra = space % autosize_children_count;
  }

  int y = 0;
  for (Children::iterator i = children.begin(); i != children.end(); i++) {
    Widget *widget = *i;
    bool is_visible = widget->isVisible();

    // position the widget correctly
    widget->setRealPosition(0, y);

    // calculate the real width
    int w = widget->getWidth();
    if (w == AUTOSIZE) {
      w = widget->getWishWidth();
      if (w == AUTOSIZE)
        w = real_width;
    }
    if (w > real_width)
      w = real_width;

    // calculate the real height
    int h = widget->getHeight();
    if (h == AUTOSIZE) {
      h = widget->getWishHeight();
      if (h == AUTOSIZE) {
        h = autosize_height;
        if (is_visible && autosize_height_extra) {
          autosize_height_extra--;
          h++;
        }
      }
    }

    widget->setRealSize(w, h);

    if (is_visible)
      y += h;
  }

  // make sure that the currently focused widget is visible
  updateScroll();
}

void ListBox::onChildMoveResize(Widget& activator, const Rect& oldsize,
    const Rect& newsize)
{
  // sanity check
  assert(newsize.getLeft() == UNSETPOS && newsize.getTop() == UNSETPOS);

  if (!activator.isVisible())
    return;

  int old_height = oldsize.getHeight();
  int new_height = newsize.getHeight();

  if (old_height == new_height)
    return;

  int autosize_change = 0;
  if (old_height == AUTOSIZE) {
    old_height = activator.getWishHeight();
    if (old_height == AUTOSIZE) {
      old_height = 1;
      autosize_change--;
    }
  }
  if (new_height == AUTOSIZE) {
    new_height = activator.getWishHeight();
    if (new_height == AUTOSIZE) {
      new_height = 1;
      autosize_change++;
    }
  }

  updateChildren(new_height - old_height, autosize_change);
}

void ListBox::onChildWishSizeChange(Widget& activator, const Size& oldsize,
    const Size& newsize)
{
  if (!activator.isVisible() || activator.getHeight() != AUTOSIZE)
    return;

  // the widget is visible and is autosized
  int old_height = oldsize.getHeight();
  int new_height = newsize.getHeight();

  if (old_height == new_height)
    return;

  updateChildren(new_height - old_height, 0);
}

void ListBox::onChildVisible(Widget& activator, bool visible)
{
  // the widget is being hidden or deleted
  int h = activator.getHeight();
  int sign = visible ? 1 : -1;
  int autosize_change = 0;
  if (h == AUTOSIZE) {
    h = activator.getWishHeight();
    if (h == AUTOSIZE) {
      h = 1;
      autosize_change = sign;
    }
  }
  updateChildren(sign * h, autosize_change);
}

void ListBox::updateChildren(int children_height_change,
    int autosize_children_count_change)
{
  // set new children data
  children_height += children_height_change;
  assert(children_height >= 0);
  autosize_children_count += autosize_children_count_change;
  assert(autosize_children_count >= 0);

  // reposition all child widgets
  updateArea();
  signal_children_height_change(*this, children_height);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
