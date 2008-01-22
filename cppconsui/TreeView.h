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

#include "Scrollable.h"
#include "LineStyle.h"

#include <vector>

//TODO subclass of a srollable ???

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
: public Scrollable
{
	public:
		TreeView(Widget& parent, int x, int y, int w, int h, LineStyle *linestyle);
		virtual ~TreeView();

		virtual void Draw(void);

		virtual void GiveFocus(void);
		virtual void TakeFocus(void);

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

		int AddNode(int parentid, Widget *widget, void *data);
		void DeleteNode(int nodeid, bool keepsubnodes);

		int GetSelected(void);
		int GetDepth(int nodeid);
		void* SetData(int nodeid, void *newdata);
		void* GetData(int nodeid);
		Widget* GetWidget(int nodeid);
		void SetParent(int nodeid, Widget *widget, void *data);
		void SetParent(int nodeid, int parentid); //TODO changes depth, parentid must be exactle before nodeid

	protected:
		typedef struct TreeNode;
		typedef std::vector<TreeNode*> TreeNodes;
		typedef struct TreeNode {
			/* unique identifier */
			int id;
			/* widget to show. not const because width is changed to
			 * fit. eg labels can show `...' when the text
			 * does not fit in the given space
			 * */
			Widget *widget;
			/* user supplied data*/
			TreeNodes children;
			void *data;
			/* used for drawing *only* */ //TODO is this true?
			int depth;
			/* stores the total height of all child widgets together */
			int height;
			/* if a subgroup can be collapsed by the *user* or not. defines
			 * if it is possible to *change*. (if open can close or if
			 * closed can open) */
			bool collapsable;
			/* if the subgroup is being displaed */
			bool open;
			/* signals for users to connect to */
			//TODO add more signals we need, see cpp file
			sigc::connection sig_redraw;
		};

		TreeNode* FindNode(int nodeid);
		TreeNode* FindNode(TreeNode *parent, int nodeid);
		TreeNode* FindParent(int childid);
		TreeNode* FindParent(TreeNode *node, int childid);

		void DeleteNode(TreeNode *node);
		int GenerateId(void);
		int AddNode(TreeNode *parent, TreeNode *node);
		int DrawNode(TreeNode *node, int top);

		TreeNode *root, *focusnode;

		LineStyle *linestyle;
		int itemswidth, itemsheight;

		bool focuscycle;

	private:
		TreeView();
		
		void OnChildRedraw(void);
};

#endif /* __TREEVIEW_H__ */
