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
 * AbstractListBox class.
 *
 * @ingroup cppconsui
 */

#ifndef __ABSTRACTLISTBOX_H__
#define __ABSTRACTLISTBOX_H__

#include "ScrollPane.h"

/**
 * Abstract class that defines common interface for ListBox and
 * HorizontalListBox.
 */
class AbstractListBox
: public ScrollPane
{
	public:
		AbstractListBox(int w, int h);
		virtual ~AbstractListBox() {}

		/**
		 * Adds a new button in the end of ListBox.
		 */
		void AppendItem(const gchar *title, sigc::slot<void> function);
		/**
		 * Appends a separator (usually horizontal or vertical line) into the
		 * ListBox.
		 */
		virtual void AppendSeparator() = 0;
		/**
		 * Appends a widget into the ListBox.
		 */
		virtual void AppendWidget(Widget& widget) = 0;

	protected:
		// Container
		virtual void AddWidget(Widget& widget, int x, int y);

	private:
		AbstractListBox(const AbstractListBox&);
		AbstractListBox& operator=(const AbstractListBox&);
};

#endif /* __ABSTRACTLISTBOX_H__ */
