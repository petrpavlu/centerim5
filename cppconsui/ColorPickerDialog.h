/*
 * Copyright (C) 2012-2013 by CenterIM developers
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
 * ColorPickerDialog class.
 *
 * @ingroup cppconsui
 */

#ifndef __COLORPICKERDIALOG_H__
#define __COLORPICKERDIALOG_H__

#include "AbstractDialog.h"
#include "ColorScheme.h"
#include "ColorPickerPalette.h"

namespace CppConsUI
{

class ColorPickerDialog
: public AbstractDialog
{
public:
  ColorPickerDialog(const char *title, int default_color, int flags);
  virtual ~ColorPickerDialog() {}

  /**
   * Signal emitted when the user closes the dialog.
   */
  sigc::signal<void, ColorPickerDialog&, ResponseType, int> signal_response;

protected:
  int color;

  // AbstractDialog
  virtual void emitResponse(ResponseType response);

  virtual void onColorSelected(ColorPickerPalette& activator, int new_color);

private:
  CONSUI_DISABLE_COPY(ColorPickerDialog);
};

} // namespace CppConsUI

#endif // __COLORPICKERDIALOG_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
