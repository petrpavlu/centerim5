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
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// AbstractListBox class.
///
/// @ingroup cppconsui

#ifndef ABSTRACTLISTBOX_H
#define ABSTRACTLISTBOX_H

#include "AbstractLine.h"
#include "Button.h"
#include "Container.h"

namespace CppConsUI {

/// Abstract class that defines common interface for ListBox and
/// HorizontalListBox.
class AbstractListBox : public Container {
public:
  AbstractListBox(int w, int h);
  virtual ~AbstractListBox() override {}

  /// Inserts a new button into ListBox before a given position.
  virtual Button *insertItem(std::size_t pos, const char *title,
    const sigc::slot<void, Button &> &callback);

  /// Adds a new button in the end of ListBox.
  virtual Button *appendItem(
    const char *title, const sigc::slot<void, Button &> &callback);

  /// Inserts a separator (usually a horizontal or vertical line) into the
  /// ListBox before a given position.
  virtual AbstractLine *insertSeparator(std::size_t pos) = 0;

  /// Appends a separator (usually a horizontal or vertical line) into the
  /// ListBox.
  virtual AbstractLine *appendSeparator() = 0;

  /// Inserts a widget into the ListBox before a given position.
  virtual void insertWidget(std::size_t pos, Widget &widget) = 0;

  /// Appends a widget into the ListBox.
  virtual void appendWidget(Widget &widget) = 0;

protected:
  // Container
  using Container::addWidget;

private:
  CONSUI_DISABLE_COPY(AbstractListBox);
};

} // namespace CppConsUI

#endif // ABSTRACTLISTBOX_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
