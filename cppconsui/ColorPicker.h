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
/// ColorPicker class.
///
/// @ingroup cppconsui

#ifndef COLORPICKER_H
#define COLORPICKER_H

#include "ColorPickerComboBox.h"
#include "HorizontalListBox.h"
#include "Label.h"

namespace CppConsUI {

class ColorPicker : public HorizontalListBox {
public:
  ColorPicker(int fg, int bg, const char *text, bool sample = false);
  virtual ~ColorPicker() override {}

  virtual void setColorPair(int fg, int bg);
  virtual void setText(const char *new_text);

  /// Signal emitted when a colorpair has been selected.
  sigc::signal<void, ColorPicker &, int, int> signal_colorpair_selected;

protected:
  class Sample : public Widget {
  public:
    Sample(int w, int fg = -1, int bg = -1);
    virtual ~Sample() override {}

    // Widget
    virtual int draw(Curses::ViewPort area, Error &error) override;

    virtual void setColors(int fg, int bg);

  protected:
    ColorScheme::Color color_;

  private:
    CONSUI_DISABLE_COPY(Sample);
  };

  ColorPickerComboBox *fg_combo_;
  ColorPickerComboBox *bg_combo_;
  Label *label_;
  Sample *sample_;

  virtual void onColorChanged(ComboBox &activator, int new_color);

private:
  CONSUI_DISABLE_COPY(ColorPicker);
};

} // namespace CppConsUI

#endif // COLORPICKER_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
