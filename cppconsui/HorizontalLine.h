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
 * HorizontalLine class
 *
 * @ingroup cppconsui
 */

#ifndef __HORIZONTALLINE_H__
#define __HORIZONTALLINE_H__

#include "AbstractLine.h"

/**
 * A widget representing a horizontal line.
 */
class HorizontalLine
: public AbstractLine
{
	public:
		HorizontalLine(int w, LineStyle::Type ltype = LineStyle::DEFAULT);
		virtual ~HorizontalLine() {}

		// Widget
		virtual void Draw();

	protected:

	private:
		HorizontalLine();
		HorizontalLine(const HorizontalLine&);
		HorizontalLine& operator=(const HorizontalLine&);
};

#endif /* __HORIZONTALLINE_H__ */
