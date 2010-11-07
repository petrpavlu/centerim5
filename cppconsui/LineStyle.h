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
 * LineStyle class.
 *
 * @ingroup cppconsui
 */

#ifndef __LINESTYLE_H__
#define __LINESTYLE_H__

#include <glib.h>

struct LineElements;

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

  void SetStyle(Type t);
  Type GetStyle() const;

  const gchar *H() const;
  const gchar *HBegin() const;
  const gchar *HEnd() const;
  const gchar *HUp() const;
  const gchar *HDown() const;
  const gchar *V() const;
  const gchar *VBegin() const;
  const gchar *VEnd() const;
  const gchar *VLeft() const;
  const gchar *VRight() const;
  const gchar *Cross() const;
  const gchar *CornerTL() const;
  const gchar *CornerTR() const;
  const gchar *CornerBL() const;
  const gchar *CornerBR() const;

protected:
  Type type;

private:
  bool utf8;
  LineElements *GetCurrentElems() const;
};

#endif // __LINESTYLE_H__
