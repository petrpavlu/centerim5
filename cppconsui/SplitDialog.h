/*
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
 * SplitDialog class.
 *
 * @ingroup cppconsui
 */

#ifndef __SPLITDIALOG_H__
#define __SPLITDIALOG_H__

#include "AbstractDialog.h"

namespace CppConsUI
{

class SplitDialog
: public AbstractDialog
{
public:
  SplitDialog(int x, int y, int w, int h, const char *title = NULL);
  explicit SplitDialog(const char *title = NULL);
  virtual ~SplitDialog();

  // Widget
  virtual void cleanFocus();

  // Container
  virtual void moveFocus(FocusDirection direction);

  virtual void setContainer(Container& cont);
  virtual Container *getContainer() const { return container; }

  /**
   * Signal emitted when the user closes the dialog.
   */
  sigc::signal<void, SplitDialog&, ResponseType> signal_response;

protected:
  Container *container;

  Widget *cont_old_focus;
  Widget *buttons_old_focus;
  sigc::connection cont_old_focus_conn;
  sigc::connection buttons_old_focus_conn;

  // AbstractDialog
  virtual void emitResponse(ResponseType response);

  virtual void onOldFocusVisible(Widget& activator, bool visible);

private:
  CONSUI_DISABLE_COPY(SplitDialog);
};

} // namespace CppConsUI

#endif // __SPLITDIALOG_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
