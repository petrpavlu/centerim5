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

class Scrollable
{
	public:
		Scrollable() { ; }
		virtual ~Scrollable() { ; }
		/* Get the size of the scrollable area */
		virtual Rect GetScrollSize(void) = 0;

		/* Set the size of the scrollable area*/
		virtual void SetScrollSize(const int width, const int height) = 0;

		/* Adjust the visible area to include the given coordinates */
		virtual void AdjustScroll(const int x, const int y) = 0;
		/* Adjust the visible area to include the given rectangle, and
		 * to include _at_least_ (Rect.x, Rect.y). 
		 * This is useful when e.g. text appears right-to-left. We want
		 * to see the text, but at least the start of it, which is
		 * at the right.
		 *
		 * In that case Rect.w and Rect.h will be negative.
		 * */
		virtual void AdjustScroll(const Rect) = 0;

		/* Get the visible scroll area coordinates */
		virtual Rect GetScrollPosition(void) = 0;

	protected:

	private:
		Scrollable(const Scrollable&);

		Scrollable& operator=(const Scrollable&);
};

#endif /* __SCROLLABLE_H__ */
