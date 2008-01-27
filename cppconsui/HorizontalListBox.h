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

#ifndef __HORIZONTALLISTBOX_H__
#define __HORIZONTALLISTBOX_H__

#include "AbstractListBox.h"

class HorizontalListBox
: public AbstractListBox
{
	public:
		HorizontalListBox(Widget& parent, int x, int y, int w, int h);
		virtual ~HorizontalListBox();

		virtual void AddWidget(Widget *widget) = 0;
		virtual void RemoveWidget(Widget *widget) = 0;

	protected:

	private:
		bool movingwidget;

		HorizontalListBox(void);
		HorizontalListBox(const HorizontalListBox&);

		HorizontalListBox& operator=(const HorizontalListBox&);
};

#endif /* __HORIZONTALLISTBOX_H__ */
