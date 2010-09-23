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
 * HorizontalListBox class.
 *
 * @ingroup cppconsui
 */

#ifndef __HORIZONTALLISTBOX_H__
#define __HORIZONTALLISTBOX_H__

#include "AbstractListBox.h"

/**
 * Implementation of AbstractListBox class where widgets are placed
 * horizontally.
 */
class HorizontalListBox
: public AbstractListBox
{
	public:
		HorizontalListBox(int w, int h);
		virtual ~HorizontalListBox() {}

		// Widget
		virtual void UpdateArea();
		virtual void Draw();

		// AbstractListBox
		virtual void AppendSeparator();
		virtual void AppendWidget(Widget& widget);

		// Container
		virtual void RemoveWidget(Widget& widget);
		virtual Curses::Window *GetSubPad(const Widget& child, int begin_x,
				int begin_y, int ncols, int nlines);

	protected:
		int children_width;
		int autosize_children;
		int autosize_width;
		std::set<const Widget *> autosize_extra;

		// Container
		virtual void OnChildMoveResize(Widget& widget, Rect& oldsize,
				Rect& newsize);

		void UpdateScrollWidth();

	private:
		HorizontalListBox(const HorizontalListBox&);
		HorizontalListBox& operator=(const HorizontalListBox&);
};

#endif /* __HORIZONTALLISTBOX_H__ */
