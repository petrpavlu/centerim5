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
 * ListBox class.
 *
 * @ingroup cppconsui
 */

#ifndef __LISTBOX_H__
#define __LISTBOX_H__

#include "AbstractListBox.h"

/**
 * Implementation of AbstractListBox class where widgets are placed
 * vertically.
 */
class ListBox
: public AbstractListBox
{
	public:
		ListBox(int x, int y, int w, int h);
		virtual ~ListBox() {}

		// AbstractListBox
		void AddSeparator();

		// Container
		virtual void AddWidget(Widget& widget);
		virtual void RemoveWidget(Widget& widget);

	protected:

	private:
		ListBox();
		ListBox(const ListBox&);
		ListBox& operator=(const ListBox&);
};

#endif /* __LISTBOX_H__ */
