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
 * ColorPickerDialog class implementation.
 *
 * @ingroup cppconsui
 */

#include "ColorPickerDialog.h"

#include "gettext.h"

namespace CppConsUI
{

ColorPickerDialog::ColorPickerDialog(const char *title, const int defaultvalue,
    int flags)
: AbstractDialog(title)
{
  AddButton(OK_BUTTON_TEXT, RESPONSE_OK);

  pick = new ColorPickerPalette(0, flags);

  pick->signal_color_selected.connect(sigc::mem_fun(this,
        &ColorPickerDialog::OnColorSelected));

  layout->InsertWidget(0, *pick);

  Resize (pick->GetWidth() + 2, pick->GetHeight() + 4);
}

void ColorPickerDialog::OnColorSelected (ColorPickerPalette& activator, int color_)
{
  color = color_;
  Response(RESPONSE_OK);
}

void ColorPickerDialog::EmitResponse(ResponseType response)
{
  signal_response(*this, response, color);
}


} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
