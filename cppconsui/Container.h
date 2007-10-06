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

#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include <vector>

#include "Widget.h"

class Container
: public Widget
{
	public:
		Container(WINDOW* parentarea, int x, int y, int w, int h);
		~Container();

		virtual void Move(WINDOW* parentarea, int newx, int newy);
		virtual void Resize(WINDOW* parentarea, int neww, int newh);
		virtual void MoveResize(WINDOW* parentarea, int newx, int newy, int neww, int newh);
		virtual void Draw(void);

		virtual void GiveFocus(void);
		virtual void TakeFocus(void);

		void SetFocusChild(Widget* widget);
		Widget* GetFocusChild(void);

		void AddWidget(Widget *widget);
		void RemoveWidget(Widget *widget);

	protected:
		Widget *focuschild;

	private:
		typedef std::pair<Widget*, sigc::connection> Child;
		typedef std::vector<Child> Children;

		Container();
		Container(const Container&);

		void OnChildRedraw(void);
		
		Children children;
};

#endif /* __CONTAINER_H__ */
