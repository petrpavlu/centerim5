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
 * Widget class implementation.
 *
 * @ingroup cppconsui
 */

#include "Widget.h"

#include "ColorScheme.h"
#include "CoreManager.h"

#include <cassert>
#include <cstring>

namespace CppConsUI
{

Widget::Widget(int w, int h)
: xpos(UNSETPOS), ypos(UNSETPOS), width(w), height(h), wish_width(AUTOSIZE)
, wish_height(AUTOSIZE), real_xpos(UNSETPOS), real_ypos(UNSETPOS)
, real_width(0), real_height(0), can_focus(false), has_focus(false)
, visible(true), parent(NULL), color_scheme(NULL)
{
}

Widget::~Widget()
{
  setVisibility(false);

  delete [] color_scheme;
}

void Widget::moveResize(int newx, int newy, int neww, int newh)
{
  if (newx == xpos && newy == ypos && neww == width && newh == height)
    return;

  Rect oldsize(xpos, ypos, width, height);
  Rect newsize(newx, newy, neww, newh);

  xpos = newx;
  ypos = newy;
  width = neww;
  height = newh;

  signalMoveResize(oldsize, newsize);
}

Widget *Widget::getFocusWidget()
{
  if (can_focus)
    return this;
  return NULL;
}

void Widget::cleanFocus()
{
  if (!has_focus)
    return;

  has_focus = false;
  signal_focus(*this, false);
  redraw();
}

bool Widget::restoreFocus()
{
  return grabFocus();
}

void Widget::ungrabFocus()
{
  if (!parent || !has_focus)
    return;

  has_focus = false;
  signal_focus(*this, false);
  redraw();
}

bool Widget::grabFocus()
{
  if (!parent || has_focus)
    return false;

  if (can_focus && isVisibleRecursive()) {
    if (parent->setFocusChild(*this)) {
      has_focus = true;
      signal_focus(*this, true);
      redraw();
    }
    return true;
  }

  return false;
}

void Widget::setVisibility(bool new_visible)
{
  if (new_visible == visible)
    return;

  visible = new_visible;

  if (parent) {
    parent->updateFocusChain();

    Container *t = getTopContainer();
    if (visible) {
      if (!t->getFocusWidget()) {
        /* There is no focused widget, try if this or a widget
         * that was revealed can grab it. */
        t->moveFocus(Container::FOCUS_DOWN);
      }
    }
    else {
      Widget *focus = t->getFocusWidget();
      if (focus && !focus->isVisibleRecursive()) {
        // focused widget was hidden, move the focus
        t->moveFocus(Container::FOCUS_DOWN);
      }
    }

    signalVisible(visible);
  }

  signal_visible(*this, visible);
  redraw();
}

bool Widget::isVisibleRecursive() const
{
  if (!parent || !visible)
    return false;

  return parent->isWidgetVisible(*this);
}

void Widget::setParent(Container& new_parent)
{
  // we don't support parent change
  assert(!parent);

  parent = &new_parent;

  parent->updateFocusChain();

  Container *t = getTopContainer();
  if (!t->getFocusWidget()) {
    /* There is no focused widget, try if this or a child widget (in case
     * of Container) can grab it. */
    Widget *w = getFocusWidget();
    if (w)
      w->grabFocus();
  }
  else {
    /* Clean focus if this widget/container was added to a container that
     * already has a focused widget. */
    cleanFocus();
  }
}

/* All following moveResize() wrappers should always call member methods to
 * get xpos, ypos, width, height and never use member variables directly. See
 * Window getLeft(), getTop(), getWidth(), getHeight(). */
void Widget::move(int newx, int newy)
{
  moveResize(newx, newy, getWidth(), getHeight());
}

void Widget::resize(int neww, int newh)
{
  moveResize(getLeft(), getTop(), neww, newh);
}

void Widget::setLeft(int newx)
{
  moveResize(newx, getTop(), getWidth(), getHeight());
}

void Widget::setTop(int newy)
{
  moveResize(getLeft(), newy, getWidth(), getHeight());
}

void Widget::setWidth(int neww)
{
  moveResize(getLeft(), getTop(), neww, getHeight());
}

void Widget::setHeight(int newh)
{
  moveResize(getLeft(), getTop(), getWidth(), newh);
}

Point Widget::getAbsolutePosition() const
{
  if (!parent)
    return Point(0, 0);

  return parent->getAbsolutePosition(*this);
}

Point Widget::getRelativePosition(const Container& ref) const
{
  if (!parent)
    return Point(0, 0);

  return parent->getRelativePosition(ref, *this);
}

void Widget::setRealPosition(int newx, int newy)
{
  real_xpos = newx;
  real_ypos = newy;
}

void Widget::setRealSize(int neww, int newh)
{
  if (neww == real_width && newh == real_height)
    return;

  real_width = neww;
  real_height = newh;

  updateArea();
}

void Widget::setColorScheme(const char *new_color_scheme)
{
  delete [] color_scheme;

  if (new_color_scheme) {
    size_t size = std::strlen(new_color_scheme) + 1;
    color_scheme = new char[size];
    std::strcpy(color_scheme, new_color_scheme);
  }
  else
    color_scheme = NULL;

  redraw();
}

const char *Widget::getColorScheme() const
{
  if (color_scheme)
    return color_scheme;
  else if (parent)
    return parent->getColorScheme();

  return NULL;
}

void Widget::updateArea()
{
  // empty for Widget
}

void Widget::signalMoveResize(const Rect& oldsize, const Rect& newsize)
{
  if (!parent)
    return;

  /* Tell the parent about the new size so it can position the widget
   * correctly. */
  parent->onChildMoveResize(*this, oldsize, newsize);
}

void Widget::signalWishSizeChange(const Size& oldsize,
    const Size& newsize)
{
  if (!parent)
    return;

  /* Tell the parent about the new wish size so it can position the widget
   * correctly. */
  parent->onChildWishSizeChange(*this, oldsize, newsize);
}

void Widget::signalVisible(bool visible)
{
  if (!parent)
    return;

  /* Tell the parent about the new visibility status so it can reposition its
   * child widgets as appropriate. */
  parent->onChildVisible(*this, visible);
}

void Widget::redraw()
{
  if (!parent)
    return;

  COREMANAGER->redraw();
}

void Widget::setWishSize(int neww, int newh)
{
  if (neww == wish_width && newh == wish_height)
    return;

  Size oldsize(wish_width, wish_height);
  Size newsize(neww, newh);

  wish_width = neww;
  wish_height = newh;

  signalWishSizeChange(oldsize, newsize);
}

int Widget::getColorPair(const char *widget, const char *property) const
{
  return COLORSCHEME->getColorPair(getColorScheme(), widget, property);
}

Container *Widget::getTopContainer()
{
  if (parent)
    return parent->getTopContainer();
  return dynamic_cast<Container*>(this);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
