/*
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
 * AbstractLine class.
 *
 * @ingroup cppconsui
 */

#ifndef __ABSTRACTLINE_H__
#define __ABSTRACTLINE_H__

#include "Widget.h"
#include "LineStyle.h"

/**
 * Abstract class that defines common interface for VerticalLine and
 * HorizontalLine.
 */
class AbstractLine
: public Widget
{
	public:
		AbstractLine(Widget& parent, int x, int y, int w, int h,
				LineStyle::Type ltype = LineStyle::DEFAULT);
		virtual ~AbstractLine() {}

		/**
		 * Sets a new line style.
		 */
		void SetLineStyle(LineStyle::Type ltype);
		/**
		 * Returns a current line style.
		 */
		LineStyle::Type GetLineStyle();

	protected:
		LineStyle linestyle;

	private:
		AbstractLine();
		AbstractLine(const AbstractLine&);
		AbstractLine& operator=(const AbstractLine&);
};

#endif /* __ABSTRACTLINE_H__ */