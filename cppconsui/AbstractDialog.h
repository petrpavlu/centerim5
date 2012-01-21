/*
 * Copyright (C) 2011 by CenterIM developers
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
 * AbstractDialog class.
 *
 * @ingroup cppconsui
 */

#ifndef __ABSTRACTDIALOG_H__
#define __ABSTRACTDIALOG_H__

#define OK_BUTTON_TEXT _("Ok")
#define CANCEL_BUTTON_TEXT _("Cancel")
#define YES_BUTTON_TEXT _("Yes")
#define NO_BUTTON_TEXT _("No")

#include "HorizontalLine.h"
#include "HorizontalListBox.h"
#include "ListBox.h"
#include "Window.h"

namespace CppConsUI
{

class AbstractDialog
: public Window
{
public:
  enum ResponseType {
    RESPONSE_OK,
    RESPONSE_CANCEL, ///< Cancel button or close dialog.
    RESPONSE_YES,
    RESPONSE_NO
  };

  AbstractDialog(int x, int y, int w, int h, const char *title = NULL);
  explicit AbstractDialog(const char *title = NULL);
  virtual ~AbstractDialog() {}

  // FreeWindow
  virtual void Close();

  virtual void AddButton(const char *label, ResponseType response);
  virtual void AddSeparator();
  virtual void Response(ResponseType response);

protected:
  ListBox *layout;
  HorizontalLine *separator;
  HorizontalListBox *buttons;

  virtual void InitLayout();
  virtual void EmitResponse(ResponseType response) = 0;
  virtual void OnButtonResponse(Button& activator, ResponseType response);

private:
  AbstractDialog(const AbstractDialog&);
  AbstractDialog& operator=(const AbstractDialog&);
};

} // namespace CppConsUI

#endif // __ABSTRACTDIALOG_H__

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
