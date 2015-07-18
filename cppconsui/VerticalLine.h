/*
 * Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
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
 * VerticalLine class.
 *
 * @ingroup cppconsui
 */

#ifndef __VERTICALLINE_H__
#define __VERTICALLINE_H__

#include "AbstractLine.h"

namespace CppConsUI {

/**
 * A widget representing a vertical line.
 */
class VerticalLine : public AbstractLine {
public:
  VerticalLine(int h);
  virtual ~VerticalLine() {}

  // Widget
  virtual void draw(Curses::ViewPort area);

protected:
private:
  CONSUI_DISABLE_COPY(VerticalLine);
};

} // namespace CppConsUI

#endif // __VERTICALLINE_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
