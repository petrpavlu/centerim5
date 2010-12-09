/*
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
 * SplitDialog class implementation.
 *
 * @ingroup cppconsui
 */

#include "SplitDialog.h"

#include "gettext.h"

SplitDialog::SplitDialog(int x, int y, int w, int h, const gchar *title,
    LineStyle::Type ltype)
: Dialog(x, y, w, h, title, ltype)
, container(NULL)
, old_focus(NULL)
{
  buttons->SetFocusCycle(Container::FOCUS_CYCLE_LOCAL);
}

SplitDialog::SplitDialog(const gchar *title, LineStyle::Type ltype)
: Dialog(title, ltype)
, container(NULL)
, old_focus(NULL)
{
  buttons->SetFocusCycle(Container::FOCUS_CYCLE_LOCAL);
}

SplitDialog::~SplitDialog()
{
  old_focus_conn.disconnect();
  old_focus = NULL;
}

void SplitDialog::MoveFocus(FocusDirection direction)
{
  if (!container) {
    Dialog::MoveFocus(direction);
    return;
  }

  switch (direction) {
    case FOCUS_PREVIOUS:
      if (layout->GetFocusChild() == container) {
        // focus is held by the container, give it to the last button
        old_focus = GetFocusWidget();
        old_focus_conn.disconnect();
        old_focus_conn = old_focus->signal_delete.connect(sigc::mem_fun(this,
              &SplitDialog::OnOldFocusDelete));

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
          Widget *new_focus = old_focus;
          old_focus = GetFocusWidget();
          old_focus_conn.disconnect();
          old_focus_conn = old_focus->signal_delete.connect(sigc::mem_fun(
                this, &SplitDialog::OnOldFocusDelete));

          if ((new_focus && new_focus->GrabFocus()) || container->GrabFocus())
            return;
        }
      }
      break;
    case FOCUS_NEXT:
      if (layout->GetFocusChild() == container) {
        // focus is held by the container, give it to the first button
        old_focus = GetFocusWidget();
        old_focus_conn.disconnect();
        old_focus_conn = old_focus->signal_delete.connect(sigc::mem_fun(this,
              &SplitDialog::OnOldFocusDelete));

        if (buttons->GrabFocus())
          return;
      }
      else if (layout->GetFocusChild() == buttons) {
        FocusChain focus_chain(NULL);
        buttons->GetFocusChain(focus_chain, focus_chain.begin());

        FocusChain::pre_order_iterator iter = --focus_chain.end();
        if (GetFocusWidget() == *iter) {
          // focus is held by the last button, give it to the container
          Widget *new_focus = old_focus;
          old_focus = GetFocusWidget();
          old_focus_conn.disconnect();
          old_focus_conn = old_focus->signal_delete.connect(sigc::mem_fun(
                this, &SplitDialog::OnOldFocusDelete));

          if ((new_focus && new_focus->GrabFocus()) || container->GrabFocus())
            return;
        }
      }
      break;
    case FOCUS_LEFT:
    case FOCUS_RIGHT:
      if (layout->GetFocusChild() != buttons) {
        Widget *new_focus = old_focus;
        old_focus = GetFocusWidget();
        old_focus_conn.disconnect();
        old_focus_conn = old_focus->signal_delete.connect(sigc::mem_fun(this,
              &SplitDialog::OnOldFocusDelete));

        /* First try to focus the previously focused widget, if it fails then
         * try any widget. */
        if ((new_focus && new_focus->GrabFocus()) || buttons->GrabFocus())
          return;
      }
      break;
    case FOCUS_UP:
    case FOCUS_DOWN:
      if (layout->GetFocusChild() != container) {
        Widget *new_focus = old_focus;
        old_focus = GetFocusWidget();
        old_focus_conn.disconnect();
        old_focus_conn = old_focus->signal_delete.connect(sigc::mem_fun(this,
              &SplitDialog::OnOldFocusDelete));

        /* First try to focus the previously focused widget, if it fails then
         * try any widget. */
        if ((new_focus && new_focus->GrabFocus()) || container->GrabFocus())
          return;
      }
      break;
    default:
      break;
  }
  Dialog::MoveFocus(direction);
}

void SplitDialog::SetContainer(Container& cont)
{
  g_assert(!container);
  g_warn_if_fail(cont.Width() == AUTOSIZE);
  g_warn_if_fail(cont.Height() == AUTOSIZE);

  container = &cont;
  cont.SetFocusCycle(Container::FOCUS_CYCLE_LOCAL);
  layout->InsertWidget(0, cont);
}

void SplitDialog::OnOldFocusDelete(Widget& activator)
{
  g_assert(old_focus == &activator);

  old_focus_conn.disconnect();
  old_focus = NULL;
}
