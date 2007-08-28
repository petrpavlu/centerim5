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

#include "TreeView.h"
#include "Keys.h"

#include "Scrollable.h"
#include "LineStyle.h"
#include "Keys.h"

TreeView::TreeView(WINDOW *parentarea, int x, int y, int w, int h, LineStyle *linestyle)
: Scrollable(parentarea, x, y, w, h, w, h)
, linestyle(linestyle)
, itemswidth(0)
, itemsheight(0)
, focusnode(NULL)
, focuscycle(true)
{
	AddCombo(key_up, sigc::mem_fun(this, &TreeView::FocusPrevious));
	AddCombo(key_down, sigc::mem_fun(this, &TreeView::FocusNext));

	canfocus = true;

	if (!linestyle)
		linestyle == LineStyle::LineStyleDefault();

	root = new TreeNode;
	root->widget = NULL;
	root->depth = -1;
	root->children.clear();
	root->id = 0;
	root->collapsable = false;
	root->open = true;
}

TreeView::~TreeView()
{
	TreeNodes::iterator i;

	delete linestyle;

	DeleteNode(root);
}

void TreeView::Draw(void)
{
	DrawNode(root, 0);
	Scrollable::Draw();
}

int TreeView::DrawNode(TreeNode *node, int top)
{
	int height = 0, j = top, oldh, depthoffset;
	TreeNodes::iterator i;
	TreeNode *child;

	depthoffset = (node->depth+1) * 3;

	/* draw this node first */
	if (node->widget) {
		node->widget->Move(scrollarea, depthoffset + 3, top);
		node->widget->Draw();
		height += node->widget->Height();
	}

	if (node->open) {
		if (node->children.size()) {
			for (j = top+1; j < top+height ; j++)
				mvwadd_wch(scrollarea, j, depthoffset + 1, linestyle->V());
		}

		for (i = node->children.begin(); i != node->children.end(); i++) {
			child = *i;

			if (child != node->children.back())
				mvwadd_wch(scrollarea, top+height,  depthoffset + 1, linestyle->VRight());
			else
				mvwadd_wch(scrollarea, top+height,  depthoffset + 1, linestyle->CornerBL());

			mvwadd_wch(scrollarea, top+height,  depthoffset + 2, linestyle->H());
			
			if (child->collapsable) {
				mvwaddch(scrollarea, top+height, depthoffset + 3, '[');
				mvwaddch(scrollarea, top+height, depthoffset + 4, child->open ? '-' : '+');
				mvwaddch(scrollarea, top+height, depthoffset + 5, ']');
			} else {
				mvwadd_wch(scrollarea, top+height, depthoffset + 3, linestyle->H());
				mvwadd_wch(scrollarea, top+height, depthoffset + 4, linestyle->H());
				mvwadd_wch(scrollarea, top+height, depthoffset + 5, linestyle->HEnd());
			}

			oldh = height;
			height += DrawNode(child, top+height);

			if (child != node->children.back()) {
				for (j = top+oldh+1; j < top+height ; j++)
					mvwadd_wch(scrollarea, j, depthoffset + 1, linestyle->V());
			}
		}
	}
	return height;
}

void TreeView::GiveFocus(void)
{
	focus = true;
	if (focusnode)
		focusnode->widget->GiveFocus();
}

void TreeView::TakeFocus(void)
{
	focus = false;
	if (focusnode)
		focusnode->widget->TakeFocus();
}

void TreeView::FocusNext(void)
{
	TreeNodes::iterator i;
	TreeNode *node, *newfocus = NULL, *parent, *child;

	if (!focusnode) return; /* no nodes have been added yet */
	//if (root->children.size() == 1) /* only one noe can have focus (and has it) */
	//why does that segfault?

	node = focusnode;

	if (node->open && node->children.size()) {
		newfocus = node->children.front();
	} else {
		parent = FindParent(node->id);

		for (i = parent->children.begin(); i != parent->children.end(); ) {
			child = *i;
			if (child == node) {
				i++;
				if (i == parent->children.end()) {
					if (parent == root) {
						if (focuscycle) {
							newfocus = *(parent->children.begin());
							break;
						} else {
							return;
						}
					}

					node = parent;
					parent = FindParent(parent->id);
					i = parent->children.begin();
					continue;
				} else {
					newfocus = *i;
					break;
				}
			}
			i++;
		}
	}

	if (newfocus) {
		focusnode->widget->TakeFocus();
		focusnode = newfocus;
		focusnode->widget->GiveFocus();
		SetInputChild(focusnode->widget);
		signal_redraw();
	}
}

void TreeView::FocusPrevious(void)
{
//TODO fix this, doesn't work quite well
	TreeNodes::iterator i;
	TreeNode *node, *newfocus = NULL, *parent, *child;

	if (!focusnode) return; /* no nodes have been added yet */
	//if (root->children.size() == 1) /* only one noe can have focus (and has it) */
	//why does that segfault?

	node = focusnode;

	if (node->open && node->children.size()) {
		newfocus = node->children.front();
	} else {
		parent = FindParent(node->id);

		for (i = parent->children.begin(); i != parent->children.end(); ) {
			child = *i;
			if (child == node) {
				if (i == parent->children.begin()) {
					if (parent == root) {
						if (focuscycle) {
							newfocus = parent->children.back();
							break;
						} else {
							return;
						}
					}

					node = parent;
					parent = FindParent(parent->id);
					i = parent->children.begin();
					continue;
				} else {
					i--;
					newfocus = *i;
					break;
				}
			}
			i++;
		}
	}

	if (newfocus) {
		focusnode->widget->TakeFocus();
		focusnode = newfocus;
		focusnode->widget->GiveFocus();
		SetInputChild(focusnode->widget);
		signal_redraw();
	}
}

