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

#ifndef __TREEVIEW_H__
#define __TREEVIEW_H__

#include "ScrollPane.h"
#include "LineStyle.h"

#include "tree.hh"

class TreeView
: public ScrollPane
{
	protected:
		class TreeNode;

	public:
		/**
		 * Node drawing style.
		 */
		enum Style {
			STYLE_NORMAL, ///< Draw "[+]" if the node is collapsed.
			STYLE_VOID ///< Don't draw any extra information.
		};

		typedef tree<TreeNode> TheTree;
		typedef TheTree::pre_order_iterator NodeReference;

		TreeView(int w, int h, LineStyle::Type ltype = LineStyle::DEFAULT);
		virtual ~TreeView();
	
		// Widget
		virtual void MoveResize(int newx, int newy, int neww, int newh);
		virtual void Draw();
		virtual bool SetFocusChild(Widget& child);
		virtual bool StealFocus();
		virtual Curses::Window *GetSubPad(const Widget& child, int begin_x,
				int begin_y, int ncols, int nlines);

		// Container
		virtual void GetFocusChain(FocusChain& focus_chain,
				FocusChain::iterator parent);
		virtual void SetActive(int i);
		virtual int GetActive() const;

		/**
		 * Folds given node.
		 */
		void Collapse(const NodeReference node);
		/**
		 * Unfolds given node.
		 */
		void Expand(const NodeReference node);
		/**
		 * Toggles folding for given node.
		 */
		void ToggleCollapsed(const NodeReference node);
		void ActionToggleCollapsed();

		/**
		 * Returns root node reference.
		 */
		const NodeReference Root()
			{ return thetree.begin(); }

		/**
		 * Inserts a widget before a specified position. TreeView takes
		 * ownership of the widget.
		 */
		const NodeReference InsertNode(const NodeReference position,
				Widget& widget);
		/**
		 * Inserts a widget after a specified position. TreeView takes
		 * ownership of the widget.
		 */
		const NodeReference InsertNodeAfter(const NodeReference position,
				Widget& widget);
		/**
		 * Prepends a widget to a specified parent. TreeView takes ownership
		 * of the widget.
		 */
		const NodeReference PrependNode(const NodeReference parent,
				Widget& widget);
		/**
		 * Appends a widget to a specified parent. TreeView takes ownership of
		 * the widget.
		 */
		const NodeReference AppendNode(const NodeReference parent,
				Widget& widget);

		/**
		 * Deletes given node.
		 */
		void DeleteNode(const NodeReference node, bool keepchildren);
		/**
		 * Deletes all children of given node.
		 */
		void DeleteChildren(const NodeReference node, bool keepchildren);

		/**
		 * Returns reference to currently focused node/widget.
		 */
		const NodeReference GetSelected();

		/**
		 * Returns node depth.
		 */
		int GetDepth(const NodeReference node);

		Widget *GetWidget(const NodeReference node);
		/**
		 * Detaches a given node from its current location and moves it before
		 * a given position.
		 */
		void MoveBefore(const NodeReference node,
				const NodeReference position);
		/**
		 * Detaches a given node from its current location and moves it after
		 * a given position.
		 */
		void MoveAfter(const NodeReference node,
				const NodeReference position);
		/**
		 * Detaches a given node from its current location and appents it to
		 * a new parent.
		 */
		void SetParent(const NodeReference node,
				const NodeReference newparent);
		//void SetPosition(const NodeReference node, int index);
		void SetStyle(const NodeReference node, Style s);
		Style GetStyle(const NodeReference node) const;

	protected:
		class TreeNode {
			/* If TreeNode would be just protected/private and all variables
			 * public then variables can be accessed from outside using
			 * NodeReference. */
			friend class TreeView;

			public:

			protected:

			private:
				/**
				 * Flag whether the subtree is unfolded.
				 */
				bool open;

				/**
				 * Selected node drawing style.
				 */
				Style style;

				/**
				 * Widget to show. Not const because width is changed to fit.
				 * E.g. labels can show `...' when the text does not fit in
				 * the given space.
				 */
				Widget *widget;

				/**
				 * Signal connection to the widget.
				 * @todo Review.
				 */
				sigc::connection sig_redraw;
				sigc::connection sig_moveresize;
				sigc::connection sig_focus;
		};

		// Container
		virtual void AddWidget(Widget& widget, int x, int y);

		int DrawNode(TheTree::sibling_iterator node, int top);

		TreeNode AddNodeInit(Widget& widget);
		void AddNodeFinalize(NodeReference& iter);

		TheTree thetree;
		NodeReference focus_node;

		LineStyle linestyle;

	private:
		TreeView(const TreeView&);
		TreeView& operator=(const TreeView&);
		
		/* Handlers of signals */
		void OnChildRedraw(Widget& widget);
		void OnChildMoveResize(Widget& widget, Rect &oldsize, Rect &newsize);
		void OnChildFocus(Widget& widget, bool focus);

		void ActionCollapse();
		void ActionExpand();

		/** it handles the automatic registration of defined keys */
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

#endif /* __TREEVIEW_H__ */
