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
 * MenuWindow class implementation.
 *
 * @ingroup cppconsui
 */

#include "MenuWindow.h"

namespace CppConsUI
{

MenuWindow::MenuWindow(int x, int y, int w, int h, const char *title)
: Window(x, y, w, h, title, TYPE_TOP)
, wish_height(3), ref(NULL), xshift(0), yshift(0), hide_on_close(false)
{
  wish_width = MENU_WINDOW_WISH_WIDTH;

  listbox = new ListBox(AUTOSIZE, AUTOSIZE);
  listbox->signal_children_height_change.connect(sigc::mem_fun(this,
        &MenuWindow::onChildrenHeightChange));
  Window::addWidget(*listbox, 0, 0);
}

MenuWindow::MenuWindow(Widget& ref_, int w, int h, const char *title)
: Window(0, 0, w, h, title, TYPE_TOP)
, wish_height(3), ref(NULL), xshift(0), yshift(0), hide_on_close(false)
{
  wish_width = MENU_WINDOW_WISH_WIDTH;

  listbox = new ListBox(AUTOSIZE, AUTOSIZE);
  listbox->signal_children_height_change.connect(sigc::mem_fun(this,
        &MenuWindow::onChildrenHeightChange));
  Window::addWidget(*listbox, 0, 0);

  setRefWidget(ref_);
}

MenuWindow::~MenuWindow()
{
  if (ref)
    ref->remove_destroy_notify_callback(this);
}

void MenuWindow::draw()
{
  updateSmartPositionAndSize();

  Window::draw();
}

void MenuWindow::show()
{
  if (ref) {
    g_assert(!ref_visible_conn.connected());

    ref_visible_conn = ref->signal_visible.connect(sigc::mem_fun(this,
          &MenuWindow::onRefWidgetVisible));
  }

  if (hide_on_close) {
    // make sure that the first widget in the focus chain is always focused
    listbox->cleanFocus();
    listbox->moveFocus(Container::FOCUS_DOWN);
  }

  Window::show();
}

void MenuWindow::hide()
{
  if (ref)
    ref_visible_conn.disconnect();

  Window::hide();
}

void MenuWindow::close()
{
  if (hide_on_close)
    hide();
  else
    Window::close();
}

Button *MenuWindow::insertSubMenu(size_t pos, const char *title,
    MenuWindow& submenu)
{
  Button *button = prepareSubMenu(title, submenu);
  listbox->insertWidget(pos, *button);
  return button;
}

Button *MenuWindow::appendSubMenu(const char *title, MenuWindow& submenu)
{
  Button *button = prepareSubMenu(title, submenu);
  listbox->appendWidget(*button);
  return button;
}

void MenuWindow::setHideOnClose(bool new_hide_on_close)
{
  if (hide_on_close == new_hide_on_close)
    return;

  hide_on_close = new_hide_on_close;
}

void MenuWindow::setRefWidget(Widget& new_ref)
{
  if (ref == &new_ref)
    return;

  if (ref)
    ref->remove_destroy_notify_callback(this);

  ref = &new_ref;
  ref->add_destroy_notify_callback(this, onRefWidgetDestroy_);
  if (visible)
    redraw();
}

void MenuWindow::cleanRefWidget()
{
  if (!ref)
    return;

  ref->remove_destroy_notify_callback(this);
  ref = NULL;
  if (visible)
    redraw();
}

void MenuWindow::setLeftShift(int x)
{
  if (xshift == x)
    return;

  xshift = x;
  if (visible)
    redraw();
}

void MenuWindow::setTopShift(int y)
{
  if (yshift == y)
    return;

  yshift = y;
  if (visible)
    redraw();
}

void MenuWindow::addWidget(Widget& widget, int x, int y)
{
  Window::addWidget(widget, x, y);
}

void MenuWindow::onScreenResizedInternal()
{
  updateSmartPositionAndSize();
  Window::onScreenResizedInternal();
}

Button *MenuWindow::prepareSubMenu(const char *title, MenuWindow& submenu)
{
  // setup submenu correctly
  submenu.hide();
  submenu.setHideOnClose(true);
  signal_hide.connect(sigc::hide(sigc::mem_fun(submenu, &MenuWindow::hide)));

  // create an opening button
  Button *button = new Button(title);
  button->signal_activate.connect(sigc::hide(sigc::mem_fun(submenu,
          &MenuWindow::show)));

  submenu.setRefWidget(*button);

  return button;
}

void MenuWindow::updateSmartPositionAndSize()
{
  /* This code is called when position or size of this window should be
   * updated.
   *
   * This can happen when:
   * - the screen is resized,
   * - listbox wish height changed,
   * - reference widget changed its position on the screen.
   *
   * Unfortunately the position of the reference widget isn't known until the
   * widget is drawn on the screen. This happens just before
   * FlowMenuWindow::draw() is called thus this method has to be ultimately
   * called from that method.
   *
   * Note that none of the below called methods (move(), setWishHeight())
   * doesn't trigger update-area procedure if it isn't really necessary.
   */

  if (!ref) {
    // absolute screen position
    int h = listbox->getChildrenHeight() + 2;
    int max = Curses::getmaxy() - win_y;
    if (h > max)
      setWishHeight(MAX(max, 3));
    else
      setWishHeight(h);
  }
  else {
    // relative screen position
    Point p = ref->getAbsolutePosition();
    int x = p.getX() + xshift;
    int y = p.getY() + yshift;

    int above = y;
    int below = Curses::getmaxy() - y - 1;
    int req_h;
    if (win_h == AUTOSIZE)
      req_h = listbox->getChildrenHeight() + 2;
    else
      req_h = win_h;

    if (below > req_h) {
      // draw the window under the combobox
      move(x, y + 1);
      setWishHeight(req_h);
    }
    else if (above > req_h) {
      // draw the window above the combobox
      move(x, y - req_h);
      setWishHeight(req_h);
    }
    else if (win_h == AUTOSIZE) {
      if (below >= above) {
        move(x, y + 1);
        setWishHeight(below);
      }
      else {
        move(x, 0);
        setWishHeight(above);
      }
    }
  }
}

void MenuWindow::onChildrenHeightChange(ListBox& /*activator*/,
    int /*new_height*/)
{
  if (win_h != AUTOSIZE)
    return;

  updateSmartPositionAndSize();
}

void MenuWindow::onRefWidgetVisible(Widget& /*activator*/, bool visible)
{
  if (visible)
    return;

  // close this window if the reference widget is no longer visible
  close();
}

void MenuWindow::onRefWidgetDestroy()
{
  // ref widget is about to die right now, this window should be destroyed too
  g_assert(ref);
  ref = NULL;
  delete this;
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
