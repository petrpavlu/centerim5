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

/** @file Container.h Container class
 *  @ingroup cppconsui
 */

#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include <vector>

#include "Widget.h"
#include "tree.hh"
/** The generic widget container class
 * 
 * It implements @ref MoveFocus "moving focus" in different @ref Widget::FocusDirection "directions".
 *
 * 
 */
class Container
: public Widget
{
	public:
		/** type to keep a tree of "focusable" widgets as leaves and 
		 *   Containers as internal nodes
		 */
		typedef tree<Widget*> FocusChain;
		typedef enum {
			FocusCycleNone, ///< the focus doesn't cycle, it ends at the last widget from the focus chain
			FocusCycleLocal, ///< the focus cycles only 
			FocusCycleGlobal ///< the focus cycles also through the other containers windows
		} FocusCycleScope;

		Container(int w, int h);
		virtual ~Container();

		/** These functions are used to call Widget::UpdateArea of all children when necessary 
		 * @todo replace these functions by reimplementing the virtual UpdateArea method
		 * @{
		 */
		void UpdateAreas(void);
		virtual void MoveResize(const int newx, const int newy, const int neww, const int newh);
		virtual void Draw(void);
		 /** @} */
		/** Adds the widget to the children list and steals the reference. It means that 
		 * the widget will get deleted by the Container if not @ref RemoveWidget "removed" 
		 * before the Container gets deleted.
		 * @todo should we check if the widget has the current container as parent ? Should this be 
		 *  mandatory ?
		 */
		virtual void AddWidget(Widget &widget, int x, int y);
		/** Removes the widget from the children list but it doesn't delete the widget.
		 * The caller must ensure proper deletion of the widget.
		 */
		virtual void RemoveWidget(Widget &widget);
		
		/** Does the proper cleanup of all the children widgets
		 */
		virtual void Clear(void);

		/** The following functions implement the focus cycling between the 
		 * children widgets.
		 * @{
		 */
		/** It builds the tree of the focus chain starting from this container and puts it into
		 * the focus_chain tree as a subtree of @ref parent.
		 */
		virtual void GetFocusChain(FocusChain& focus_chain, FocusChain::iterator parent);
		/** 
		 * @todo have a return value (to see if focus was moved successfully or not) ?
		 */
		virtual void MoveFocus(FocusDirection direction);

		virtual void SetActive(int i);
		virtual int GetActive(void); //TODO better name?
		
		void FocusCycle(FocusCycleScope scope) { focus_cycle_scope = scope; }
		FocusCycleScope FocusCycle(void) const { return focus_cycle_scope; }
		/** @} */

	protected:
		/** It keeps a child widget, along with the redraw connection
		 * @todo implement some constructor to easily connect to the widget's redraw signal
		 * @todo use reference to widget instead of pointer ?
		 */
		typedef struct {
			Widget *widget;

			/* signal connection to the widget */
			sigc::connection sig_redraw;
			//sigc::connection sig_resize;
			//sigc::connection sig_focus;
		} Child;
		typedef std::vector<Child> Children;

		FocusCycleScope focus_cycle_scope;

		Children::iterator ChildrenBegin(void)
			{ return children.begin(); }
		Children::iterator ChildrenEnd(void)
			{ return children.end(); }

	private:
		Container(void);
		Container(const Container&);

		Container& operator=(const Container&);

		void OnChildRedraw(Widget& widget);
		
		Children children;
		
		/** it handles the automatic registration of defined keys */
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

#endif /* __CONTAINER_H__ */
