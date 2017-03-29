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
/// MenuWindow class implementation.
///
/// @ingroup cppconsui

#include "MenuWindow.h"

#include <algorithm>
#include <cassert>

namespace CppConsUI {

MenuWindow::MenuWindow(int x, int y, int w, int h, const char *title)
  : Window(x, y, w, h, title, TYPE_TOP), wish_height_(3), ref_(nullptr),
    xshift_(0), yshift_(0), hide_on_close_(false)
{
  wish_width_ = MENU_WINDOW_WISH_WIDTH;

  listbox_ = new ListBox(AUTOSIZE, AUTOSIZE);
  listbox_->signal_children_height_change.connect(
    sigc::mem_fun(this, &MenuWindow::onChildrenHeightChange));
  Window::addWidget(*listbox_, 1, 1);
}

MenuWindow::MenuWindow(Widget &ref, int w, int h, const char *title)
  : Window(0, 0, w, h, title, TYPE_TOP), wish_height_(3), ref_(nullptr),
    xshift_(0), yshift_(0), hide_on_close_(false)
{
  wish_width_ = MENU_WINDOW_WISH_WIDTH;

  listbox_ = new ListBox(AUTOSIZE, AUTOSIZE);
  listbox_->signal_children_height_change.connect(
    sigc::mem_fun(this, &MenuWindow::onChildrenHeightChange));
  Window::addWidget(*listbox_, 1, 1);

  setReferenceWidget(ref);
}

MenuWindow::~MenuWindow()
{
  cleanReferenceWidget();
}

void MenuWindow::onAbsolutePositionChange(Widget &widget)
{
  if (&widget == ref_)
    updatePositionAndSize();
  Window::onAbsolutePositionChange(widget);
}

void MenuWindow::show()
{
  if (ref_ != nullptr) {
    assert(!ref_visible_conn_.connected());

    ref_visible_conn_ = ref_->signal_visible.connect(
      sigc::mem_fun(this, &MenuWindow::onReferenceWidgetVisible));
  }

  if (hide_on_close_) {
    // Make sure that the first widget in the focus chain is always focused.
    listbox_->cleanFocus();
    listbox_->moveFocus(Container::FOCUS_DOWN);
  }

  Window::show();
}

void MenuWindow::hide()
{
  if (ref_ != nullptr)
    ref_visible_conn_.disconnect();

  Window::hide();
}

void MenuWindow::close()
{
  if (hide_on_close_)
    hide();
  else
    Window::close();
}

void MenuWindow::onScreenResized()
{
  updatePositionAndSize();
}

Button *MenuWindow::insertSubMenu(
  std::size_t pos, const char *title, MenuWindow &submenu)
{
  Button *button = prepareSubMenu(title, submenu);
  listbox_->insertWidget(pos, *button);
  return button;
}

Button *MenuWindow::appendSubMenu(const char *title, MenuWindow &submenu)
{
  Button *button = prepareSubMenu(title, submenu);
  listbox_->appendWidget(*button);
  return button;
}

void MenuWindow::setHideOnClose(bool new_hide_on_close)
{
  if (hide_on_close_ == new_hide_on_close)
    return;

  hide_on_close_ = new_hide_on_close;
}

void MenuWindow::setReferenceWidget(Widget &new_ref)
{
  if (ref_ == &new_ref)
    return;

  // Clean the current reference.
  cleanReferenceWidget();

  ref_ = &new_ref;
  ref_->add_destroy_notify_callback(this, onReferenceWidgetDestroy_);
  ref_->registerAbsolutePositionListener(*this);
  updatePositionAndSize();
}

void MenuWindow::cleanReferenceWidget()
{
  if (ref_ == nullptr)
    return;

  ref_->remove_destroy_notify_callback(this);
  ref_->unregisterAbsolutePositionListener(*this);
  ref_ = nullptr;
}

void MenuWindow::setLeftShift(int x)
{
  if (xshift_ == x)
    return;

  xshift_ = x;
  updatePositionAndSize();
}

void MenuWindow::setTopShift(int y)
{
  if (yshift_ == y)
    return;

  yshift_ = y;
  updatePositionAndSize();
}

void MenuWindow::addWidget(Widget &widget, int x, int y)
{
  Window::addWidget(widget, x, y);
}

Button *MenuWindow::prepareSubMenu(const char *title, MenuWindow &submenu)
{
  // Setup submenu correctly.
  submenu.hide();
  submenu.setHideOnClose(true);
  signal_hide.connect(sigc::hide(sigc::mem_fun(submenu, &MenuWindow::hide)));

  // Create an opening button.
  auto button = new Button(title);
  button->signal_activate.connect(
    sigc::hide(sigc::mem_fun(submenu, &MenuWindow::show)));

  submenu.setReferenceWidget(*button);

  return button;
}

void MenuWindow::updatePositionAndSize()
{
  // This code is called when a position or size of this window should be
  // updated.
  //
  // This can happen when:
  // - the screen is resized,
  // - the listbox wish height changed,
  // - reference widget changed its position on the screen.

  if (ref_ == nullptr) {
    // Absolute screen position.
    int h = listbox_->getChildrenHeight() + 2;
    int max = Curses::getHeight() - ypos_;
    if (h > max)
      setWishHeight(std::max(max, 3));
    else
      setWishHeight(h);
    return;
  }

  // Relative position to another widget.
  Point p = ref_->getAbsolutePosition();
  if (p.getX() == UNSETPOS || p.getY() == UNSETPOS)
    p = Point(0, 0);

  int x = p.getX() + xshift_;
  int y = p.getY() + yshift_;

  int above = y;
  int below = Curses::getHeight() - y - 1;
  int req_h;
  if (height_ == AUTOSIZE)
    req_h = listbox_->getChildrenHeight() + 2;
  else
    req_h = height_;

  if (below > req_h) {
    // Draw the window under the combobox.
    move(x, y + 1);
    setWishHeight(req_h);
  }
  else if (above > req_h) {
    // Draw the window above the combobox.
    move(x, y - req_h);
    setWishHeight(req_h);
  }
  else if (height_ == AUTOSIZE) {
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
  if (height_ != AUTOSIZE)
    return;

  updatePositionAndSize();
}

void MenuWindow::onReferenceWidgetVisible(Widget & /*activator*/, bool visible)
{
  if (visible)
    return;

  // Close this window if the reference widget is no longer visible.
  close();
}

void MenuWindow::onReferenceWidgetDestroy()
{
  // Ref widget is about to die right now, this window should be destroyed too.
  assert(ref_ != nullptr);
  ref_ = nullptr;
  delete this;
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
