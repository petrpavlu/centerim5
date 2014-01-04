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
 * ScrollPane class.
 *
 * @ingroup cppconsui
 */

#ifndef __SCROLLPANE_H__
#define __SCROLLPANE_H__

#include "Container.h"

namespace CppConsUI
{

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
  virtual void draw();
  virtual int getRealWidth() const;
  virtual int getRealHeight() const;

  // Container
  virtual Point getRelativePosition(const Container& ref,
      const Widget& child) const;
  virtual Point getAbsolutePosition(const Widget& child) const;

  /**
   * Sets a size of the scrollable area.
   */
  virtual void setScrollSize(int swidth, int sheight);
  /**
   * Sets a width of the scrollable area.
   */
  virtual void setScrollWidth(int swidth)
    { setScrollSize(swidth, scroll_height); }
  /**
   * Sets a height of the scrollable area.
   */
  virtual void setScrollHeight(int sheight)
    { setScrollSize(scroll_width, sheight); }

  /**
   * Returns a size of the scrollable area.
   */
  virtual Size getScrollSize() const
    { return Size(scroll_width, scroll_height); }
  /**
   * Returns a width of the scrollable area.
   */
  virtual int getScrollWidth() const { return scroll_width; }
  /**
   * Returns a height of the scrollable area.
   */
  virtual int getScrollHeight() const { return scroll_height; }

  /**
   * Adjusts a visible area to a given position.
   */
  virtual void adjustScroll(int newx, int newy);
  /**
   * Returns a visible scroll area coordinates.
   */
  virtual Point getScrollPosition()
    { return Point(scroll_xpos, scroll_ypos); }
  /**
   * Returns a visible scroll area x position.
   */
  virtual int getScrollPositionX() { return scroll_xpos; }
  /**
   * Returns a visible scroll area y position.
   */
  virtual int getScrollPositionY() { return scroll_ypos; }

  /**
   * Adjusts a scroll area to make a given position visible.
   */
  virtual void makeVisible(int x, int y);
  /**
   * Adjusts a scroll area to make a given area visible.
   */
  virtual void makeVisible(int x, int y, int w, int h);

protected:
  int scroll_xpos, scroll_ypos, scroll_width, scroll_height;
  bool update_screen_area;

  Curses::Window *screen_area;

  // Widget
  virtual void updateArea();
  virtual void proceedUpdateArea();

  virtual void updateVirtualArea();
  virtual void proceedUpdateVirtualArea();

  virtual void drawEx(bool container_draw);
  virtual bool makePointVisible(int x, int y);

private:
  CONSUI_DISABLE_COPY(ScrollPane);
};

} // namespace CppConsUI

#endif // __SCROLLPANE_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
