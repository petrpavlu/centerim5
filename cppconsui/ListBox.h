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

#ifndef __LISTBOX_H__
#define __LISTBOX_H__

#include "AbstractListBox.h"

class ListBox
: public AbstractListBox
{
	public:
		ListBox(Widget& parent, int x, int y, int w, int h);
		virtual ~ListBox();

		void AddSeparator();

		virtual void AddWidget(Widget *widget);
		virtual void RemoveWidget(Widget *widget);

	protected:

	private:
		ListBox();
		ListBox(const ListBox&);

		ListBox& operator=(const ListBox&);

		bool movingwidget;
};

#endif /* __LISTBOX_H__ */
