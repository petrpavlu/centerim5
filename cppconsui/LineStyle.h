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
 * LineStyle class.
 *
 * @ingroup cppconsui
 */

#ifndef __LINESTYLE_H__
#define __LINESTYLE_H__

namespace CppConsUI
{

class LineStyle
{
public:
  enum Type {
    DEFAULT,
    ASCII,
    ASCII_ROUNDED,
    LIGHT,
    LIGHT_ROUNDED,
    HEAVY
  };

  LineStyle(Type t = DEFAULT);
  //LineStyle(const LineStyle &other);
  //LineStyle &operator=(LineStyle &other);
  virtual ~LineStyle() {}

  void SetStyle(Type t) { type = t; }
  Type GetStyle() const { return type; };

  const char *H() const;
  const char *HUp() const;
  const char *HDown() const;

  const char *V() const;
  const char *VLeft() const;
  const char *VRight() const;

  const char *Cross() const;
  const char *CornerTL() const;
  const char *CornerTR() const;
  const char *CornerBL() const;
  const char *CornerBR() const;

protected:
  struct LineElements
  {
    const char *h;         // horizontal line
    const char *h_up;      //   and line up
    const char *h_down;    //   and line down
    const char *v;         // vertical line
    const char *v_left;    //   and line left
    const char *v_right;   //   and line right
    const char *cross;     // horizontal and vertical line crossed
    const char *corner_tl; // top-left corner
    const char *corner_tr; // top-right corner
    const char *corner_bl; // bottom-left corner
    const char *corner_br; // bottom-right corner
  };

  const static LineElements line_elements_ascii;
  const static LineElements line_elements_ascii_rounded;
  const static LineElements line_elements_light;
  const static LineElements line_elements_light_rounded;
  const static LineElements line_elements_heavy;

  Type type;

  const LineElements *GetCurrentElems() const;

private:
};

} // namespace CppConsUI

#endif // __LINESTYLE_H__
