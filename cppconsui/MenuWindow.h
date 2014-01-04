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
 * MenuWindow class.
 *
 * @ingroup cppconsui
 */

#ifndef __MENUWINDOW_H__
#define __MENUWINDOW_H__

#include "ListBox.h"
#include "Window.h"

#define MENU_WINDOW_WISH_WIDTH 40

namespace CppConsUI
{

class MenuWindow
: public Window
{
public:
  MenuWindow(int x, int y, int w, int h, const char *title = NULL);
  MenuWindow(Widget& ref_, int w, int h, const char *title = NULL);
  virtual ~MenuWindow();

  // Widget
  virtual void draw();

  // FreeWindow
  virtual void show();
  virtual void hide();
  virtual void close();

  virtual Button *insertItem(size_t pos, const char *title,
      const sigc::slot<void, Button&>& callback)
    { return listbox->insertItem(pos, title, callback); }
  virtual Button *appendItem(const char *title,
      const sigc::slot<void, Button&>& callback)
    { return listbox->appendItem(title, callback); }
  virtual AbstractLine *insertSeparator(size_t pos)
    { return listbox->insertSeparator(pos); }
  virtual AbstractLine *appendSeparator()
    { return listbox->appendSeparator(); }
  virtual Button *insertSubMenu(size_t pos, const char *title,
      MenuWindow& submenu);
  virtual Button *appendSubMenu(const char *title,
      MenuWindow& submenu);
  virtual void insertWidget(size_t pos, Widget& widget)
    { listbox->insertWidget(pos, widget); }
  virtual void appendWidget(Widget& widget)
    { listbox->appendWidget(widget); }

  virtual void setHideOnClose(bool new_hide_on_close);
  virtual int getHideOnClose() const { return hide_on_close; }

  /**
   * Assigns a reference widget which is used to calculate on-screen position
   * of this MenuWindow. Note that if the reference widget is destroyed then
   * the MenuWindow dies too.
   */
  virtual void setRefWidget(Widget& new_ref);
  /**
   * Removes any reference widget assignment.
   */
  virtual void cleanRefWidget();
  /**
   * Returns the currently set reference widget.
   */
  virtual Widget *getRefWidget() const { return ref; }

  virtual int getLeftShift() const { return xshift; }
  virtual int getTopShift() const { return yshift; }

  virtual void setLeftShift(int x);
  virtual void setTopShift(int y);

protected:
  ListBox *listbox;
  int wish_height;

  Widget *ref;
  int xshift, yshift;
  sigc::connection ref_visible_conn;

  bool hide_on_close;

  // Container
  virtual void addWidget(Widget& widget, int x, int y);

  // FreeWindow
  virtual void onScreenResizedInternal();

  virtual Button *prepareSubMenu(const char *title, MenuWindow& submenu);

  /**
   * Recalculates desired on-screen position and size of this window. This
   * handles mainly autosize magic.
   */
  virtual void updateSmartPositionAndSize();

  virtual void onChildrenHeightChange(ListBox& activator, int new_height);

  virtual void onRefWidgetVisible(Widget& activator, bool visible);

  static void *onRefWidgetDestroy_(void *win)
    { reinterpret_cast<MenuWindow*>(win)->onRefWidgetDestroy(); return NULL; }
  virtual void onRefWidgetDestroy();

private:
  CONSUI_DISABLE_COPY(MenuWindow);
};

} // namespace CppConsUI

#endif // __MENUWINDOW_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