int TreeView::AddNode(int parentid, Widget *widget, void *data)
{
	int newwidth, newheight;
	TreeNode *parent, *child, *node;

	//TODO check input and throw some errors (or return -1?)

	/* construct a new node */
	node = new TreeNode;
	node->id = GenerateId();
	node->widget = widget;
	node->data = data;
	node->collapsable = true;
	node->open = true;

	if (!focusnode) {
		focusnode = node;
		SetInputChild(focusnode->widget);
	}

	parent = FindNode(parentid);
	if (parent == NULL)
		parent = root;

	node->depth = parent->depth + 1;
	parent->children.push_back(node);

	//TODO we want the widgets resize events
	//so we can adjust the scroll area
	node->sig_redraw = widget->signal_redraw.connect(sigc::mem_fun(this, &TreeView::OnChildRedraw));
	
	
	itemswidth += widget->Width();
	itemsheight += widget->Height();

	/* only really resize if needed and 
	 * add a bit (read: a lot) of slack space
	 * */
	//TODO change width calculation for configurable tree drawing
	if (scrollw < itemswidth)
		newwidth = itemswidth * 2;
	if (scrollh < itemsheight)
		newheight = itemsheight * 2;

	ResizeScroll(newwidth, newheight);

	return node->id;
}

int TreeView::AddNode(TreeNode *parent, TreeNode *node)
{
	if (!parent)
		return -1;
	
	parent->children.push_back(node);

	return node->id;
}

void TreeView::DeleteNode(int nodeid, bool keepsubnodes)
{
	TreeNodes::iterator i;
	TreeNode *node, *child, *parent;
	int depth;

	node = FindNode(nodeid);

	if (!node)
		return;

	if (focusnode == node) {
		focusnode = NULL;
		SetInputChild(NULL);
	}

	parent = FindParent(nodeid);
	if (!parent)
		parent = root;

	/* Move subnodes to the parent */
	if (keepsubnodes) {
		i = node->children.begin();
		while (i != node->children.end()) {
			child = *i;
			AddNode(parent, child);

			node->children.erase(i);
			i = node->children.begin();
		}
	}

	/* Remove the node to be deleted from its parent */
	for (i = parent->children.begin(); i != parent->children.end(); i++) {
		child = *i;
		if (child == node) {
			parent->children.erase(i);
			break;
		}
	}

	//TODO disconnect signals
	node->sig_redraw.disconnect();

	/* Delete the node objects */
	DeleteNode(node);
}

int TreeView::GetDepth(int nodeid)
{
	TreeNode *node = FindNode(nodeid);

	if (node)
		return node->depth;

	return -1;
}

void* TreeView::SetData(int nodeid, void *newdata)
{
	void *olddata = NULL;
	TreeNode *node = FindNode(nodeid);

	if (node) {
		olddata = node->data;
		node->data = newdata;
	}

	return olddata;
}

void* TreeView::GetData(int nodeid)
{
	TreeNode *node = FindNode(nodeid);

	if (node)
		return node->data;
	
	return NULL;
}

Widget* TreeView::GetWidget(int nodeid)
{
	TreeNode *node = FindNode(nodeid);

	if (node)
		return node->widget;
	
	return NULL;
}

void TreeView::SetParent(int nodeid, Widget *widget, void *data) //TODO changes depth, parentid must be exactle before nodeid
{
}

void TreeView::SetParent(int nodeid, int parentid) //TODO changes depth, parentid must be exactle before nodeid
{
	TreeNodes::iterator i;
	TreeNode *node, *curparent, *newparent, *child;

	node = FindNode(nodeid);
	if (!node) return;

	newparent = FindNode(parentid);
	if (!newparent) return;

	curparent = FindParent(nodeid);

	if (newparent->id == curparent->id)
		return;

	/* Remove the node from its current parent */
	for (i = curparent->children.begin(); i != curparent->children.end(); i++) {
		child = *i;
		if (child == node) {
			curparent->children.erase(i);
			break;
		}
	}

	/* Add the node to its current parent */
	node->depth = newparent->depth + 1;
	newparent->children.push_back(node);

}

TreeView::TreeNode* TreeView::FindNode(int nodeid)
{
	return FindNode(root, nodeid);
}

TreeView::TreeNode* TreeView::FindNode(TreeNode *parent, int nodeid)
{
	TreeNodes::iterator i;
	TreeNode *child = NULL;

	if (nodeid < 0)
		return NULL;

	if (parent->id == nodeid)
		return parent;

	for (i = parent->children.begin(); i != parent->children.end(); i++) {
		if ((child = FindNode(*i, nodeid)) != NULL)
			break;
	}

	return child;
}

TreeView::TreeNode* TreeView::FindParent(int childid)
{
	return FindParent(root, childid);
}

TreeView::TreeNode* TreeView::FindParent(TreeNode *node, int childid)
{
	TreeNodes::iterator i;
	TreeNode *child = NULL, *parent;

	if (childid == -1)
		return NULL;

	for (i = node->children.begin(); i != node->children.end(); i++) {
		child = *i;
		if (child->id == childid) {
			return node;
		} else {
			parent = FindParent(child, childid);
			if (parent)
				return parent;
		}
	}

	return NULL;
}

void TreeView::DeleteNode(TreeNode *node)
{
	TreeNodes::iterator i;

	for (i = node->children.begin(); i != node->children.end(); i++)
		DeleteNode(*i);

	delete node;
}

int TreeView::GenerateId(void)
{
	static int i = 0;

	i++;
	return i;
}

void TreeView::OnChildRedraw(void)
{
	signal_redraw();
}
