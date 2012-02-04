/*
 * Copyright (C) 2008 by Mark Pustjens <pustjens@dds.nl>
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
 * ColorPickerDialog class.
 *
 * @ingroup cppconsui
 */

#ifndef __COLORPICKERDIALOG_H__
#define __COLORPICKERDIALOG_H__

#include "AbstractDialog.h"
#include "Button.h"
#include "ColorScheme.h"
#include "ColorPickerPalette.h"

#include <vector>

namespace CppConsUI
{

class ColorPickerDialog
: public AbstractDialog
{
  public:
    ColorPickerDialog(const char *title, int defaultvalue,
        int flags);
    virtual ~ColorPickerDialog() {}

    virtual void OnColorSelected (ColorPickerPalette& activator, int color);

    /**
     * Signal emitted when the user closes the dialog.
     */
    sigc::signal<void, ColorPickerDialog&, ResponseType, int> signal_response;

  protected:

    // AbstractDialog
    virtual void EmitResponse(ResponseType response);

    ColorPickerPalette *pick;
    int color;

  private:
    ColorPickerDialog(const ColorPickerDialog&);
    ColorPickerDialog& operator=(const ColorPickerDialog&);
};

} // namespace CppConsUI

#endif // __COLORPICKERDIALOG_H__

/* vim: set tabstop=2 shiftwidth=2 tw=78 expandtab : */
