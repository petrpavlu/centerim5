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
  if (screen_area)
    delete screen_area;
}

void ScrollPane::Draw()
{
  DrawEx(true);
}

int ScrollPane::GetRealWidth() const
{
  if (!screen_area)
    return 0;
  return screen_area->getmaxx();
}

int ScrollPane::GetRealHeight() const
{
  if (!screen_area)
    return 0;
  return screen_area->getmaxy();
}

Point ScrollPane::GetRelativePosition(const Container& ref,
    const Widget& child) const
{
  g_assert(child.GetParent() == this);

  if (!parent || this == &ref)
    return Point(child.GetLeft() - scroll_xpos, child.GetTop() - scroll_ypos);

  Point p = parent->GetRelativePosition(ref, *this);
  return Point(p.GetX() + child.GetLeft() - scroll_xpos,
      p.GetY() + child.GetTop() - scroll_ypos);
}

Point ScrollPane::GetAbsolutePosition(const Widget& child) const
{
  g_assert(child.GetParent() == this);

  if (!parent)
    return Point(child.GetLeft() - scroll_xpos, child.GetTop() - scroll_ypos);

  Point p = parent->GetAbsolutePosition(*this);
  return Point(p.GetX() + child.GetLeft() - scroll_xpos,
      p.GetY() + child.GetTop() - scroll_ypos);
}

void ScrollPane::SetScrollSize(int swidth, int sheight)
{
  if (swidth == scroll_width && sheight == scroll_height)
    return;

  scroll_width = swidth;
  scroll_height = sheight;
  UpdateVirtualArea();

  signal_scrollarea_resize(*this, Size(scroll_width, scroll_height));
}

void ScrollPane::AdjustScroll(int newx, int newy)
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

  if (scrolled) {
    Redraw();
    signal_scrollarea_scroll(*this, Point(scroll_xpos, scroll_ypos));
  }
}

void ScrollPane::MakeVisible(int x, int y)
{
  if (!screen_area) {
    AdjustScroll(0, 0);
    return;
  }

  if (!MakePointVisible(x, y))
    return;

  Redraw();
  signal_scrollarea_scroll(*this, Point(scroll_xpos, scroll_ypos));
}

void ScrollPane::MakeVisible(int x, int y, int w, int h)
{
  if (!screen_area) {
    AdjustScroll(0, 0);
    return;
  }

  bool scrolled = false;
  if (MakePointVisible(x + w - 1, y + h - 1))
    scrolled = true;
  if (MakePointVisible(x, y))
    scrolled = true;

  if (!scrolled)
    return;

  Redraw();
  signal_scrollarea_scroll(*this, Point(scroll_xpos, scroll_ypos));
}

void ScrollPane::UpdateArea()
{
  update_screen_area = true;
  Redraw();
}

void ScrollPane::ProceedUpdateArea()
{
  g_assert(parent);

  if (update_screen_area) {
    if (screen_area)
      delete screen_area;
    screen_area = parent->GetSubPad(*this, xpos, ypos, width, height);

    // fix scroll position if necessary
    AdjustScroll(scroll_xpos, scroll_ypos);

    update_screen_area = false;
  }
}

void ScrollPane::UpdateVirtualArea()
{
  if (!update_area)
    for (Children::iterator i = children.begin(); i != children.end(); i++)
      i->widget->UpdateArea();

  update_area = true;
}

void ScrollPane::ProceedUpdateVirtualArea()
{
  if (update_area) {
    if (area)
      delete area;
    area = Curses::Window::newpad(scroll_width, scroll_height);

    update_area = false;
  }
}

void ScrollPane::DrawEx(bool container_draw)
{
  ProceedUpdateArea();
  ProceedUpdateVirtualArea();

  if (!area || !screen_area) {
    if (screen_area)
      screen_area->fill(GetColorPair("container", "background"));
    return;
  }

  if (container_draw)
    Container::Draw();

  /* If the defined scrollable area is smaller than the widget, make sure
   * the copy works. */
  int copyw = MIN(scroll_width, screen_area->getmaxx()) - 1;
  int copyh = MIN(scroll_height, screen_area->getmaxy()) - 1;

  area->copyto(screen_area, scroll_xpos, scroll_ypos, 0, 0, copyw, copyh, 0);
}

bool ScrollPane::MakePointVisible(int x, int y)
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
