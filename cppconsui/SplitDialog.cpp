/*
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
 * SplitDialog class implementation.
 *
 * @ingroup cppconsui
 */

#include "SplitDialog.h"

#include "gettext.h"

SplitDialog::SplitDialog(int x, int y, int w, int h, const char *title,
    LineStyle::Type ltype)
: AbstractDialog(x, y, w, h, title, ltype), container(NULL)
, cont_old_focus(NULL) , buttons_old_focus(NULL)
{
  buttons->SetFocusCycle(Container::FOCUS_CYCLE_LOCAL);
}

SplitDialog::SplitDialog(const char *title, LineStyle::Type ltype)
: AbstractDialog(title, ltype), container(NULL), cont_old_focus(NULL)
, buttons_old_focus(NULL)
{
  buttons->SetFocusCycle(Container::FOCUS_CYCLE_LOCAL);
}

SplitDialog::~SplitDialog()
{
  /* Slots has to be disconnected manually because sigc::~trackable() is
   * called too late. */
  cont_old_focus_conn.disconnect();
  cont_old_focus = NULL;
  buttons_old_focus_conn.disconnect();
  buttons_old_focus = NULL;
}

void SplitDialog::CleanFocus()
{
  Widget *f = layout->GetFocusChild();
  if (f) {
    if (f == container) {
      cont_old_focus_conn.disconnect();
      cont_old_focus = container->GetFocusWidget();
      if (cont_old_focus)
        cont_old_focus_conn = cont_old_focus->signal_delete.connect(
            sigc::mem_fun(this, &SplitDialog::OnOldFocusDelete));
    }
    else if (f == buttons) {
      buttons_old_focus_conn.disconnect();
      buttons_old_focus = buttons->GetFocusWidget();
      if (buttons_old_focus)
        buttons_old_focus_conn = buttons_old_focus->signal_delete.connect(
            sigc::mem_fun(this, &SplitDialog::OnOldFocusDelete));
    }
  }

  AbstractDialog::CleanFocus();
}

void SplitDialog::MoveFocus(FocusDirection direction)
{
  if (!container) {
    AbstractDialog::MoveFocus(direction);
    return;
  }

  /**
   * @todo Rewrite to take advantage of focus chain caching.
   */

  switch (direction) {
    case FOCUS_PREVIOUS:
      if (layout->GetFocusChild() == container) {
        // focus is held by the container, give it to the last button
        FocusChain focus_chain(NULL);
        buttons->GetFocusChain(focus_chain, focus_chain.begin());

        FocusChain::pre_order_iterator iter = --focus_chain.end();
        if (*iter && (*iter)->GrabFocus())
          return;
      }
      else if (layout->GetFocusChild() == buttons) {
        FocusChain focus_chain(NULL);
        buttons->GetFocusChain(focus_chain, focus_chain.begin());

        FocusChain::leaf_iterator iter = focus_chain.begin_leaf();
        if (GetFocusWidget() == *iter) {
          // focus is held by the first button, give it to the container
          if ((cont_old_focus && cont_old_focus->GrabFocus())
              || container->GrabFocus())
            return;
        }
      }
      break;
    case FOCUS_NEXT:
      if (layout->GetFocusChild() == container) {
        // focus is held by the container, give it to the first button
        if (buttons->GrabFocus())
          return;
      }
      else if (layout->GetFocusChild() == buttons) {
        FocusChain focus_chain(NULL);
        buttons->GetFocusChain(focus_chain, focus_chain.begin());

        FocusChain::pre_order_iterator iter = --focus_chain.end();
        if (GetFocusWidget() == *iter) {
          // focus is held by the last button, give it to the container
          if ((cont_old_focus && cont_old_focus->GrabFocus())
              || container->GrabFocus())
            return;
        }
      }
      break;
    case FOCUS_LEFT:
    case FOCUS_RIGHT:
      if (layout->GetFocusChild() != buttons) {
        /* First try to focus the previously focused widget, if it fails then
         * try any widget. */
        if ((buttons_old_focus && buttons_old_focus->GrabFocus())
            || buttons->GrabFocus())
          return;
      }
      break;
    case FOCUS_UP:
    case FOCUS_DOWN:
      if (layout->GetFocusChild() != container) {
        /* First try to focus the previously focused widget, if it fails then
         * try any widget. */
        if ((cont_old_focus && cont_old_focus->GrabFocus())
            || container->GrabFocus())
          return;
      }
      break;
    default:
      break;
  }
  AbstractDialog::MoveFocus(direction);
}

void SplitDialog::SetContainer(Container& cont)
{
  g_assert(!container);
  g_warn_if_fail(cont.GetWidth() == AUTOSIZE);
  g_warn_if_fail(cont.GetHeight() == AUTOSIZE);

  container = &cont;
  cont.SetFocusCycle(Container::FOCUS_CYCLE_LOCAL);
  layout->InsertWidget(0, cont);
}

void SplitDialog::EmitResponse(SplitDialog::ResponseType response)
{
  signal_response(*this, response);
}

void SplitDialog::OnOldFocusDelete(Widget& activator)
{
  if (&activator == cont_old_focus) {
    cont_old_focus_conn.disconnect();
    cont_old_focus = NULL;
  }
  else if (&activator == buttons_old_focus) {
    buttons_old_focus_conn.disconnect();
    buttons_old_focus = NULL;
  }
  else
    g_assert_not_reached();
}
