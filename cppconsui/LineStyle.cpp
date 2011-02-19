/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010 by CenterIM developers
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
 * LineStyle class implementation.
 *
 * @ingroup cppconsui
 */

#include "LineStyle.h"

struct LineElements {
  const char *h;         // Horizontal line
  const char *h_begin;   // "        " line begin
  const char *h_end;     // "        " line end
  const char *h_up;      // "             " and line up
  const char *h_down;    // "             " and line down
  const char *v;         // Vertical line
  const char *v_begin;   // "      " line begin
  const char *v_end;     // "      " line end
  const char *v_left;    // "           " and line left
  const char *v_right;   // "           " and line right
  const char *cross;     // Horizontal and Vertical line crossed
  const char *corner_tl; // Top-left corner
  const char *corner_tr; // Top-right corner
  const char *corner_bl; // Bottom-left corner
  const char *corner_br; // Bottom-right corner
};

/// @todo Add styles for double lines, block elements.
static LineElements line_elements_ascii = {
  "-", "-", "-", "+", "+",
  "|", "|", "|", "+", "+",
  "+", "+", "+", "+", "+",
};

static LineElements line_elements_ascii_rounded = {
  "-", "-", "-", "-", "-",
  "|", "|", "|", "|", "|",
  "+", "/", "\\", "\\", "/",
};

static LineElements line_elements_light = {
  "\342\224\200", "\342\225\266", "\342\225\264", "\342\224\264", "\342\224\254",
  "\342\224\202", "\342\225\267", "\342\225\265", "\342\224\244", "\342\224\234",
  "\342\224\274", "\342\224\214", "\342\224\220", "\342\224\224", "\342\224\230",
};

static LineElements line_elements_light_rounded = {
  "\342\224\200", "\342\225\266", "\342\225\264", "\342\224\264", "\342\224\254",
  "\342\224\202", "\342\225\267", "\342\225\265", "\342\224\244", "\342\224\234",
  "\342\224\274", "\342\225\255", "\342\225\256", "\342\225\257", "\342\225\260",
};

static LineElements line_elements_heavy = {
  "\342\224\201", "\342\225\272", "\342\225\270", "\342\224\273", "\342\224\263",
  "\342\224\203", "\342\225\273", "\342\225\271", "\342\224\253", "\342\224\243",
  "\342\225\213", "\342\224\217", "\342\224\223", "\342\224\227", "\342\224\233",
};

LineStyle::LineStyle(Type t)
: type(t)
{
  utf8 = g_get_charset(NULL);
}

void LineStyle::SetStyle(Type t)
{
  type = t;
}

LineStyle::Type LineStyle::GetStyle() const
{
  return type;
}

const char *LineStyle::H() const
{
  return GetCurrentElems()->h;
}

const char *LineStyle::HBegin() const
{
  return GetCurrentElems()->h_begin;
}

const char *LineStyle::HEnd() const
{
  return GetCurrentElems()->h_end;
}

const char *LineStyle::HUp() const
{
  return GetCurrentElems()->h_up;
}

const char *LineStyle::HDown() const
{
  return GetCurrentElems()->h_down;
}

const char *LineStyle::V() const
{
  return GetCurrentElems()->v;
}

const char *LineStyle::VBegin() const
{
  return GetCurrentElems()->v_begin;
}

const char *LineStyle::VEnd() const
{
  return GetCurrentElems()->v_end;
}

const char *LineStyle::VLeft() const
{
  return GetCurrentElems()->v_left;
}

const char *LineStyle::VRight() const
{
  return GetCurrentElems()->v_right;
}

const char *LineStyle::Cross() const
{
  return GetCurrentElems()->cross;
}

const char *LineStyle::CornerTL() const
{
  return GetCurrentElems()->corner_tl;
}

const char *LineStyle::CornerTR() const
{
  return GetCurrentElems()->corner_tr;
}

const char *LineStyle::CornerBL() const
{
  return GetCurrentElems()->corner_bl;
}

const char *LineStyle::CornerBR() const
{
  return GetCurrentElems()->corner_br;
}

LineElements *LineStyle::GetCurrentElems() const
{
  switch (type) {
    case ASCII:
      return &line_elements_ascii;
    case ASCII_ROUNDED:
      return &line_elements_ascii_rounded;
    case LIGHT:
      return &line_elements_light;
    case LIGHT_ROUNDED:
      return &line_elements_light_rounded;
    case HEAVY:
      return &line_elements_heavy;
    case DEFAULT:
      if (utf8)
        return &line_elements_light;
      else
        return &line_elements_ascii;
  }

  g_assert_not_reached();
  return NULL;
}
