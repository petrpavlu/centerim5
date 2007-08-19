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
		Scrollable(WINDOW* parentarea, int x, int y, int w, int h, int scrollw, int scrollh);
		~Scrollable();

		virtual void Draw(void);

		int ScrollWidth(void) { return scrollw; }
		int ScrollHeight(void) { return scrollh; }
	protected:
		void Scroll(const char *key);
		void ResizeScroll(int neww, int newh);

		WINDOW* scrollarea;
		int scrollw, scrollh;
		int xpos, ypos;

	private:
		Scrollable();
};

#endif /* __SCROLLABLE_H__ */
