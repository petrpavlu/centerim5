/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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
 * LineStyle class implementation.
 *
 * @ingroup cppconsui
 */

#include "LineStyle.h"

#include "CoreManager.h"

namespace CppConsUI
{

const LineStyle::LineElements LineStyle::line_elements_ascii = {
  "-", "+", "+",
  "|", "+", "+",
  "+", "+", "+", "+", "+",
};

const LineStyle::LineElements LineStyle::line_elements_ascii_rounded = {
  "-", "-", "-",
  "|", "|", "|",
  "+", "/", "\\", "\\", "/",
};

const LineStyle::LineElements LineStyle::line_elements_light = {
  "\342\224\200", "\342\224\264", "\342\224\254",
  "\342\224\202", "\342\224\244", "\342\224\234",
  "\342\224\274", "\342\224\214", "\342\224\220", "\342\224\224", "\342\224\230",
};

const LineStyle::LineElements LineStyle::line_elements_light_rounded = {
  "\342\224\200", "\342\224\264", "\342\224\254",
  "\342\224\202", "\342\224\244", "\342\224\234",
  "\342\224\274", "\342\225\255", "\342\225\256", "\342\225\257", "\342\225\260",
};

const LineStyle::LineElements LineStyle::line_elements_heavy = {
  "\342\224\201", "\342\224\273", "\342\224\263",
  "\342\224\203", "\342\224\253", "\342\224\243",
  "\342\225\213", "\342\224\217", "\342\224\223", "\342\224\227", "\342\224\233",
};

LineStyle::LineStyle(Type t)
: type(t)
{
}

const char *LineStyle::H() const
{
  return GetCurrentElems()->h;
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

const LineStyle::LineElements *LineStyle::GetCurrentElems() const
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
      if (COREMANAGER->IsFallbackDrawMode())
        return &line_elements_ascii;
      return &line_elements_light;
  }

  g_assert_not_reached();
  return NULL;
}

} // namespace CppConsUI
