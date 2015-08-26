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
/// ColorPickerDialog class implementation.
///
/// @ingroup cppconsui

#include "ColorPickerDialog.h"

#include "gettext.h"

namespace CppConsUI {

ColorPickerDialog::ColorPickerDialog(
  const char *title, int default_color, int flags)
  : AbstractDialog(title)
{
  addButton(OK_BUTTON_TEXT, RESPONSE_OK);

  ColorPickerPalette *pick = new ColorPickerPalette(default_color, flags);
  layout_->insertWidget(0, *pick);
  pick->signal_color_selected.connect(
    sigc::mem_fun(this, &ColorPickerDialog::onColorSelected));

  resize(pick->getWidth() + 2, pick->getHeight() + 4);
}

void ColorPickerDialog::onColorSelected(
  ColorPickerPalette & /*activator*/, int new_color)
{
  color_ = new_color;
  response(RESPONSE_OK);
}

void ColorPickerDialog::emitResponse(ResponseType response)
{
  signal_response(*this, response, color_);
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
