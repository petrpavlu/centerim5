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

#ifndef __SCROLLABLE_H__
#define __SCROLLABLE_H__

#include "Widget.h"

class Scrollable
: public Widget
{
	public:
		Scrollable(Widget& parent, int x, int y, int w, int h, int scrollw, int scrollh);
		virtual ~Scrollable();

		void UpdateArea();

		virtual void Draw(void);

		int ScrollWidth(void) { return scrollw; }
		int ScrollHeight(void) { return scrollh; }

		void AdjustScroll(const int newx, const int newy);

	protected:
		void Scroll(const int deltax, const int deltay);
		void ResizeScroll(int neww, int newh);

		int scrollw, scrollh;
		int xpos, ypos;

	private:
		Scrollable();

		WINDOW* scrollarea;
};

#endif /* __SCROLLABLE_H__ */
