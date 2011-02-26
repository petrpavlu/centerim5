/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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
 * ScrollPane class.
 *
 * @ingroup cppconsui
 */

#ifndef __SCROLLPANE_H__
#define __SCROLLPANE_H__

#include "Container.h"

/**
 * TODO
 */
class ScrollPane
: public Container
{
public:
  ScrollPane(int w, int h, int scrollw, int scrollh);
  virtual ~ScrollPane();

  // Widget
  virtual void Draw();

  // Container
  virtual Point GetAbsolutePosition(const Container& ref,
      const Widget& child) const;
  virtual Point GetAbsolutePosition(const Widget& child) const;

  /**
   * Returns a size of the scrollable area.
   */
  Rect GetScrollSize() { return Rect(0, 0, scroll_width, scroll_height); }
  /**
   * Returns a width of the scrollable area.
   */
  int GetScrollWidth() { return scroll_width; }
  /**
   * Returns a height of the scrollable area.
   */
  int GetScrollHeight() { return scroll_height; }

  /**
   * Sets a size of the scrollable area.
   */
  void SetScrollSize(int swidth, int sheight);
  /**
   * Sets a width of the scrollable area.
   */
  void SetScrollWidth(int swidth)
    { SetScrollSize(swidth, scroll_height); }
  /**
   * Sets a height of the scrollable area.
   */
  void SetScrollHeight(int sheight)
    { SetScrollSize(scroll_width, sheight); }

  /**
   * Adjusts a visible area to a given position.
   */
  void AdjustScroll(int newx, int newy);
  /**
   * Returns a visible scroll area coordinates.
   */
  Point GetScrollPosition() { return Point(scroll_xpos, scroll_ypos); }
  /**
   * Returns a visible scroll area x position.
   */
  int GetScrollPositionX() { return scroll_xpos; }
  /**
   * Returns a visible scroll area y position.
   */
  int GetScrollPositionY() { return scroll_ypos; }

  /**
   * Adjusts a scroll area to make a given position visible.
   */
  void MakeVisible(int x, int y);

protected:
  int scroll_xpos, scroll_ypos, scroll_width, scroll_height;
  bool update_screen_area;

  Curses::Window *screen_area;

  // Widget
  virtual void UpdateArea();
  virtual void RealUpdateArea();

  virtual void UpdateVirtualArea();
  virtual void RealUpdateVirtualArea();

  virtual void DrawEx(bool container_draw);

private:
  ScrollPane(const ScrollPane&);
  ScrollPane& operator=(const ScrollPane&);
};

#endif // __SCROLLPANE_H__
