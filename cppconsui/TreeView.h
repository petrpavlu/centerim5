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
		typedef struct {
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
			sigc::connection sig_moveresize;
			sigc::connection sig_focus;
		} TreeNode;
		typedef tree<TreeNode> TheTree;
		typedef TheTree::pre_order_iterator NodeReference;

		TreeView(int w, int h, LineStyle::Type ltype = LineStyle::DEFAULT);
		virtual ~TreeView();
	
		virtual void Draw(void);

		virtual bool SetFocusChild(Widget &child);
		virtual bool StealFocus(void);

		/* Actions for keybindings */
		void Collapse(const NodeReference node);
		void ActionCollapse(void);
		void Expand(const NodeReference node);
		void ActionExpand(void);
		void ToggleCollapsed(const NodeReference node);
		void ActionToggleCollapsed(void);

		const NodeReference Root(void)
			{ return thetree.begin(); }

		const NodeReference AddNode(const NodeReference &parent, Widget *widget, void *data);
		void DeleteNode(const NodeReference &node, bool keepchildren);
		void DeleteChildren(const NodeReference &node, bool keepchildren);

		const TreeView::NodeReference& GetSelected(void);
		int GetDepth(const NodeReference &node);
		void* SetData(const NodeReference &node, void *newdata);
		void* GetData(const NodeReference &node);
		Widget* GetWidget(const NodeReference &node);
		void SetParent(NodeReference node, NodeReference newparent);

		virtual void GetFocusChain(FocusChain& focus_chain, FocusChain::iterator parent);

		void SetFocusCycle(bool cycle) { focus_cycle = cycle; }
		bool GetFocusCycle(void) { return focus_cycle; }

		void SetActive(int i);
		int GetActive(void); //TODO better name?

	protected:
		void DeleteNode(const NodeReference &node);
		const NodeReference AddNode(const NodeReference &parent, TreeNode &node);
		int DrawNode(TheTree::sibling_iterator node, int top);

		TheTree thetree;
		NodeReference focus_node;

		LineStyle linestyle;
		int itemswidth, itemsheight;

		bool focus_cycle;

	private:
		TreeView();
		TreeView(const TreeView&);

		TreeView& operator=(const TreeView&);
		
		/* Handlers of signals */
		void OnChildRedraw(Widget& widget);
		void OnChildMoveResize(Widget& widget, Rect &oldsize, Rect &newsize);
		void OnChildFocus(Widget& widget, bool focus);

		/** it handles the automatic registration of defined keys */
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

#endif /* __TREEVIEW_H__ */
