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
 * AbstractListBox class.
 *
 * @ingroup cppconsui
 */

#ifndef __ABSTRACTLISTBOX_H__
#define __ABSTRACTLISTBOX_H__

#include "AbstractLine.h"
#include "Button.h"
#include "ScrollPane.h"

/**
 * Abstract class that defines common interface for ListBox and
 * HorizontalListBox.
 */
class AbstractListBox
: public ScrollPane
{
public:
  AbstractListBox(int w, int h);
  virtual ~AbstractListBox() {}

  /**
   * Inserts a new button into ListBox before a given position.
   */
  Button *InsertItem(size_t pos, const gchar *title,
      sigc::slot<void, Button&> function);
  /**
   * Adds a new button in the end of ListBox.
   */
  Button *AppendItem(const gchar *title, sigc::slot<void, Button&> function);
  /**
   * Inserts a separator (usually a horizontal or vertical line) into the
   * ListBox before a given position.
   */
  virtual AbstractLine *InsertSeparator(size_t pos) = 0;
  /**
   * Appends a separator (usually a horizontal or vertical line) into the
   * ListBox.
   */
  virtual AbstractLine *AppendSeparator() = 0;
  /**
   * Inserts a widget into the ListBox before a given position.
   */
  virtual void InsertWidget(size_t pos, Widget& widget) = 0;
  /**
   * Appends a widget into the ListBox.
   */
  virtual void AppendWidget(Widget& widget) = 0;

protected:
  // Container
  virtual void AddWidget(Widget& widget, int x, int y);

private:
  AbstractListBox(const AbstractListBox&);
  AbstractListBox& operator=(const AbstractListBox&);
};

#endif // __ABSTRACTLISTBOX_H__
