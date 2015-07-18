/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 by CenterIM developers
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

#include <algorithm>
#include <cassert>

namespace CppConsUI {

MenuWindow::MenuWindow(int x, int y, int w, int h, const char *title)
  : Window(x, y, w, h, title, TYPE_TOP), wish_height(3), ref(NULL), xshift(0),
    yshift(0), hide_on_close(false)
{
  wish_width = MENU_WINDOW_WISH_WIDTH;

  listbox = new ListBox(AUTOSIZE, AUTOSIZE);
  listbox->signal_children_height_change.connect(
    sigc::mem_fun(this, &MenuWindow::onChildrenHeightChange));
  Window::addWidget(*listbox, 1, 1);
}

MenuWindow::MenuWindow(Widget &ref_, int w, int h, const char *title)
  : Window(0, 0, w, h, title, TYPE_TOP), wish_height(3), ref(NULL), xshift(0),
    yshift(0), hide_on_close(false)
{
  wish_width = MENU_WINDOW_WISH_WIDTH;

  listbox = new ListBox(AUTOSIZE, AUTOSIZE);
  listbox->signal_children_height_change.connect(
    sigc::mem_fun(this, &MenuWindow::onChildrenHeightChange));
  Window::addWidget(*listbox, 1, 1);

  setReferenceWidget(ref_);
}

MenuWindow::~MenuWindow()
{
  cleanReferenceWidget();
}

void MenuWindow::onAbsolutePositionChange(Widget &widget)
{
  if (&widget == ref)
    updatePositionAndSize();
  Window::onAbsolutePositionChange(widget);
}

void MenuWindow::show()
{
  if (ref) {
    assert(!ref_visible_conn.connected());

    ref_visible_conn = ref->signal_visible.connect(
      sigc::mem_fun(this, &MenuWindow::onReferenceWidgetVisible));
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

void MenuWindow::onScreenResized()
{
  updatePositionAndSize();
}

Button *MenuWindow::insertSubMenu(
  size_t pos, const char *title, MenuWindow &submenu)
{
  Button *button = prepareSubMenu(title, submenu);
  listbox->insertWidget(pos, *button);
  return button;
}

Button *MenuWindow::appendSubMenu(const char *title, MenuWindow &submenu)
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

void MenuWindow::setReferenceWidget(Widget &new_ref)
{
  if (ref == &new_ref)
    return;

  // clean the current reference
  cleanReferenceWidget();

  ref = &new_ref;
  ref->add_destroy_notify_callback(this, onReferenceWidgetDestroy_);
  ref->registerAbsolutePositionListener(*this);
  updatePositionAndSize();
}

void MenuWindow::cleanReferenceWidget()
{
  if (!ref)
    return;

  ref->remove_destroy_notify_callback(this);
  ref->unregisterAbsolutePositionListener(*this);
  ref = NULL;
}

void MenuWindow::setLeftShift(int x)
{
  if (xshift == x)
    return;

  xshift = x;
  updatePositionAndSize();
}

void MenuWindow::setTopShift(int y)
{
  if (yshift == y)
    return;

  yshift = y;
  updatePositionAndSize();
}

void MenuWindow::addWidget(Widget &widget, int x, int y)
{
  Window::addWidget(widget, x, y);
}

Button *MenuWindow::prepareSubMenu(const char *title, MenuWindow &submenu)
{
  // setup submenu correctly
  submenu.hide();
  submenu.setHideOnClose(true);
  signal_hide.connect(sigc::hide(sigc::mem_fun(submenu, &MenuWindow::hide)));

  // create an opening button
  Button *button = new Button(title);
  button->signal_activate.connect(
    sigc::hide(sigc::mem_fun(submenu, &MenuWindow::show)));

  submenu.setReferenceWidget(*button);

  return button;
}

void MenuWindow::updatePositionAndSize()
{
  /* This code is called when a position or size of this window should be
   * updated.
   *
   * This can happen when:
   * - the screen is resized,
   * - the listbox wish height changed,
   * - reference widget changed its position on the screen.
   */

  if (!ref) {
    // absolute screen position
    int h = listbox->getChildrenHeight() + 2;
    int max = Curses::getHeight() - ypos;
    if (h > max)
      setWishHeight(std::max(max, 3));
    else
      setWishHeight(h);
    return;
  }

  // relative position to another widget
  Point p = ref->getAbsolutePosition();
  if (p.getX() == UNSETPOS || p.getY() == UNSETPOS)
    p = Point(0, 0);

  int x = p.getX() + xshift;
  int y = p.getY() + yshift;

  int above = y;
  int below = Curses::getHeight() - y - 1;
  int req_h;
  if (height == AUTOSIZE)
    req_h = listbox->getChildrenHeight() + 2;
  else
    req_h = height;

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
  else if (height == AUTOSIZE) {
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

void MenuWindow::onChildrenHeightChange(
  ListBox & /*activator*/, int /*new_height*/)
{
  if (height != AUTOSIZE)
    return;

  updatePositionAndSize();
}

void MenuWindow::onReferenceWidgetVisible(Widget & /*activator*/, bool visible)
{
  if (visible)
    return;

  // close this window if the reference widget is no longer visible
  close();
}

void MenuWindow::onReferenceWidgetDestroy()
{
  // ref widget is about to die right now, this window should be destroyed too
  assert(ref);
  ref = NULL;
  delete this;
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
