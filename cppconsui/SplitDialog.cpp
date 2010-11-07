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
, container_index(0)
, buttons_index(0)
{
  buttons->SetFocusCycle(Container::FOCUS_CYCLE_LOCAL);
}

SplitDialog::SplitDialog(const gchar *title, LineStyle::Type ltype)
: Dialog(title, ltype)
, container(NULL)
, container_index(0)
, buttons_index(0)
{
  buttons->SetFocusCycle(Container::FOCUS_CYCLE_LOCAL);
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
        // focus is held by container, give it to the last button
        container_index = container->GetActive();

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
          buttons_index = buttons->GetActive();
          if (container->SetActive(container_index) || container->GrabFocus())
            return;
        }
      }
      break;
    case FOCUS_NEXT:
      if (layout->GetFocusChild() == container) {
        // focus is held by container, give it to the first button
        container_index = container->GetActive();
        if (buttons->GrabFocus())
          return;
      }
      else if (layout->GetFocusChild() == buttons) {
        FocusChain focus_chain(NULL);
        buttons->GetFocusChain(focus_chain, focus_chain.begin());

        FocusChain::pre_order_iterator iter = --focus_chain.end();
        if (GetFocusWidget() == *iter) {
          // focus is held by the last button, give it to the container
          buttons_index = buttons->GetActive();
          if (container->SetActive(container_index) || container->GrabFocus())
            return;
        }
      }
      break;
    case FOCUS_LEFT:
    case FOCUS_RIGHT:
      if (layout->GetFocusChild() != buttons) {
        container_index = container->GetActive();
        /* First try to focus the previously focused widget, if it fails then
         * try any widget. */
        if (buttons->SetActive(buttons_index) || buttons->GrabFocus())
          return;
      }
      break;
    case FOCUS_UP:
    case FOCUS_DOWN:
      if (layout->GetFocusChild() != container) {
        buttons_index = buttons->GetActive();
        /* First try to focus the previously focused widget, if it fails then
         * try any widget. */
        if (container->SetActive(container_index) || container->GrabFocus())
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
