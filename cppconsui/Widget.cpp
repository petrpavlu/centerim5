/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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
 * Widget class implementation.
 *
 * @ingroup cppconsui
 */

#include "Widget.h"

#include "ColorScheme.h"
#include "CoreManager.h"
#include "Container.h"

#include <string>

Widget::Widget(int w, int h)
: xpos(UNSET), ypos(UNSET), width(w), height(h), can_focus(false)
, has_focus(false), visible(true), area(NULL), update_area(false)
, parent(NULL) , color_scheme(NULL)
{
}

Widget::~Widget()
{
  SetVisibility(false);

  if (area)
    delete area;
  if (color_scheme)
    g_free(color_scheme);

  signal_delete(*this);
}

void Widget::MoveResize(int newx, int newy, int neww, int newh)
{
  if (newx == xpos && newy == ypos && neww == width && newh == height)
    return;

  Rect oldsize(xpos, ypos, width, height);
  Rect newsize(newx, newy, neww, newh);

  xpos = newx;
  ypos = newy;
  width = neww;
  height = newh;

  UpdateArea();
  signal_moveresize(*this, oldsize, newsize);
}

void Widget::UpdateArea()
{
  update_area = true;
  Redraw();
}

Widget *Widget::GetFocusWidget()
{
  if (can_focus)
    return this;
  return NULL;
}

void Widget::CleanFocus()
{
  if (!has_focus)
    return;

  has_focus = false;
  signal_focus(*this, false);
  Redraw();
}

bool Widget::RestoreFocus()
{
  return GrabFocus();
}

void Widget::UngrabFocus()
{
  if (!parent || !has_focus)
    return;

  has_focus = false;
  signal_focus(*this, false);
  Redraw();
}

bool Widget::GrabFocus()
{
  if (!parent || has_focus)
    return false;

  if (can_focus && IsVisibleRecursive()) {
    if (parent->SetFocusChild(*this)) {
      has_focus = true;
      signal_focus(*this, true);
      Redraw();
    }
    return true;
  }

  return false;
}

void Widget::SetVisibility(bool visible)
{
  if (this->visible != visible) {
    this->visible = visible;

    Container *t = GetTopContainer();
    if (t) {
      t->UpdateFocusChain();

      if (visible) {
        if (!t->GetFocusWidget()) {
          /* There is no focused widget, try if this or a widget
           * that was revealed can grab it. */
          t->MoveFocus(Container::FOCUS_DOWN);
        }
      }
      else {
        Widget *focus = t->GetFocusWidget();
        if (focus && !focus->IsVisibleRecursive()) {
          // focused widget was hidden, move the focus
          t->MoveFocus(Container::FOCUS_DOWN);
        }
      }
    }

    signal_visible(*this, visible);
    Redraw();
  }
}

bool Widget::IsVisibleRecursive() const
{
  if (!parent || !visible)
    return false;

  return parent->IsWidgetVisible(*this);
}

void Widget::SetParent(Container& parent)
{
  // we don't support parent change
  g_assert(!this->parent);

  this->parent = &parent;

  Container *t = GetTopContainer();
  t->UpdateFocusChain();
  if (!t->GetFocusWidget()) {
    /* There is no focused widget, try if this or a child widget (in case
     * of Container) can grab it. */
    Widget *w = GetFocusWidget();
    if (w)
      w->GrabFocus();
  }
  else {
    /* Clean focus if this widget/container was added to a container that
     * already has a focused widget. */
    CleanFocus();
  }

  UpdateArea();
}

Point Widget::GetAbsolutePosition() const
{
  if (!parent)
    return Point(0, 0);

  return parent->GetAbsolutePosition(*this);
}

void Widget::SetColorScheme(const char *scheme)
{
  if (color_scheme)
    g_free(color_scheme);

  if (scheme)
    color_scheme = g_strdup(scheme);
  else
    color_scheme = NULL;

  Redraw();
}

const char *Widget::GetColorScheme() const
{
  if (color_scheme)
    return color_scheme;
  else if (parent)
    return parent->GetColorScheme();

  return NULL;
}

void Widget::RealUpdateArea()
{
  g_assert(parent);

  if (update_area) {
    if (area)
      delete area;
    area = parent->GetSubPad(*this, xpos, ypos, width, height);
    update_area = false;
  }
}

void Widget::Redraw()
{
  FreeWindow *win = dynamic_cast<FreeWindow*>(GetTopContainer());
  if (win && COREMANAGER->HasWindow(*win))
    COREMANAGER->Redraw();
}

int Widget::GetColorPair(const char *widget, const char *property) const
{
  return COLORSCHEME->GetColorPair(GetColorScheme(), widget, property);
}

Container *Widget::GetTopContainer()
{
  if (parent)
    return parent->GetTopContainer();
  return dynamic_cast<Container*>(this);
}
