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
  virtual void Draw();

  // AbstractListBox
  virtual VerticalLine *InsertSeparator(size_t pos);
  virtual VerticalLine *AppendSeparator();
  virtual void InsertWidget(size_t pos, Widget& widget);
  virtual void AppendWidget(Widget& widget);

  // Container
  virtual Curses::Window *GetSubPad(const Widget& child, int begin_x,
      int begin_y, int ncols, int nlines);

  virtual int GetChildrenWidth() const { return children_width; };

  sigc::signal<void, HorizontalListBox&, int> signal_children_width_change;

protected:
  int children_width;
  int autosize_children;
  int autosize_width;
  std::set<const Widget*> autosize_extra;
  bool reposition_widgets;

  // Container
  virtual void OnChildMoveResize(Widget& activator, const Rect& oldsize,
      const Rect& newsize);
  virtual void OnChildVisible(Widget& widget, bool visible);

  virtual void UpdateScrollWidth();

private:
  HorizontalListBox(const HorizontalListBox&);
  HorizontalListBox& operator=(const HorizontalListBox&);
};

} // namespace CppConsUI

#endif // __HORIZONTALLISTBOX_H__
