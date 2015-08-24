/*
 * Copyright (C) 2008 Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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
 * ColorScheme class implementation.
 *
 * @ingroup cppconsui
 */

#include "ColorScheme.h"

#include "CoreManager.h"

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include "gettext.h"

namespace CppConsUI {

int ColorScheme::getAttributes(
  int scheme, int property, int subproperty, int *out_attrs, Error &error)
{
  assert(out_attrs != NULL);

  PropertyPair property_pair(property, subproperty);
  Schemes::const_iterator i;
  Properties::const_iterator j;
  if ((i = schemes.find(scheme)) != schemes.end() &&
    (j = i->second.find(property_pair)) != i->second.end()) {
    Color c = j->second;
    if (getColorPair(c, out_attrs, error) != 0)
      return error.getCode();
    *out_attrs |= c.attrs;
#ifdef SAVE_COLOR_PAIRS
    // Save updated color in case it got inverted.
    schemes[scheme][property_pair] = c;
#endif // SAVE_COLOR_PAIRS
    return 0;
  }

  *out_attrs = 0;
  return 0;
}

#ifdef SAVE_COLOR_PAIRS
int ColorScheme::getColorPair(Color &c, int *out_attrs, Error &error)
#else
int ColorScheme::getColorPair(const Color &c, int *out_attrs, Error &error)
#endif // SAVE_COLOR_PAIRS
{
  assert(out_attrs != NULL);

  ColorPairs::const_iterator i;
  int fg = c.foreground;
  int bg = c.background;
  ColorPair color_pair(fg, bg);

  // Check if the pair already exists.
  if ((i = pairs.find(color_pair)) != pairs.end()) {
    *out_attrs = i->second;
    return 0;
  }

#ifdef SAVE_COLOR_PAIRS
  // Check if the inverse pairs exists.
  if ((i = pairs.find(color_pair)) != pairs.end()) {
    // If the inverse pair exists, use that one and flip the REVERSE bit.
    c.foreground = bg;
    c.background = fg;
    c.out_attrs ^= Curses::Attr::REVERSE;
    *out_attrs = i->second;
    return 0;
  }
#endif // SAVE_COLOR_PAIRS

  // Add a new color pair to the palette.
  if (Curses::initColorPair(pairs.size() + 1, fg, bg, out_attrs, error) != 0)
    return error.getCode();

  pairs[color_pair] = *out_attrs;
  return 0;
}

bool ColorScheme::setAttributes(int scheme, int property, int foreground,
  int background, int attrs, bool overwrite)
{
  return setAttributesExt(
    scheme, property, 0, foreground, background, attrs, overwrite);
}

bool ColorScheme::setAttributesExt(int scheme, int property, int subproperty,
  int foreground, int background, int attrs, bool overwrite)
{
  PropertyPair property_pair(property, subproperty);
  Schemes::const_iterator i;
  if (!overwrite && (i = schemes.find(scheme)) != schemes.end() &&
    i->second.find(property_pair) != i->second.end())
    return false;

  schemes[scheme][property_pair] = Color(foreground, background, attrs);
  return true;
}

void ColorScheme::freeScheme(int scheme)
{
  Schemes::const_iterator i = schemes.find(scheme);

  if (i == schemes.end())
    return;

  schemes.erase(scheme);
}

void ColorScheme::clear()
{
  schemes.clear();
  pairs.clear();
}

const char *ColorScheme::propertyToWidgetName(int property)
{
  switch (property) {
  case BUTTON_FOCUS:
  case BUTTON_NORMAL:
    return "button";
  case CHECKBOX_FOCUS:
  case CHECKBOX_NORMAL:
    return "checkbox";
  case CONTAINER_BACKGROUND:
    return "container";
  case HORIZONTALLINE_LINE:
    return "horizontalline";
  case LABEL_TEXT:
    return "label";
  case PANEL_LINE:
  case PANEL_TITLE:
    return "panel";
  case TEXTEDIT_TEXT:
    return "textedit";
  case TEXTVIEW_TEXT:
  case TEXTVIEW_SCROLLBAR:
    return "textview";
  case VERTICALLINE_LINE:
    return "verticalline";
  case TREEVIEW_LINE:
    return "treeview";
  }
  return NULL;
}

const char *ColorScheme::propertyToPropertyName(int property)
{
  switch (property) {
  case BUTTON_FOCUS:
  case CHECKBOX_FOCUS:
    return "focus";
  case BUTTON_NORMAL:
  case CHECKBOX_NORMAL:
    return "normal";
  case CONTAINER_BACKGROUND:
    return "background";
  case HORIZONTALLINE_LINE:
  case PANEL_LINE:
  case VERTICALLINE_LINE:
  case TREEVIEW_LINE:
    return "line";
  case LABEL_TEXT:
  case TEXTEDIT_TEXT:
  case TEXTVIEW_TEXT:
    return "text";
  case PANEL_TITLE:
    return "title";
  case TEXTVIEW_SCROLLBAR:
    return "scrollbar";
  }
  return NULL;
}

ColorScheme::PropertyConversionResult ColorScheme::stringPairToPropertyPair(
  const char *widget, const char *property, int *out_property,
  int *out_subproperty)
{
  assert(out_property != NULL);
  assert(out_subproperty != NULL);

  *out_subproperty = 0;

  if (strcmp(widget, "button") == 0) {
    if (strcmp(property, "focus") == 0) {
      *out_property = BUTTON_FOCUS;
      return CONVERSION_SUCCESS;
    }
    if (strcmp(property, "normal") == 0) {
      *out_property = BUTTON_NORMAL;
      return CONVERSION_SUCCESS;
    }
    return CONVERSION_ERROR_PROPERTY;
  }
  else if (strcmp(widget, "checkbox") == 0) {
    if (strcmp(property, "focus") == 0) {
      *out_property = CHECKBOX_FOCUS;
      return CONVERSION_SUCCESS;
    }
    if (strcmp(property, "normal") == 0) {
      *out_property = CHECKBOX_NORMAL;
      return CONVERSION_SUCCESS;
    }
    return CONVERSION_ERROR_PROPERTY;
  }
  else if (strcmp(widget, "container") == 0) {
    if (strcmp(property, "background") == 0) {
      *out_property = CONTAINER_BACKGROUND;
      return CONVERSION_SUCCESS;
    }
    return CONVERSION_ERROR_PROPERTY;
  }
  else if (strcmp(widget, "horizontalline") == 0) {
    if (strcmp(property, "line") == 0) {
      *out_property = HORIZONTALLINE_LINE;
      return CONVERSION_SUCCESS;
    }
    return CONVERSION_ERROR_PROPERTY;
  }
  else if (strcmp(widget, "label") == 0) {
    if (strcmp(property, "text") == 0) {
      *out_property = LABEL_TEXT;
      return CONVERSION_SUCCESS;
    }
    return CONVERSION_ERROR_PROPERTY;
  }
  else if (strcmp(widget, "panel") == 0) {
    if (strcmp(property, "line") == 0) {
      *out_property = PANEL_LINE;
      return CONVERSION_SUCCESS;
    }
    if (strcmp(property, "title") == 0) {
      *out_property = PANEL_TITLE;
      return CONVERSION_SUCCESS;
    }
    return CONVERSION_ERROR_PROPERTY;
  }
  else if (strcmp(widget, "textedit") == 0) {
    if (strcmp(property, "text") == 0) {
      *out_property = TEXTEDIT_TEXT;
      return CONVERSION_SUCCESS;
    }
    return CONVERSION_ERROR_PROPERTY;
  }
  else if (strcmp(widget, "textview") == 0) {
    if (strcmp(property, "text") == 0) {
      *out_property = TEXTVIEW_TEXT;
      return CONVERSION_SUCCESS;
    }
    if (strcmp(property, "scrollbar") == 0) {
      *out_property = TEXTVIEW_SCROLLBAR;
      return CONVERSION_SUCCESS;
    }

    // Handle text_<number> and color_<number> properties.
    if (strncmp(property, "text_", 5) == 0)
      property += 5;
    else if (strncmp(property, "color_", 6) == 0)
      property += 6;
    else
      return CONVERSION_ERROR_PROPERTY;

    if (!std::isdigit((unsigned char)*property))
      return CONVERSION_ERROR_PROPERTY;

    char *endptr;
    errno = 0;
    long i = std::strtol(property, &endptr, 10);
    assert(i >= 0);
    if (endptr != '\0' || errno == ERANGE || i > INT_MAX)
      return CONVERSION_ERROR_PROPERTY;

    *out_property = TEXTVIEW_TEXT;
    *out_subproperty = i;
    return CONVERSION_SUCCESS;
  }
  else if (strcmp(widget, "verticalline") == 0) {
    if (strcmp(property, "line") == 0) {
      *out_property = VERTICALLINE_LINE;
      return CONVERSION_SUCCESS;
    }
  }
  else if (strcmp(widget, "treeview") == 0) {
    if (strcmp(property, "line") == 0) {
      *out_property = TREEVIEW_LINE;
      return CONVERSION_SUCCESS;
    }
  }

  return CONVERSION_ERROR_WIDGET;
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
