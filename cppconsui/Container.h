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
#include "tree.hh"

class Container
: public Widget
{
	public:
		typedef tree<Widget*> FocusChain;
		typedef enum {
			FocusNext,
			FocusPrevious,
			FocusDown,
			FocusUp,
			FocusRight,
			FocusLeft
		} FocusDirection;

		Container(Widget& parent, const int x, const int y, const int w, const int h);
		virtual ~Container();

		void UpdateAreas(void);
		virtual void Move(const int newx, const int newy);
		virtual void Resize(const int neww, const int newh);
		virtual void MoveResize(const int newx, const int newy, const int neww, const int newh);
		virtual void Draw(void);

		void AddWidget(Widget *widget);
		void RemoveWidget(Widget *widget);

		virtual void FocusCyclePrevious(void);
		virtual void FocusCycleNext(void);

		virtual void Clear(void);

		virtual void GetFocusChain(FocusChain& focus_chain, FocusChain::iterator parent);
		virtual void MoveFocus(FocusDirection direction);

	protected:
		typedef std::pair<Widget*, sigc::connection> Child;
		typedef std::vector<Child> Children;

		bool focus_cycle;

		void SetFocusCycle(bool cycle) { focus_cycle = cycle; }
		bool GetFocusCycle(void) { return focus_cycle; }

		Children::iterator ChildrenBegin(void)
			{ return children.begin(); }
		Children::iterator ChildrenEnd(void)
			{ return children.end(); }

	private:
		Container(void);
		Container(const Container&);

		Container& operator=(const Container&);

		void OnChildRedraw(Widget* widget);
		
		Children children;
};

#endif /* __CONTAINER_H__ */
