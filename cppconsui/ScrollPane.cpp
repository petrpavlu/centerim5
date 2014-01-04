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
 * ScrollPane class implementation.
 *
 * @ingroup cppconsui
 */

#include "ScrollPane.h"

#include <algorithm>
#include <cassert>

namespace CppConsUI
{

ScrollPane::ScrollPane(int w, int h, int scrollw, int scrollh)
: Container(w, h), scroll_xpos(0), scroll_ypos(0), scroll_width(scrollw)
, scroll_height(scrollh), update_screen_area(false), screen_area(NULL)
{
  update_area = true;
}

ScrollPane::~ScrollPane()
{
  delete screen_area;
}

void ScrollPane::draw()
{
  drawEx(true);
}

int ScrollPane::getRealWidth() const
{
  if (!screen_area)
    return 0;
  return screen_area->getmaxx();
}

int ScrollPane::getRealHeight() const
{
  if (!screen_area)
    return 0;
  return screen_area->getmaxy();
}

Point ScrollPane::getRelativePosition(const Container& ref,
    const Widget& child) const
{
  assert(child.getParent() == this);

  if (!parent || this == &ref)
    return Point(child.getLeft() - scroll_xpos, child.getTop() - scroll_ypos);

  Point p = parent->getRelativePosition(ref, *this);
  return Point(p.getX() + child.getLeft() - scroll_xpos,
      p.getY() + child.getTop() - scroll_ypos);
}

Point ScrollPane::getAbsolutePosition(const Widget& child) const
{
  assert(child.getParent() == this);

  if (!parent)
    return Point(child.getLeft() - scroll_xpos, child.getTop() - scroll_ypos);

  Point p = parent->getAbsolutePosition(*this);
  return Point(p.getX() + child.getLeft() - scroll_xpos,
      p.getY() + child.getTop() - scroll_ypos);
}

void ScrollPane::setScrollSize(int swidth, int sheight)
{
  if (swidth == scroll_width && sheight == scroll_height)
    return;

  scroll_width = swidth;
  scroll_height = sheight;
  updateVirtualArea();
}

void ScrollPane::adjustScroll(int newx, int newy)
{
  bool scrolled = false;
  if (scroll_xpos != newx || scroll_ypos != newy)
    scrolled = true;

  if (screen_area) {
    scroll_xpos = newx;
    scroll_ypos = newy;

    int real_width = screen_area->getmaxx();
    int real_height = screen_area->getmaxy();

    if (scroll_xpos + real_width > scroll_width) {
      scroll_xpos = scroll_width - real_width;
      scrolled = true;
    }
    if (scroll_xpos < 0) {
      scroll_xpos = 0;
      scrolled = true;
    }

    if (scroll_ypos + real_height > scroll_height) {
      scroll_ypos = scroll_height - real_height;
      scrolled = true;
    }
    if (scroll_ypos < 0) {
      scroll_ypos = 0;
      scrolled = true;
    }
  }
  else {
    if (!scroll_xpos && !scroll_ypos)
      scrolled = true;
    scroll_xpos = 0;
    scroll_ypos = 0;
  }

  if (scrolled)
    redraw();
}

void ScrollPane::makeVisible(int x, int y)
{
  if (!screen_area) {
    adjustScroll(0, 0);
    return;
  }

  if (!makePointVisible(x, y))
    return;

  redraw();
}

void ScrollPane::makeVisible(int x, int y, int w, int h)
{
  if (!screen_area) {
    adjustScroll(0, 0);
    return;
  }

  bool scrolled = false;
  if (makePointVisible(x + w - 1, y + h - 1))
    scrolled = true;
  if (makePointVisible(x, y))
    scrolled = true;

  if (!scrolled)
    return;

  redraw();
}

void ScrollPane::updateArea()
{
  update_screen_area = true;
  redraw();
}

void ScrollPane::proceedUpdateArea()
{
  assert(parent);

  if (!update_screen_area)
    return;

  delete screen_area;
  screen_area = parent->getSubPad(*this, xpos, ypos, width, height);

  // fix scroll position if necessary
  adjustScroll(scroll_xpos, scroll_ypos);

  update_screen_area = false;
}

void ScrollPane::updateVirtualArea()
{
  if (!update_area)
    for (Children::iterator i = children.begin(); i != children.end(); i++)
      i->widget->updateArea();

  update_area = true;
}

void ScrollPane::proceedUpdateVirtualArea()
{
  if (!update_area)
    return;

  delete area;
  area = Curses::Window::newpad(scroll_width, scroll_height);
  update_area = false;
}

void ScrollPane::drawEx(bool container_draw)
{
  proceedUpdateArea();
  proceedUpdateVirtualArea();

  if (!area || !screen_area) {
    if (screen_area)
      screen_area->fill(getColorPair("container", "background"));
    return;
  }

  if (container_draw)
    Container::draw();

  /* If the defined scrollable area is smaller than the widget, make sure
   * the copy works. */
  int copyw = std::min(scroll_width, screen_area->getmaxx()) - 1;
  int copyh = std::min(scroll_height, screen_area->getmaxy()) - 1;

  area->copyto(screen_area, scroll_xpos, scroll_ypos, 0, 0, copyw, copyh, 0);
}

bool ScrollPane::makePointVisible(int x, int y)
{
  // fix parameters
  if (x < 0)
    x = 0;
  else if (x >= scroll_width)
    x = scroll_width - 1;
  if (y < 0)
    y = 0;
  else if (y >= scroll_height)
    y = scroll_height - 1;

  int real_width = screen_area->getmaxx();
  int real_height = screen_area->getmaxy();

  bool scrolled = false;
  if (x > scroll_xpos + real_width - 1) {
    scroll_xpos = x - real_width + 1;
    scrolled = true;
  }
  else if (x < scroll_xpos) {
    scroll_xpos = x;
    scrolled = true;
  }

  if (y > scroll_ypos + real_height - 1) {
    scroll_ypos = y - real_height + 1;
    scrolled = true;
  }
  else if (y < scroll_ypos) {
    scroll_ypos = y;
    scrolled = true;
  }

  return scrolled;
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
