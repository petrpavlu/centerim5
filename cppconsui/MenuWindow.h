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
 * MenuWindow class.
 *
 * @ingroup cppconsui
 */

#ifndef __MENUWINDOW_H__
#define __MENUWINDOW_H__

#include "Window.h"
#include "ListBox.h"

class MenuWindow
: public Window
{
public:
  MenuWindow(int x, int y, int w, int h, const gchar *title = NULL,
      LineStyle::Type ltype = LineStyle::DEFAULT);
  virtual ~MenuWindow() {}

  virtual Button *AppendItem(const char *text,
      sigc::slot<void, Button&> callback)
    { return listbox->AppendItem(text, callback); }
  virtual HorizontalLine *AppendSeparator()
    { return listbox->AppendSeparator(); }

  virtual void AppendWidget(Widget& widget)
    { listbox->AppendWidget(widget); }
  virtual void RemoveWidget(Widget& widget)
    { listbox->RemoveWidget(widget); }

protected:
  ListBox *listbox;

  // Container
  virtual void AddWidget(Widget& widget, int x, int y);

private:
  MenuWindow(const MenuWindow&);
  MenuWindow& operator=(const MenuWindow&);
};

#endif // __MENUWINDOW_H__
