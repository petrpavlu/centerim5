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

#ifndef __SCROLLPANE_H__
#define __SCROLLPANE_H__

#include "Container.h"

class ScrollPane
: public Container
{
	public:
		ScrollPane(Widget& parent, int x, int y, int w, int h, int scrollw, int scrollh);
		virtual ~ScrollPane();

		void UpdateArea();

		virtual void Draw(void);

		/* Functions required by scrollpane abstract class */

		/* Get the size of the scrollable area */
		Rect GetScrollSize(void) { return Rect(0, 0, scrollw, scrollh); }
		int GetScrollWidth(void) { return scrollw; }
		int GetScrollHeight(void) { return scrollh; }

		/* Set the size of the scrollable area*/
		void SetScrollSize(const int width, const int height);
		void SetScrollWidth(const int width)
			{ SetScrollSize(width, scrollh); }
		void SetScrollHeight(const int height)
			{ SetScrollSize(scrollw, height); }

		/* Adjust the visible area to include the given coordinates/rectangle */
		void AdjustScroll(int newx, int newy);

		/* Get the visible scroll area coordinates */
		Rect GetScrollPosition(void) { return Rect(xpos, ypos, 0, 0); }

	protected:
		int scrollw, scrollh;
		int xpos, ypos;

	private:
		ScrollPane();
		ScrollPane(const ScrollPane&);

		ScrollPane& operator=(const ScrollPane&);

		Curses::Window *scrollarea;
};

#endif /* __SCROLLPANE_H__ */
