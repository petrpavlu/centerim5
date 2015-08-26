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
/// ColorPicker class implementation.
///
/// @ingroup cppconsui

#include "ColorPicker.h"

#include "ColorScheme.h"
#include "Spacer.h"

#include "gettext.h"

namespace CppConsUI {

ColorPicker::ColorPicker(int fg, int bg, const char *text, bool sample)
  : HorizontalListBox(AUTOSIZE, 1), fg_combo_(NULL), bg_combo_(NULL),
    label_(NULL), sample_(NULL)
{
  fg_combo_ = new ColorPickerComboBox(10, fg);
  bg_combo_ = new ColorPickerComboBox(10, bg);

  label_ = new Label(1, 1, "");
  setText(text);

  fg_combo_->signal_color_changed.connect(
    sigc::mem_fun(this, &ColorPicker::onColorChanged));
  bg_combo_->signal_color_changed.connect(
    sigc::mem_fun(this, &ColorPicker::onColorChanged));

  appendWidget(*label_);
  appendWidget(*fg_combo_);
  appendWidget(*(new Spacer(1, 1)));
  appendWidget(*bg_combo_);

  if (sample) {
    sample_ = new Sample(10, fg, bg);
    appendWidget(*sample_);
  }

  setColorPair(fg, bg);
}

void ColorPicker::setColorPair(int fg, int bg)
{
  fg_combo_->setColor(fg);
  bg_combo_->setColor(bg);

  if (sample_ != NULL)
    sample_->setColors(fg, bg);

  signal_colorpair_selected(*this, fg, bg);
}

void ColorPicker::setText(const char *new_text)
{
  label_->setText(new_text);
  if (new_text != NULL)
    label_->setWidth(Curses::onScreenWidth(new_text) + 1);
  else
    label_->setWidth(0);
}

void ColorPicker::onColorChanged(ComboBox &activator, int new_color)
{
  int new_fg = fg_combo_->getColor();
  int new_bg = bg_combo_->getColor();

  if (&activator == fg_combo_)
    new_fg = new_color;
  else
    new_bg = new_color;

  setColorPair(new_fg, new_bg);
}

ColorPicker::Sample::Sample(int w, int fg, int bg)
  : Widget(w, 1), color_(fg, bg)
{
}

int ColorPicker::Sample::draw(Curses::ViewPort area, Error &error)
{
  int attrs;
  if (COLORSCHEME->getColorPair(color_, &attrs, error) != 0)
    return error.getCode();

  DRAW(area.attrOn(attrs, error));
  DRAW(area.addString(1, 0, _(" SAMPLE "), error));
  DRAW(area.attrOff(attrs, error));

  return 0;
}

void ColorPicker::Sample::setColors(int fg, int bg)
{
  color_.foreground = fg;
  color_.background = bg;
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
