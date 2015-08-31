// Copyright (C) 2008 Mark Pustjens <pustjens@dds.nl>
// Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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
/// ColorScheme class.
///
/// @ingroup cppconsui

#ifndef COLORSCHEME_H
#define COLORSCHEME_H

#include "ConsUICurses.h"
#include "CppConsUI.h" // for COLORSCHEME macro

#include <map>
#include <string>

// Uncomment to enable an experimental feature to lower the number of used
// colorpairs.
//#define SAVE_COLOR_PAIRS

namespace CppConsUI {

class ColorScheme {
public:
  struct Color {
    int foreground;
    int background;
    int attrs;

    Color(int f = Curses::Color::DEFAULT, int b = Curses::Color::DEFAULT,
      int a = Curses::Attr::NORMAL)
      : foreground(f), background(b), attrs(a)
    {
    }
  };

  enum PropertyConversionResult {
    CONVERSION_SUCCESS,
    CONVERSION_ERROR_WIDGET,
    CONVERSION_ERROR_PROPERTY,
  };

  enum Property {
    PROPERTY_BUTTON_FOCUS,
    PROPERTY_BUTTON_NORMAL,
    PROPERTY_CHECKBOX_FOCUS,
    PROPERTY_CHECKBOX_NORMAL,
    PROPERTY_CONTAINER_BACKGROUND,
    PROPERTY_HORIZONTALLINE_LINE,
    PROPERTY_LABEL_TEXT,
    PROPERTY_PANEL_LINE,
    PROPERTY_PANEL_TITLE,
    PROPERTY_TEXTEDIT_TEXT,
    PROPERTY_TEXTVIEW_TEXT,
    PROPERTY_TEXTVIEW_SCROLLBAR,
    PROPERTY_VERTICALLINE_LINE,
    PROPERTY_TREEVIEW_LINE,
  };

  typedef std::pair<int, int> PropertyPair;
  typedef std::map<PropertyPair, Color> Properties;
  typedef std::map<int, Properties> Schemes;

  /// Gets color pair and Curses attributes (that can be passed to
  /// Curses::ViewPort::attrOn()) for a given scheme, widget and property
  /// combination.
  int getAttributes(
    int scheme, int property, int subproperty, int *out_attrs, Error &error);
#ifdef SAVE_COLOR_PAIRS
  int getColorPair(Color &c, int *attrs, Error &error);
#else
  int getColorPair(const Color &c, int *attrs, Error &error);
#endif

  /// Sets color pair and Curses attributes for a given scheme, widget, property
  /// combination.
  bool setAttributes(int scheme, int property, int foreground, int background,
    int attrs = Curses::Attr::NORMAL, bool overwrite = false);
  bool setAttributesExt(int scheme, int property, int subproperty,
    int foreground, int background, int attrs = Curses::Attr::NORMAL,
    bool overwrite = false);

  void freeScheme(int scheme);
  const Schemes &getSchemes() const { return schemes_; }

  void clear();

  static const char *propertyToWidgetName(int property);
  static const char *propertyToPropertyName(int property);
  static PropertyConversionResult stringPairToPropertyPair(const char *widget,
    const char *property, int *out_property, int *out_subproperty);

private:
  typedef std::pair<int, int> ColorPair;
  typedef std::map<ColorPair, int> ColorPairs;

  Schemes schemes_;
  ColorPairs pairs_;

  ColorScheme() {}
  ~ColorScheme() {}
  CONSUI_DISABLE_COPY(ColorScheme);

  friend void initializeConsUI(AppInterface &interface);
  friend void finalizeConsUI();
};

} // namespace CppConsUI

#endif // COLORSCHEME_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
