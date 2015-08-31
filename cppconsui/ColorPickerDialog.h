// Copyright (C) 2012 Mark Pustjens <pustjens@dds.nl>
// Copyright (C) 2012-2015 Petr Pavlu <setup@dagobah.cz>
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
/// ColorPickerDialog class.
///
/// @ingroup cppconsui

#ifndef COLORPICKERDIALOG_H
#define COLORPICKERDIALOG_H

#include "AbstractDialog.h"
#include "ColorScheme.h"
#include "ColorPickerPalette.h"

namespace CppConsUI {

class ColorPickerDialog : public AbstractDialog {
public:
  ColorPickerDialog(const char *title, int default_color, int flags);
  virtual ~ColorPickerDialog() {}

  /// Signal emitted when user closes the dialog.
  sigc::signal<void, ColorPickerDialog &, ResponseType, int> signal_response;

protected:
  int color_;

  // AbstractDialog
  virtual void emitResponse(ResponseType response);

  virtual void onColorSelected(ColorPickerPalette &activator, int new_color);

private:
  CONSUI_DISABLE_COPY(ColorPickerDialog);
};

} // namespace CppConsUI

#endif // COLORPICKERDIALOG_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
