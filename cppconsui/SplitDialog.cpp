/*
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
 * SplitDialog class implementation.
 *
 * @ingroup cppconsui
 */

#include "SplitDialog.h"

#include <cassert>

namespace CppConsUI
{

SplitDialog::SplitDialog(int x, int y, int w, int h, const char *title)
: AbstractDialog(x, y, w, h, title), container(NULL)
, cont_old_focus(NULL) , buttons_old_focus(NULL)
{
  buttons->setFocusCycle(Container::FOCUS_CYCLE_LOCAL);
}

SplitDialog::SplitDialog(const char *title)
: AbstractDialog(title), container(NULL), cont_old_focus(NULL)
, buttons_old_focus(NULL)
{
  buttons->setFocusCycle(Container::FOCUS_CYCLE_LOCAL);
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

void SplitDialog::cleanFocus()
{
  Widget *f = layout->getFocusChild();
  if (f) {
    if (f == container) {
      cont_old_focus_conn.disconnect();
      cont_old_focus = container->getFocusWidget();
      if (cont_old_focus)
        cont_old_focus_conn = cont_old_focus->signal_visible.connect(
            sigc::mem_fun(this, &SplitDialog::onOldFocusVisible));
    }
    else if (f == buttons) {
      buttons_old_focus_conn.disconnect();
      buttons_old_focus = buttons->getFocusWidget();
      if (buttons_old_focus)
        buttons_old_focus_conn = buttons_old_focus->signal_visible.connect(
            sigc::mem_fun(this, &SplitDialog::onOldFocusVisible));
    }
  }

  AbstractDialog::cleanFocus();
}

void SplitDialog::moveFocus(FocusDirection direction)
{
  if (!container) {
    AbstractDialog::moveFocus(direction);
    return;
  }

  /**
   * @todo Rewrite to take advantage of focus chain caching.
   */

  switch (direction) {
    case FOCUS_PREVIOUS:
      if (layout->getFocusChild() == container) {
        // focus is held by the container, give it to the last button
        FocusChain focus_chain(NULL);
        buttons->getFocusChain(focus_chain, focus_chain.begin());

        FocusChain::pre_order_iterator iter = --focus_chain.end();
        if (*iter && (*iter)->grabFocus())
          return;
      }
      else if (layout->getFocusChild() == buttons) {
        FocusChain focus_chain(NULL);
        buttons->getFocusChain(focus_chain, focus_chain.begin());

        FocusChain::leaf_iterator iter = focus_chain.begin_leaf();
        if (getFocusWidget() == *iter) {
          // focus is held by the first button, give it to the container
          if ((cont_old_focus && cont_old_focus->grabFocus()) ||
              container->grabFocus())
            return;
        }
      }
      break;
    case FOCUS_NEXT:
      if (layout->getFocusChild() == container) {
        // focus is held by the container, give it to the first button
        if (buttons->grabFocus())
          return;
      }
      else if (layout->getFocusChild() == buttons) {
        FocusChain focus_chain(NULL);
        buttons->getFocusChain(focus_chain, focus_chain.begin());

        FocusChain::pre_order_iterator iter = --focus_chain.end();
        if (getFocusWidget() == *iter) {
          // focus is held by the last button, give it to the container
          if ((cont_old_focus && cont_old_focus->grabFocus()) ||
              container->grabFocus())
            return;
        }
      }
      break;
    case FOCUS_LEFT:
    case FOCUS_RIGHT:
      if (layout->getFocusChild() != buttons) {
        /* First try to focus the previously focused widget, if it fails then
         * try any widget. */
        if ((buttons_old_focus && buttons_old_focus->grabFocus()) ||
            buttons->grabFocus())
          return;
      }
      break;
    case FOCUS_UP:
    case FOCUS_DOWN:
      if (layout->getFocusChild() != container) {
        /* First try to focus the previously focused widget, if it fails then
         * try any widget. */
        if ((cont_old_focus && cont_old_focus->grabFocus()) ||
            container->grabFocus())
          return;
      }
      break;
    default:
      break;
  }
  AbstractDialog::moveFocus(direction);
}

void SplitDialog::setContainer(Container &cont)
{
  assert(!container);

  container = &cont;
  cont.setFocusCycle(Container::FOCUS_CYCLE_LOCAL);
  layout->insertWidget(0, cont);
}

void SplitDialog::emitResponse(SplitDialog::ResponseType response)
{
  signal_response(*this, response);
}

void SplitDialog::onOldFocusVisible(Widget &activator, bool visible)
{
  if (visible)
    return;

  if (&activator == cont_old_focus) {
    cont_old_focus_conn.disconnect();
    cont_old_focus = NULL;
  }
  else if (&activator == buttons_old_focus) {
    buttons_old_focus_conn.disconnect();
    buttons_old_focus = NULL;
  }
  else
    assert(0);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
