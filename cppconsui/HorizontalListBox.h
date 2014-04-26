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
 * HorizontalListBox class.
 *
 * @ingroup cppconsui
 */

#ifndef __HORIZONTALLISTBOX_H__
#define __HORIZONTALLISTBOX_H__

#include "AbstractListBox.h"
#include "VerticalLine.h"

namespace CppConsUI
{

/**
 * Implementation of AbstractListBox class where widgets are placed
 * horizontally.
 */
class HorizontalListBox
: public AbstractListBox
{
public:
  HorizontalListBox(int w, int h);
  virtual ~HorizontalListBox() {}

  // Widget
  virtual void updateArea();
  virtual void draw();

  // AbstractListBox
  virtual VerticalLine *insertSeparator(size_t pos);
  virtual VerticalLine *appendSeparator();
  virtual void insertWidget(size_t pos, Widget& widget);
  virtual void appendWidget(Widget& widget);

  // Container
  virtual Curses::Window *getSubPad(const Widget& child, int begin_x,
      int begin_y, int ncols, int nlines);

  virtual int getChildrenWidth() const { return children_width; };

  sigc::signal<void, HorizontalListBox&, int> signal_children_width_change;

protected:
  int children_width;
  int autosize_children;
  int autosize_width;
  std::set<const Widget*> autosize_extra;
  bool reposition_widgets;

  // Container
  virtual void onChildMoveResize(Widget& activator, const Rect& oldsize,
      const Rect& newsize);
  virtual void onChildVisible(Widget& widget, bool visible);

  virtual void updateScrollWidth();

private:
  CONSUI_DISABLE_COPY(HorizontalListBox);
};

} // namespace CppConsUI

#endif // __HORIZONTALLISTBOX_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
