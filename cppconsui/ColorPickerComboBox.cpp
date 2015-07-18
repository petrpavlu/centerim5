/*
 * Copyright (C) 2012-2015 by CenterIM developers
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
 * ColorPickerComboBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "ColorPickerComboBox.h"

#include <algorithm>
#include "gettext.h"

/* This is an invalid color number that is used for the "More..." button in
 * ColorPickerComboBox. */
#define COLOR_MORE (-2)

// width of the drop-down menu
#define MENU_WIDTH 12

namespace CppConsUI {

ColorPickerComboBox::ColorPickerComboBox(int w, int color)
  : ComboBox(w, 1), selected_color(color)
#ifdef COLORPICKER_256COLOR
    ,
    colorpicker(NULL)
#endif // COLORPICKER_256COLOR
{
  // add ANSI colors
  int colors = std::min(Curses::NUM_DEFAULT_COLORS, Curses::getColorCount());
  for (int i = 0; i < colors; i++)
    addOption(NULL, i);

  // add options for default color and to open the 256 color dialog
  addOption(NULL, Curses::Color::DEFAULT);
#ifdef COLORPICKER_256COLOR
  addOption(_("More..."), COLOR_MORE);
#endif // COLORPICKER_256COLOR

  // set initial selection
  setSelectedByData(color);
}

ColorPickerComboBox::~ColorPickerComboBox()
{
#ifdef COLORPICKER_256COLOR
  if (colorpicker)
    colorpicker->close();
#endif // COLORPICKER_256COLOR
}

void ColorPickerComboBox::setColor(int new_color)
{
  if (new_color < Curses::Color::DEFAULT ||
    new_color >= Curses::getColorCount()) {
    // an invalid color was specified, use the default color
    new_color = Curses::Color::DEFAULT;
  }

  if (new_color == selected_color)
    return;

  selected_color = new_color;

#ifdef COLORPICKER_256COLOR
  if (selected_color >= Curses::Color::DEFAULT &&
    selected_color < Curses::NUM_DEFAULT_COLORS)
    setSelectedByData(selected_color);
  else
    setSelectedByData(COLOR_MORE);
#else
  setSelectedByData(selected_color);
#endif // COLORPICKER_256COLOR
}

void ColorPickerComboBox::draw(Curses::ViewPort area)
{
  int button_colorpair;
  if (has_focus)
    button_colorpair = getColorPair("button", "focus") | Curses::Attr::REVERSE;
  else
    button_colorpair = getColorPair("button", "normal");

  int color = selected_color;

  area.attrOn(button_colorpair);
  area.fill(button_colorpair, 0, 0, real_width, 1);
  area.addChar(0, 0, '[');
  area.addChar(real_width - 1, 0, ']');
  area.attrOff(button_colorpair);

  if (selected_color == Curses::Color::DEFAULT)
    area.addString(1, 0, _("DEFAULT"));
  else {
    ColorScheme::Color c(Curses::Color::DEFAULT, color);
    int colorpair = COLORSCHEME->getColorPair(c);
    area.attrOn(colorpair);
    area.fill(colorpair, 1, 0, real_width - 2, 1);
    area.attrOff(colorpair);
  }
}

void ColorPickerComboBox::onDropDown(Button & /*activator*/)
{
  dropdown = new MenuWindow(*this, MENU_WIDTH, AUTOSIZE);
  dropdown->signal_close.connect(
    sigc::mem_fun(this, &ColorPickerComboBox::dropDownClose));

  int i;
  ComboBoxEntries::iterator j;
  for (i = 0, j = options.begin(); j != options.end(); i++, j++) {
    Button *b;
    if (j->data == COLOR_MORE) {
      // add the "More..." button
      b = dropdown->appendItem(j->title,
        sigc::bind(sigc::mem_fun(this, &ColorPickerComboBox::dropDownOk), i));
    }
    else {
      // normal color button
      b = new ColorButton(j->data);
      dropdown->appendWidget(*b);
      b->signal_activate.connect(
        sigc::bind(sigc::mem_fun(this, &ColorPickerComboBox::dropDownOk), i));
    }
    if (i == selected_entry)
      b->grabFocus();
  }

  dropdown->show();
}

void ColorPickerComboBox::dropDownOk(Button & /*activator*/, int new_entry)
{
  dropdown->close();

#ifdef COLORPICKER_256COLOR
  if (options[new_entry].data != COLOR_MORE) {
    setColor(options[new_entry].data);
    return;
  }

  // the "More..." button was selected, display the color picker dialog
  // @todo Initialize the color picker dialog with a selected color.
  colorpicker = new ColorPickerDialog("", 0, 0);
  colorpicker->signal_response.connect(
    sigc::mem_fun(this, &ColorPickerComboBox::colorPickerOk));
  colorpicker->signal_close.connect(
    sigc::mem_fun(this, &ColorPickerComboBox::colorPickerClose));
  colorpicker->show();
#else
  setColor(options[new_entry].data);
#endif // COLORPICKER_256COLOR
}

#ifdef COLORPICKER_256COLOR
void ColorPickerComboBox::colorPickerOk(ColorPickerDialog &activator,
  AbstractDialog::ResponseType response, int new_color)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  // selected option didn't change
  if (new_color == selected_color)
    return;

  setColor(new_color);
}

void ColorPickerComboBox::colorPickerClose(Window & /*window*/)
{
  colorpicker = NULL;
}
#endif // COLORPICKER_256COLOR

void ColorPickerComboBox::setSelected(int new_entry)
{
  ComboBox::setSelected(new_entry);

  selected_color = options[new_entry].data;
  signal_color_changed(*this, selected_color);
}

ColorPickerComboBox::ColorButton::ColorButton(int color_)
  : Button(MENU_WIDTH - 2, 1, ""), color(color_)
{
}

void ColorPickerComboBox::ColorButton::draw(Curses::ViewPort area)
{
  int button_colorpair;
  if (has_focus)
    button_colorpair = getColorPair("button", "focus") | Curses::Attr::REVERSE;
  else
    button_colorpair = getColorPair("button", "normal");

  area.attrOn(button_colorpair);
  area.fill(button_colorpair, 0, 0, real_width, 1);
  area.addChar(0, 0, '[');
  area.addChar(real_width - 1, 0, ']');
  area.attrOff(button_colorpair);

  if (color == Curses::Color::DEFAULT)
    area.addString(1, 0, _("DEFAULT "));
  else {
    ColorScheme::Color c(Curses::Color::DEFAULT, color);
    int colorpair = COLORSCHEME->getColorPair(c);
    area.attrOn(colorpair);
    area.fill(colorpair, 1, 0, real_width - 2, 1);
    area.attrOff(colorpair);
  }
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
