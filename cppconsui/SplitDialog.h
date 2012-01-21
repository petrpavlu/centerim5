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
 * SplitDialog class.
 *
 * @ingroup cppconsui
 */

#ifndef __SPLITDIALOG_H__
#define __SPLITDIALOG_H__

#include "AbstractDialog.h"
#include "Container.h"

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
  virtual void CleanFocus();

  // Container
  virtual void MoveFocus(FocusDirection direction);

  virtual void SetContainer(Container& cont);
  virtual Container *GetContainer() const { return container; }

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
  virtual void EmitResponse(ResponseType response);

  virtual void OnOldFocusVisible(Widget& activator, bool visible);

private:
  SplitDialog(const SplitDialog&);
  SplitDialog& operator=(const SplitDialog&);
};

} // namespace CppConsUI

#endif // __SPLITDIALOG_H__

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
