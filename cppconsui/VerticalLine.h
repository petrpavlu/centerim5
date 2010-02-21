/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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

/** @file HorizontalLine.h HorizontalLine class
 * @ingroup cppconsui
 */

#ifndef __VERTICALLINE_H__
#define __VERTICALLINE_H__

#include "LineStyle.h"
#include "Widget.h"

/**
 * A widget representing a vertical line.
 */
class VerticalLine
: public Widget
{
	public:
		VerticalLine(Widget& parent, const int x, const int y, const int w,
				LineStyle::Type ltype = LineStyle::DEFAULT);
		virtual ~VerticalLine();

		// Widget
		virtual void Draw();

		void SetLineStyle(LineStyle::Type ltype);
		LineStyle::Type GetLineStyle();

	protected:
		LineStyle linestyle;

	private:
		VerticalLine();
		VerticalLine(const VerticalLine&);
		VerticalLine& operator=(const VerticalLine&);
};

#endif /* __VERTICALLINE_H__ */
