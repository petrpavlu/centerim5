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
/// MenuWindow class.
///
/// @ingroup cppconsui

#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include "ListBox.h"
#include "Window.h"

#define MENU_WINDOW_WISH_WIDTH 40

namespace CppConsUI {

class MenuWindow : public Window {
public:
  MenuWindow(int x, int y, int w, int h, const char *title = nullptr);
  MenuWindow(Widget &ref, int w, int h, const char *title = nullptr);
  virtual ~MenuWindow() override;

  // Widget
  virtual void onAbsolutePositionChange(Widget &widget) override;

  // Window
  virtual void show() override;
  virtual void hide() override;
  virtual void close() override;
  virtual void onScreenResized() override;

  virtual Button *insertItem(std::size_t pos, const char *title,
    const sigc::slot<void, Button &> &callback)
  {
    return listbox_->insertItem(pos, title, callback);
  }
  virtual Button *appendItem(
    const char *title, const sigc::slot<void, Button &> &callback)
  {
    return listbox_->appendItem(title, callback);
  }
  virtual AbstractLine *insertSeparator(std::size_t pos)
  {
    return listbox_->insertSeparator(pos);
  }
  virtual AbstractLine *appendSeparator()
  {
    return listbox_->appendSeparator();
  }
  virtual Button *insertSubMenu(
    std::size_t pos, const char *title, MenuWindow &submenu);
  virtual Button *appendSubMenu(const char *title, MenuWindow &submenu);
  virtual void insertWidget(std::size_t pos, Widget &widget)
  {
    listbox_->insertWidget(pos, widget);
  }
  virtual void appendWidget(Widget &widget) { listbox_->appendWidget(widget); }

  virtual void setHideOnClose(bool new_hide_on_close);
  virtual int getHideOnClose() const { return hide_on_close_; }

  /// Assigns a reference widget which is used to calculate on-screen position
  /// of this MenuWindow. Note that if the reference widget is destroyed then
  /// the MenuWindow dies too.
  virtual void setReferenceWidget(Widget &new_ref);

  /// Removes any reference widget assignment.
  virtual void cleanReferenceWidget();

  /// Returns the currently set reference widget.
  virtual Widget *getReferenceWidget() const { return ref_; }

  virtual int getLeftShift() const { return xshift_; }
  virtual int getTopShift() const { return yshift_; }

  virtual void setLeftShift(int x);
  virtual void setTopShift(int y);

protected:
  ListBox *listbox_;
  int wish_height_;

  Widget *ref_;
  int xshift_, yshift_;
  sigc::connection ref_visible_conn_;

  bool hide_on_close_;

  // Container
  virtual void addWidget(Widget &widget, int x, int y) override;

  virtual Button *prepareSubMenu(const char *title, MenuWindow &submenu);

  /// Recalculates desired on-screen position and size of this window. This
  /// handles mainly autosize magic.
  virtual void updatePositionAndSize();

  virtual void onChildrenHeightChange(ListBox &activator, int new_height);

  virtual void onReferenceWidgetVisible(Widget &activator, bool visible);

  static void *onReferenceWidgetDestroy_(void *win)
  {
    reinterpret_cast<MenuWindow *>(win)->onReferenceWidgetDestroy();
    return nullptr;
  }
  virtual void onReferenceWidgetDestroy();

private:
  CONSUI_DISABLE_COPY(MenuWindow);
};

} // namespace CppConsUI

#endif // MENUWINDOW_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
