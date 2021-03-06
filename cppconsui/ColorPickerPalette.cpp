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
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// ColorPickerPalette class implementation.
///
/// @ingroup cppconsui

#include "ColorPickerPalette.h"

#include "ColorScheme.h"

#include <algorithm>

#define GRAYSCALE_START 232
#define GRAYSCALE_END 255

namespace CppConsUI {

ColorPickerPalette::ColorPickerPalette(int default_color, int flags)
  : Container(0, 0)
{
  if (flags == (FLAG_HIDE_ANSI | FLAG_HIDE_GRAYSCALE | FLAG_HIDE_COLORCUBE)) {
    // Show at least ANSI colors.
    flags = FLAG_HIDE_GRAYSCALE | FLAG_HIDE_COLORCUBE;
  }

  if (Curses::getColorCount() < 256)
    flags |= (FLAG_HIDE_GRAYSCALE | FLAG_HIDE_COLORCUBE);

  if (!(flags & FLAG_HIDE_ANSI)) {
    // Default 16 colors.
    addAnsi(default_color);
  }

  if (!(flags & FLAG_HIDE_GRAYSCALE)) {
    // Grayscale ladder.
    addGrayscale(default_color);
  }

  if (!(flags & FLAG_HIDE_COLORCUBE)) {
    // 6x6x6 color cube.
    addColorCube(default_color);
  }
}

void ColorPickerPalette::onSelectColor(Button &activator)
{
  ColorPickerPaletteButton *button =
    dynamic_cast<ColorPickerPaletteButton *>(&activator);
  assert(button != nullptr);

  signal_color_selected(*this, button->getColor());
}

void ColorPickerPalette::addButton(int x, int y, int color, int default_color)
{
  auto button = new ColorPickerPaletteButton(color);
  button->signal_activate.connect(
    sigc::mem_fun(this, &ColorPickerPalette::onSelectColor));
  addWidget(*button, x, y);

  if (color == default_color)
    button->grabFocus();
}

void ColorPickerPalette::addAnsi(int default_color)
{
  int w, h, x, y;

  // Resize the ColorPickerPalette.
  w = getWidth();
  h = y = getHeight();

  // There are 8 ANSI colors, or 16 when bright colors are supported.
  w = std::max(w, Curses::NUM_DEFAULT_COLORS);
  h += 2;

  resize(w, h);

  // Add the color picker buttons, in two lines.
  // @todo Support terms with only 8 colors.
  int half = Curses::NUM_DEFAULT_COLORS / 2;
  for (x = 0; x < Curses::NUM_DEFAULT_COLORS; ++x)
    if (x < half) {
      // The first line.
      addButton(x * 2, y, x, default_color);
    }
    else {
      // The second line.
      addButton((x - half) * 2, y + 1, x, default_color);
    }
}

void ColorPickerPalette::addGrayscale(int default_color)
{
  int w, h, x, y, color;

  // Resize the ColorPickerPalette.
  w = getWidth();
  h = getHeight();

  // Add space between this and previous section.
  if (h != 0)
    ++h;

  y = h;
  w = std::max(w, (GRAYSCALE_END - GRAYSCALE_START + 1) * 2);
  h = h + 1;

  resize(w, h);

  // Add the color picker buttons.
  for (color = GRAYSCALE_START, x = 0; color <= GRAYSCALE_END; ++color, x += 2)
    addButton(x, y, color, default_color);

  addButton(x, y, Curses::Color::WHITE, default_color);
}

void ColorPickerPalette::addColorCube(int default_color)
{
  int w, h, x, y;

  // Resize the ColorPickerPalette.
  w = getWidth();
  h = getHeight();

  // Add space between this and previous section.
  if (h != 0)
    ++h;

  y = h;

  w = std::max(w, (6 * 6 * 2) + 5);
  h = h + 6;

  resize(w, h);

  // Add the color picker buttons.
  x = 0;
  for (int g = 0; g < 6; ++g) {
    for (int r = 0; r < 6; ++r) {
      for (int b = 0; b < 6; ++b) {
        addButton(x, y, 16 + (r * 36) + (g * 6) + b, default_color);
        x += 2;
      }

      ++x;
    }

    ++y;
    x = 0;
  }
}

ColorPickerPalette::ColorPickerPaletteButton::ColorPickerPaletteButton(
  int color)
  : Button(2, 1, ""), color_(color)
{
}

int ColorPickerPalette::ColorPickerPaletteButton::draw(
  Curses::ViewPort area, Error &error)
{
  ColorScheme::Color c(Curses::Color::BLACK, color_);
  int attrs;
  if (COLORSCHEME->getColorPair(c, &attrs, error) != 0)
    return error.getCode();

  if (has_focus_) {
    DRAW(area.attrOn(Curses::Attr::REVERSE, error));
    DRAW(area.addString(0, 0, "@@", error));
    DRAW(area.attrOff(Curses::Attr::REVERSE, error));
  }
  else
    DRAW(area.fill(attrs, 0, 0, 2, 1, error));

  return 0;
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
