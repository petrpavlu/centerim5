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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// SplitDialog class implementation.
///
/// @ingroup cppconsui

#include "SplitDialog.h"

#include <cassert>

namespace CppConsUI {

SplitDialog::SplitDialog(int x, int y, int w, int h, const char *title)
  : AbstractDialog(x, y, w, h, title), container_(NULL), cont_old_focus_(NULL),
    buttons_old_focus_(NULL)
{
  buttons_->setFocusCycle(Container::FOCUS_CYCLE_LOCAL);
}

SplitDialog::SplitDialog(const char *title)
  : AbstractDialog(title), container_(NULL), cont_old_focus_(NULL),
    buttons_old_focus_(NULL)
{
  buttons_->setFocusCycle(Container::FOCUS_CYCLE_LOCAL);
}

SplitDialog::~SplitDialog()
{
  // Slots has to be disconnected manually because sigc::~trackable() is called
  // too late.
  cont_old_focus_conn_.disconnect();
  cont_old_focus_ = NULL;
  buttons_old_focus_conn_.disconnect();
  buttons_old_focus_ = NULL;
}

void SplitDialog::cleanFocus()
{
  Widget *f = layout_->getFocusChild();
  if (f != NULL) {
    if (f == container_) {
      cont_old_focus_conn_.disconnect();
      cont_old_focus_ = container_->getFocusWidget();
      if (cont_old_focus_ != NULL)
        cont_old_focus_conn_ = cont_old_focus_->signal_visible.connect(
          sigc::mem_fun(this, &SplitDialog::onOldFocusVisible));
    }
    else if (f == buttons_) {
      buttons_old_focus_conn_.disconnect();
      buttons_old_focus_ = buttons_->getFocusWidget();
      if (buttons_old_focus_ != NULL)
        buttons_old_focus_conn_ = buttons_old_focus_->signal_visible.connect(
          sigc::mem_fun(this, &SplitDialog::onOldFocusVisible));
    }
  }

  AbstractDialog::cleanFocus();
}

void SplitDialog::moveFocus(FocusDirection direction)
{
  if (container_ == NULL) {
    AbstractDialog::moveFocus(direction);
    return;
  }

  /// @todo Rewrite to take advantage of focus chain caching.

  switch (direction) {
  case FOCUS_PREVIOUS:
    if (layout_->getFocusChild() == container_) {
      // Focus is held by the container, give it to the last button.
      FocusChain focus_chain(NULL);
      buttons_->getFocusChain(focus_chain, focus_chain.begin());

      FocusChain::pre_order_iterator iter = --focus_chain.end();
      if (*iter != NULL && (*iter)->grabFocus())
        return;
    }
    else if (layout_->getFocusChild() == buttons_) {
      FocusChain focus_chain(NULL);
      buttons_->getFocusChain(focus_chain, focus_chain.begin());

      FocusChain::leaf_iterator iter = focus_chain.begin_leaf();
      if (getFocusWidget() == *iter) {
        // focus is held by the first button, give it to the container
        if ((cont_old_focus_ && cont_old_focus_->grabFocus()) ||
          container_->grabFocus())
          return;
      }
    }
    break;
  case FOCUS_NEXT:
    if (layout_->getFocusChild() == container_) {
      // Focus is held by the container, give it to the first button.
      if (buttons_->grabFocus())
        return;
    }
    else if (layout_->getFocusChild() == buttons_) {
      FocusChain focus_chain(NULL);
      buttons_->getFocusChain(focus_chain, focus_chain.begin());

      FocusChain::pre_order_iterator iter = --focus_chain.end();
      if (getFocusWidget() == *iter) {
        // Focus is held by the last button, give it to the container.
        if ((cont_old_focus_ && cont_old_focus_->grabFocus()) ||
          container_->grabFocus())
          return;
      }
    }
    break;
  case FOCUS_LEFT:
  case FOCUS_RIGHT:
    if (layout_->getFocusChild() != buttons_) {
      // First try to focus the previously focused widget, if it fails then try
      // any widget.
      if ((buttons_old_focus_ && buttons_old_focus_->grabFocus()) ||
        buttons_->grabFocus())
        return;
    }
    break;
  case FOCUS_UP:
  case FOCUS_DOWN:
    if (layout_->getFocusChild() != container_) {
      // First try to focus the previously focused widget, if it fails then try
      // any widget.
      if ((cont_old_focus_ && cont_old_focus_->grabFocus()) ||
        container_->grabFocus())
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
  assert(container_ == NULL);

  container_ = &cont;
  cont.setFocusCycle(Container::FOCUS_CYCLE_LOCAL);
  layout_->insertWidget(0, cont);
}

void SplitDialog::emitResponse(SplitDialog::ResponseType response)
{
  signal_response(*this, response);
}

void SplitDialog::onOldFocusVisible(Widget &activator, bool visible)
{
  if (visible)
    return;

  if (&activator == cont_old_focus_) {
    cont_old_focus_conn_.disconnect();
    cont_old_focus_ = NULL;
  }
  else if (&activator == buttons_old_focus_) {
    buttons_old_focus_conn_.disconnect();
    buttons_old_focus_ = NULL;
  }
  else
    assert(0);
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
