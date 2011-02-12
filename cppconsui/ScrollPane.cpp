/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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
 * ScrollPane class implementation.
 *
 * @ingroup cppconsui
 */

#include "ScrollPane.h"

ScrollPane::ScrollPane(int w, int h, int scrollw, int scrollh)
: Container(w, h), scroll_xpos(0), scroll_ypos(0), scroll_width(scrollw)
, scroll_height(scrollh), update_screen_area(true), screen_area(NULL)
{
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

void ScrollPane::SetScrollSize(int swidth, int sheight)
{
  if (swidth == scroll_width && sheight == scroll_height)
    return;

  scroll_width = swidth;
  scroll_height = sheight;
  UpdateVirtualArea();
}

void ScrollPane::AdjustScroll(int newx, int newy)
{
  if (screen_area) {
    scroll_xpos = newx;
    scroll_ypos = newy;

    int real_width = screen_area->getmaxx();
    int real_height = screen_area->getmaxy();

    if (scroll_xpos + real_width > scroll_width)
      scroll_xpos = scroll_width - real_width;
    if (scroll_xpos < 0)
      scroll_xpos = 0;

    if (scroll_ypos + real_height > scroll_height)
      scroll_ypos = scroll_height - real_height;
    if (scroll_ypos < 0)
      scroll_ypos = 0;
  }
  else {
    scroll_xpos = 0;
    scroll_ypos = 0;
  }
  Redraw();
}

void ScrollPane::MakeVisible(int x, int y)
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

  if (!screen_area) {
    AdjustScroll(x, y);
    return;
  }

  int real_width = screen_area->getmaxx();
  int real_height = screen_area->getmaxy();

  bool redraw = false;
  if (x > scroll_xpos + real_width - 1) {
    scroll_xpos = x - real_width + 1;
    redraw = true;
  }
  else if (x < scroll_xpos) {
    scroll_xpos = x;
    redraw = true;
  }

  if (y > scroll_ypos + real_height - 1) {
    scroll_ypos = y - real_height + 1;
    redraw = true;
  }
  else if (y < scroll_ypos) {
    scroll_ypos = y;
    redraw = true;
  }

  if (redraw)
    Redraw();
}

void ScrollPane::UpdateArea()
{
  update_screen_area = true;
  Redraw();
}

void ScrollPane::RealUpdateArea()
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
  update_area = true;

  for (Children::iterator i = children.begin(); i != children.end(); i++)
    i->widget->UpdateArea();
}

void ScrollPane::RealUpdateVirtualArea()
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
  RealUpdateArea();
  RealUpdateVirtualArea();

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
