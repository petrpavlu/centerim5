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

#ifndef __TREEVIEW_H__
#define __TREEVIEW_H__

#include "ScrollPane.h"
#include "LineStyle.h"
#include "tree.hh"

#include <vector>

//NOTES
//
// items / subgroups are not shown at all if the item widget / subgroup item widget
// are not visible
//
// item widgets are resized to fit the width (make configurable?)
// when deleting give a choice to keep subnodes (remember to decrease their depth)
//
// widgets can occupy multiple lines
class TreeView
: public ScrollPane
{
	public:
		struct TreeNode {
			/* if a subtree can be collapsed by the *user* or not. defines
			 * if it is possible to *change*. (if open can fold or if
			 * closed can unfold)
			 * */
			bool collapsable;
			/* if the subtree is unfolded */
			bool open;

			/* the total height of all child widgets together */
			int widgetheight;
			
			/* widget to show. Not const because width is changed to
			 * fit. E.g. labels can show `...' when the text
			 * does not fit in the given space.
			 * */
			Widget *widget;
			/* user supplied data */
			void *data;

			/* signal connection to the widget */
			sigc::connection sig_redraw;
			sigc::connection sig_resize;
			sigc::connection sig_focus;
		};
		typedef tree<TreeNode> TheTree;
		typedef TheTree::pre_order_iterator NodeReference;

		TreeView(Widget& parent, int x, int y, int w, int h, LineStyle *linestyle);
		virtual ~TreeView();

		virtual void Draw(void);

		void FocusNext(void);
		void FocusPrevious(void);

		/* Actions for keybindings */
		void ActionFocusNext(void);
		void ActionFocusPrevious(void);
		void ActionCollapse(void);
		void ActionToggleCollapsed(void);
		void ActionExpand(void);
		void ActionToggleExpanded(void)
			{ ActionToggleCollapsed(); }

		const NodeReference Root(void)
			{ return thetree.begin(); }

		const NodeReference AddNode(const NodeReference &parent, Widget *widget, void *data);
		void DeleteNode(const NodeReference &node, bool keepchildren);

		const TreeView::NodeReference& GetSelected(void);
		int GetDepth(const NodeReference &node);
		void* SetData(const NodeReference &node, void *newdata);
		void* GetData(const NodeReference &node);
		Widget* GetWidget(const NodeReference &node);
		void SetParent(NodeReference node, NodeReference newparent);

		virtual void GetFocusChain(FocusChain& focus_chain, FocusChain::iterator parent);

	protected:
		void DeleteNode(const NodeReference &node);
		const NodeReference AddNode(const NodeReference &parent, TreeNode &node);
		int DrawNode(TheTree::sibling_iterator node, int top);

		void SetFocusCycle(bool cycle) { focus_cycle = cycle; }
		bool GetFocusCycle(void) { return focus_cycle; }

		TheTree thetree;
		NodeReference focusnode;

		LineStyle *linestyle;
		int itemswidth, itemsheight;

		bool focus_cycle;

	private:
		TreeView();
		TreeView(const TreeView&);

		TreeView& operator=(const TreeView&);
		
		void OnChildRedraw(Widget *widget);
		void OnChildResize(Widget *widget, Rect &oldsize, Rect &newsize);
		void OnChildFocus(Widget* widget, bool focus);
};

#endif /* __TREEVIEW_H__ */
