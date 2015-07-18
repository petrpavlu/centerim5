/*
 * Copyright (C) 2011-2015 Petr Pavlu <setup@dagobah.cz>
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

namespace CppConsUI {

class AbstractDialog : public Window {
public:
  enum ResponseType {
    RESPONSE_OK,
    RESPONSE_CANCEL, ///< Cancel button or close dialog.
    RESPONSE_YES,
    RESPONSE_NO,
  };

  AbstractDialog(int x, int y, int w, int h, const char *title = NULL);
  explicit AbstractDialog(const char *title = NULL);
  virtual ~AbstractDialog() {}

  // FreeWindow
  virtual void close();

  virtual void addButton(const char *label, ResponseType response);
  virtual void addSeparator();
  virtual void response(ResponseType response_type);

protected:
  ListBox *layout;
  HorizontalLine *separator;
  HorizontalListBox *buttons;

  virtual void initLayout();
  virtual void emitResponse(ResponseType response) = 0;
  virtual void onButtonResponse(Button &activator, ResponseType response_type);

private:
  CONSUI_DISABLE_COPY(AbstractDialog);
};

} // namespace CppConsUI

#endif // __ABSTRACTDIALOG_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
