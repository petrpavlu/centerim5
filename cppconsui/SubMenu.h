/*
 * Copyright (C) 2011 by CenterIM developers
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Softwareee the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * */

/**
 * @file
 * SubMenu class.
 *
 * @ingroup cppconsui
 */

#ifndef __SUBMENU_H__
#define __SUBMENU_H__

#include "Button.h"
#include "FlowMenuWindow.h"

namespace CppConsUI
{

/**
 * SubMenu is a button that opens a MenuWindow when activated.
 */
class SubMenu
: public Button
{
public:
  SubMenu(int w, int h, const char *text_ = NULL);
  explicit SubMenu(const char *text_ = NULL);
  virtual ~SubMenu();

  virtual Button *InsertItem(size_t pos, const char *title,
      const sigc::slot<void, Button&>& callback)
    { return submenu->InsertItem(pos, title, callback); }
  virtual Button *AppendItem(const char *title,
      const sigc::slot<void, Button&>& callback)
    { return submenu->AppendItem(title, callback); }
  virtual AbstractLine *InsertSeparator(size_t pos)
    { return submenu->InsertSeparator(pos); }
  virtual AbstractLine *AppendSeparator()
    { return submenu->AppendSeparator(); }
  virtual void InsertWidget(size_t pos, Widget& widget)
    { submenu->InsertWidget(pos, widget); }
  virtual void AppendWidget(Widget& widget)
    { submenu->AppendWidget(widget); }

  virtual void Close()
    { submenu->Close(); }

  //sigc::signal<void, SubMenu&> signal_submenu_open;

protected:
  class ExtMenuWindow
  : public FlowMenuWindow
  {
  public:
    ExtMenuWindow(SubMenu& ref);
    virtual ~ExtMenuWindow() {}

    // FreeWindow
    virtual void Close();

  protected:

  private:
    ExtMenuWindow(const ExtMenuWindow&);
    ExtMenuWindow& operator=(const ExtMenuWindow&);
  };

  ExtMenuWindow *submenu;

  sigc::connection parent_window_close_conn;

  virtual void OnActivate(Button& activator);
  virtual void OnParentWindowClose(FreeWindow& activator);

private:
  SubMenu(const SubMenu&);
  SubMenu& operator=(const SubMenu&);
};

} // namespace CppConsUI

#endif // __SUBMENU_H__
