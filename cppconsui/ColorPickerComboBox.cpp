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
/// ColorPickerComboBox class implementation.
///
/// @ingroup cppconsui

#include "ColorPickerComboBox.h"

#include <algorithm>
#include "gettext.h"

// Invalid color number that is used for the "More..." button in
// ColorPickerComboBox.
#define COLOR_MORE (-2)

// Width of the drop-down menu.
#define MENU_WIDTH 12

namespace CppConsUI {

ColorPickerComboBox::ColorPickerComboBox(int w, int color)
  : ComboBox(w, 1), selected_color_(color)
#ifdef COLORPICKER_256COLOR
    ,
    colorpicker_(NULL)
#endif // COLORPICKER_256COLOR
{
  // Add ANSI colors.
  int colors = std::min(Curses::NUM_DEFAULT_COLORS, Curses::getColorCount());
  for (int i = 0; i < colors; ++i)
    addOption(nullptr, i);

  // Add options for default color and to open the 256 color dialog.
  addOption(nullptr, Curses::Color::DEFAULT);
#ifdef COLORPICKER_256COLOR
  addOption(_("More..."), COLOR_MORE);
#endif // COLORPICKER_256COLOR

  // Set initial selection.
  setSelectedByData(color);
}

ColorPickerComboBox::~ColorPickerComboBox()
{
#ifdef COLORPICKER_256COLOR
  if (colorpicker_ != NULL)
    colorpicker_->close();
#endif // COLORPICKER_256COLOR
}

void ColorPickerComboBox::setColor(int new_color)
{
  if (new_color < Curses::Color::DEFAULT ||
    new_color >= Curses::getColorCount()) {
    // an invalid color was specified, use the default color
    new_color = Curses::Color::DEFAULT;
  }

  if (new_color == selected_color_)
    return;

  selected_color_ = new_color;

#ifdef COLORPICKER_256COLOR
  if (selected_color_ >= Curses::Color::DEFAULT &&
    selected_color_ < Curses::NUM_DEFAULT_COLORS)
    setSelectedByData(selected_color_);
  else
    setSelectedByData(COLOR_MORE);
#else
  setSelectedByData(selected_color_);
#endif // COLORPICKER_256COLOR
}

int ColorPickerComboBox::draw(Curses::ViewPort area, Error &error)
{
  int attrs;
  if (has_focus_) {
    DRAW(getAttributes(ColorScheme::PROPERTY_BUTTON_FOCUS, &attrs, error));
    attrs |= Curses::Attr::REVERSE;
  }
  else
    DRAW(getAttributes(ColorScheme::PROPERTY_BUTTON_NORMAL, &attrs, error));

  DRAW(area.attrOn(attrs, error));
  DRAW(area.fill(attrs, 0, 0, real_width_, 1, error));
  DRAW(area.addChar(0, 0, '[', error));
  DRAW(area.addChar(real_width_ - 1, 0, ']', error));
  DRAW(area.attrOff(attrs, error));

  if (selected_color_ == Curses::Color::DEFAULT)
    DRAW(area.addString(1, 0, _("DEFAULT"), error));
  else {
    ColorScheme::Color c(Curses::Color::DEFAULT, selected_color_);
    int colorpair;
    DRAW(COLORSCHEME->getColorPair(c, &colorpair, error));
    DRAW(area.attrOn(colorpair, error));
    DRAW(area.fill(colorpair, 1, 0, real_width_ - 2, 1, error));
    DRAW(area.attrOff(colorpair, error));
  }

  return 0;
}

void ColorPickerComboBox::onDropDown(Button & /*activator*/)
{
  dropdown_ = new MenuWindow(*this, MENU_WIDTH, AUTOSIZE);
  dropdown_->signal_close.connect(
    sigc::mem_fun(this, &ColorPickerComboBox::dropDownClose));

  int i;
  ComboBoxEntries::iterator j;
  for (i = 0, j = options_.begin(); j != options_.end(); ++i, ++j) {
    Button *b;
    if (j->data == COLOR_MORE) {
      // Add the "More..." button.
      b = dropdown_->appendItem(j->title,
        sigc::bind(sigc::mem_fun(this, &ColorPickerComboBox::dropDownOk), i));
    }
    else {
      // Normal color button.
      b = new ColorButton(j->data);
      dropdown_->appendWidget(*b);
      b->signal_activate.connect(
        sigc::bind(sigc::mem_fun(this, &ColorPickerComboBox::dropDownOk), i));
    }
    if (i == selected_entry_)
      b->grabFocus();
  }

  dropdown_->show();
}

void ColorPickerComboBox::dropDownOk(Button & /*activator*/, int new_entry)
{
  dropdown_->close();

#ifdef COLORPICKER_256COLOR
  if (options_[new_entry].data != COLOR_MORE) {
    setColor(options_[new_entry].data);
    return;
  }

  // Rhe "More..." button was selected, display the color picker dialog.
  // @todo Initialize the color picker dialog with a selected color.
  colorpicker_ = new ColorPickerDialog("", 0, 0);
  colorpicker_->signal_response.connect(
    sigc::mem_fun(this, &ColorPickerComboBox::colorPickerOk));
  colorpicker_->signal_close.connect(
    sigc::mem_fun(this, &ColorPickerComboBox::colorPickerClose));
  colorpicker_->show();
#else
  setColor(options_[new_entry].data);
#endif // COLORPICKER_256COLOR
}

#ifdef COLORPICKER_256COLOR
void ColorPickerComboBox::colorPickerOk(ColorPickerDialog &activator,
  AbstractDialog::ResponseType response, int new_color)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  // Selected option did not change.
  if (new_color == selected_color)
    return;

  setColor(new_color);
}

void ColorPickerComboBox::colorPickerClose(Window & /*window*/)
{
  colorpicker_ = NULL;
}
#endif // COLORPICKER_256COLOR

void ColorPickerComboBox::setSelected(int new_entry)
{
  ComboBox::setSelected(new_entry);

  selected_color_ = options_[new_entry].data;
  signal_color_changed(*this, selected_color_);
}

ColorPickerComboBox::ColorButton::ColorButton(int color)
  : Button(MENU_WIDTH - 2, 1, ""), color_(color)
{
}

int ColorPickerComboBox::ColorButton::draw(Curses::ViewPort area, Error &error)
{
  int attrs;
  if (has_focus_) {
    DRAW(getAttributes(ColorScheme::PROPERTY_BUTTON_FOCUS, &attrs, error));
    attrs |= Curses::Attr::REVERSE;
  }
  else
    DRAW(getAttributes(ColorScheme::PROPERTY_BUTTON_NORMAL, &attrs, error));

  DRAW(area.attrOn(attrs, error));
  DRAW(area.fill(attrs, 0, 0, real_width_, 1, error));
  DRAW(area.addChar(0, 0, '[', error));
  DRAW(area.addChar(real_width_ - 1, 0, ']', error));
  DRAW(area.attrOff(attrs, error));

  if (color_ == Curses::Color::DEFAULT)
    DRAW(area.addString(1, 0, _("DEFAULT "), error));
  else {
    ColorScheme::Color c(Curses::Color::DEFAULT, color_);
    int colorpair;
    DRAW(COLORSCHEME->getColorPair(c, &colorpair, error));
    DRAW(area.attrOn(colorpair, error));
    DRAW(area.fill(colorpair, 1, 0, real_width_ - 2, 1, error));
    DRAW(area.attrOff(colorpair, error));
  }

  return 0;
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
