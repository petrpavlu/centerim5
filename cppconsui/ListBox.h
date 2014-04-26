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
 * ListBox class.
 *
 * @ingroup cppconsui
 */

#ifndef __LISTBOX_H__
#define __LISTBOX_H__

#include "AbstractListBox.h"
#include "HorizontalLine.h"

namespace CppConsUI
{

/**
 * Implementation of AbstractListBox class where widgets are placed
 * vertically.
 */
class ListBox
: public AbstractListBox
{
public:
  ListBox(int w, int h);
  virtual ~ListBox() {}

  // Widget
  virtual void updateArea();
  virtual void draw();

  // AbstractListBox
  virtual HorizontalLine *insertSeparator(size_t pos);
  virtual HorizontalLine *appendSeparator();
  virtual void insertWidget(size_t pos, Widget& widget);
  virtual void appendWidget(Widget& widget);

  // Container
  virtual Curses::Window *getSubPad(const Widget& child, int begin_x,
      int begin_y, int ncols, int nlines);

  virtual int getChildrenHeight() const { return children_height; };

  sigc::signal<void, ListBox&, int> signal_children_height_change;

protected:
  int children_height;
  int autosize_children;
  int autosize_height;
  std::set<const Widget*> autosize_extra;
  bool reposition_widgets;

  // Container
  virtual void onChildMoveResize(Widget& activator, const Rect& oldsize,
      const Rect& newsize);
  virtual void onChildVisible(Widget& activator, bool visible);

  virtual void updateScrollHeight();

private:
  CONSUI_DISABLE_COPY(ListBox);
};

} // namespace CppConsUI

#endif // __LISTBOX_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
