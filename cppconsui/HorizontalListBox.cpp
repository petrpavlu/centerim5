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
 * HorizontalListBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "HorizontalListBox.h"

#include <algorithm>

namespace CppConsUI
{

HorizontalListBox::HorizontalListBox(int w, int h)
: AbstractListBox(w, h), children_width(0), autosize_children_count(0)
{
}

void HorizontalListBox::updateArea()
{
  AbstractListBox::updateArea();

  if (!screen_area)
    return;

  // adjust accordingly a size of the virtual scroll area
  setScrollHeight(screen_area->getmaxy());
  int origw = area ? area->getmaxx() : 0;
  updateScrollWidth();
  if (area && origw != area->getmaxx())
    repositionChildren();
}

VerticalLine *HorizontalListBox::insertSeparator(size_t pos)
{
  VerticalLine *l = new VerticalLine(AUTOSIZE);
  insertWidget(pos, *l);
  return l;
}

VerticalLine *HorizontalListBox::appendSeparator()
{
  VerticalLine *l = new VerticalLine(AUTOSIZE);
  appendWidget(*l);
  return l;
}

void HorizontalListBox::insertWidget(size_t pos, Widget& widget)
{
  ScrollPane::insertWidget(pos, widget, UNSETPOS, UNSETPOS);

  if (!widget.isVisible())
    return;

  // calculate the expected width by the widget
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

void HorizontalListBox::appendWidget(Widget& widget)
{
  insertWidget(children.size(), widget);
}

void HorizontalListBox::onChildMoveResize(Widget& activator,
    const Rect& oldsize, const Rect& newsize)
{
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
      autosize_change--;
    }
  }
  if (new_width == AUTOSIZE) {
    new_width = activator.getWishWidth();
    if (new_width == AUTOSIZE) {
      new_width = 1;
      autosize_change++;
    }
  }

  updateChildren(new_width - old_width, autosize_change);
}

void HorizontalListBox::onChildWishSizeChange(Widget& activator,
    const Size& oldsize, const Size& newsize)
{
  if (!activator.isVisible() || activator.getWidth() != AUTOSIZE)
    return;

  // the widget is visible and is autosized
  int old_width = oldsize.getWidth();
  int new_width = newsize.getWidth();

  if (old_width == new_width)
    return;

  updateChildren(new_width - old_width, 0);
}

void HorizontalListBox::onChildVisible(Widget& activator, bool visible)
{
  // the widget is being hidden or deleted
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

void HorizontalListBox::updateChildren(int children_width_change,
    int autosize_children_count_change)
{
  // set new children data
  children_width += children_width_change;
  assert(children_width >= 0);
  autosize_children_count += autosize_children_count_change;
  assert(autosize_children_count >= 0);

  // update scroll width and reposition all child widgets
  updateScrollWidth();
  repositionChildren();
  signal_children_width_change(*this, children_width);
}

void HorizontalListBox::updateScrollWidth()
{
  int realw = 0;
  if (screen_area)
    realw = screen_area->getmaxx();

  setScrollWidth(std::max(realw, children_width));
}

void HorizontalListBox::repositionChildren()
{
  if (!area)
    return;

  int autosize_width = 1;
  int autosize_width_extra = 0;
  int realw = area->getmaxx();
  if (autosize_children_count && children_width < realw) {
    int space = realw - (children_width - autosize_children_count);
    autosize_width = space / autosize_children_count;
    autosize_width_extra = space % autosize_children_count;
  }

  int x = 0;
  for (Children::iterator i = children.begin(); i != children.end(); i++) {
    Widget *widget = *i;
    if (!widget->isVisible())
      continue;

    // position the widget correctly
    widget->startPositioning();
    widget->move(x, 0);
    int w = widget->getWidth();
    if (w == AUTOSIZE) {
      w = autosize_width;
      if (autosize_width_extra) {
        autosize_width_extra--;
        w++;
      }
      widget->setAutoWidth(w);
    }
    widget->finishPositioning();

    x += w;
  }

  // make sure that the currently focused widget is visible
  if (focus_child) {
    int w = focus_child->getWidth();
    if (w == AUTOSIZE)
      w = focus_child->getAutoWidth();

    makeVisible(focus_child->getLeft(), focus_child->getTop(), w, 1);
  }
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
