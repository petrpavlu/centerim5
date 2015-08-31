// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
// Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
//
// This file is part of CenterIM.
//
// CenterIM is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// CenterIM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// ListBox class.
///
/// @ingroup cppconsui

#ifndef LISTBOX_H
#define LISTBOX_H

#include "AbstractListBox.h"
#include "HorizontalLine.h"

namespace CppConsUI {

/// Implementation of AbstractListBox class where widgets are placed vertically.
class ListBox : public AbstractListBox {
public:
  ListBox(int w, int h);
  virtual ~ListBox() {}

  // AbstractListBox
  virtual HorizontalLine *insertSeparator(size_t pos);
  virtual HorizontalLine *appendSeparator();
  virtual void insertWidget(size_t pos, Widget &widget);
  virtual void appendWidget(Widget &widget);

  virtual int getChildrenHeight() const { return children_height_; };

  sigc::signal<void, ListBox &, int> signal_children_height_change;

protected:
  /// Total height of all visible children.
  int children_height_;

  /// Number of visible children that has their height set to AUTOSIZE.
  int autosize_children_count_;

  // Widget
  virtual void updateArea();

  // Container
  virtual void onChildMoveResize(
    Widget &activator, const Rect &oldsize, const Rect &newsize);
  virtual void onChildWishSizeChange(
    Widget &activator, const Size &oldsize, const Size &newsize);
  virtual void onChildVisible(Widget &activator, bool visible);

  virtual void updateChildren(
    int children_height_change, int autosize_children_count_change);

private:
  CONSUI_DISABLE_COPY(ListBox);
};

} // namespace CppConsUI

#endif // LISTBOX_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
