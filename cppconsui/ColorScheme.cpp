/*
 * Copyright (C) 2008 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2012 by CenterIM developers
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

/* Uncomment to enable an experimental feature to lower the number of used
 * colorpairs */
//#define SAVE_COLOR_PAIRS

#include "ColorScheme.h"

#include "gettext.h"

namespace CppConsUI
{

ColorScheme *ColorScheme::Instance()
{
  static ColorScheme instance;
  return &instance;
}

int ColorScheme::GetColorPair(const char *scheme, const char *widget,
    const char *property)
{
  g_assert(widget);
  g_assert(property);

  Schemes::const_iterator i;
  Widgets::const_iterator j;
  Properties::const_iterator k;
  if (scheme && (i = schemes.find(scheme)) != schemes.end()
      && (j = i->second.find(widget)) != i->second.end()
      && (k = j->second.find(property)) != j->second.end()) {
    Color c = k->second;
    int ret = GetColorPair(c) | c.attrs;
    schemes[scheme][widget][property] = c;
    return ret;
  }

  return 0;
}

int ColorScheme::GetColorPair(Color& c)
{
  ColorPairs::const_iterator i;
  int fg, bg, res;

  fg = c.foreground;
  bg = c.background;

  // Check if the pair already exists
  if ((i = pairs.find(std::make_pair(fg, bg))) != pairs.end())
    return i->second;

#ifdef SAVE_COLOR_PAIRS
  // Check if the inverse pairs exists
  if ((i = pairs.find(std::make_pair(bg, fg))) != pairs.end()) {
    // If the inverse pair exists, use that one and flip the REVERSE bit.
    c.foreground = bg;
    c.background = fg;
    c.attrs ^= Curses::Attr::REVERSE;
    return i->second;
  }
#endif // SAVE_COLOR_PAIRS

  // No existing pair we can use. Check if we can add a new one to the palette.
  if ((int) pairs.size() >= Curses::nrcolorpairs()) {
    g_warning(_("Color pairs limit exceeded."));
    return 0;
  }

  // Add a new colorpair to the palette
  if (!Curses::init_colorpair(pairs.size() + 1, fg, bg, &res)) {
    g_warning(_("Adding colorpair failed."));
    return 0;
  }
  pairs[std::make_pair(fg, bg)] = res;
  return res;
}

bool ColorScheme::SetColorPair(const char *scheme, const char *widget,
    const char *property, int foreground, int background, int attrs,
    bool overwrite)
{
  g_assert(widget);
  g_assert(property);

  Schemes::const_iterator i;
  Widgets::const_iterator j;
  Properties::const_iterator k;
  if (!overwrite && scheme && (i = schemes.find(scheme)) != schemes.end()
      && (j = i->second.find(widget)) != i->second.end()
      && (k = j->second.find(property)) != j->second.end())
    return false;

  schemes[scheme][widget][property] = Color(foreground, background, attrs);
  return true;
}

void ColorScheme::FreeScheme(const char *scheme)
{
  Schemes::const_iterator i;

  g_assert(scheme);

  i = schemes.find(scheme);

  if (i == schemes.end())
    return;

  schemes.erase(scheme);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
