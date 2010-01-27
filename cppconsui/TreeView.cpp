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

#include "ConsuiCurses.h"

#include "ScrollPane.h"
#include "LineStyle.h"
#include "Keys.h"

#include "ConsuiLog.h"

/* Put this macro around variable that will be used in the future,
 * remove macro when variables are used.
 * This macro is to get rid of compiler warnings
 */
#define VARIABLE_NOT_USED(x) x=x;

#define CONTEXT_TREEVIEW "treeview"

TreeView::TreeView(Widget& parent, int x, int y, int w, int h, LineStyle::Type ltype)
: ScrollPane(parent, x, y, w, h, w, h)
, linestyle(NULL)
, itemswidth(0)
, itemsheight(0)
, focus_cycle(false)
{
	linestyle = new LineStyle(ltype);

	/* initialise the tree */
	TreeNode root;
	root.widget = NULL;
	root.widgetheight = 0;
	root.collapsable = false;
	root.open = true;
	thetree.set_head(root);
	focus_node = thetree.begin();

	//TODO remove the next line
	//we should change the scroll size to the maximum possible
	//(height of completely unfolded tree) when nodes are added/removed
	//for this we need to maintain the widgetheight value correctly
	SetScrollSize(w, 200);

	AdjustScroll(0, 0);
	DeclareBindables();
}

TreeView::~TreeView()
{
	//TODO deletenode does not free the memory, do this
	DeleteNode(thetree.begin(), false);
	delete linestyle; //TODO this shouldn't be deletable, in fact fix the whole linestyle thingie (the principle)
}

void TreeView::DeclareBindables()
{
	DeclareBindable(CONTEXT_TREEVIEW, "fold-subtree", sigc::mem_fun(this, &TreeView::ActionCollapse),
					InputProcessor::Bindable_Normal);
	DeclareBindable(CONTEXT_TREEVIEW, "unfold-subtree", sigc::mem_fun(this, &TreeView::ActionExpand),
					InputProcessor::Bindable_Normal);
}

DEFINE_SIG_REGISTERKEYS(TreeView, RegisterKeys);
bool TreeView::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_TREEVIEW, "fold-subtree", _("Collapse the selected subtree"), "-");
	RegisterKeyDef(CONTEXT_TREEVIEW, "unfold-subtree", _("Expand the selected subtree"), "+");
	return true;
}

void TreeView::Draw(void)
{
	Curses::werase(area);
	DrawNode(thetree.begin(), 0);
	ScrollPane::Draw();
}

int TreeView::DrawNode(TheTree::sibling_iterator node, int top)
{
	int height = 0, j = top, oldh, depthoffset;
	TheTree::sibling_iterator i;
	TreeNode *child, *parent;

	depthoffset = thetree.depth(node) * 3;

	parent = &(*node);

	/* draw the parent node first */
	if (parent->widget) {
		parent->widget->Move(depthoffset + 3, top);
		parent->widget->Draw();
		height += parent->widget->Height();
	}

	if (parent->open) {
		if (thetree.number_of_children(node) > 0) {
			for (j = top+1; j < top + height; j++)
				Curses::mvwaddstring(area, j, depthoffset + 1, 1, linestyle->V());
		}

		for (i = node.begin(); i != node.end(); i++) {
			child = &(*i);

			if (i != node.back())
				Curses::mvwaddstring(area, top + height,  depthoffset + 1, 1, linestyle->VRight());
			else
				Curses::mvwaddstring(area, top + height,  depthoffset + 1, 1, linestyle->CornerBL());

			Curses::mvwaddstring(area, top + height, depthoffset + 2, 1, linestyle->H());
			
			if (child->collapsable && thetree.number_of_children(i) > 0) {
				Curses::mvwaddstring(area, top + height, depthoffset + 3, 1, "[");
				Curses::mvwaddstring(area, top + height, depthoffset + 4, 1, child->open ? "-" : "+");
				Curses::mvwaddstring(area, top + height, depthoffset + 5, 1, "]");
			} else {
				Curses::mvwaddstring(area, top + height, depthoffset + 3, 1, linestyle->H());
				Curses::mvwaddstring(area, top + height, depthoffset + 4, 1, linestyle->H());
				Curses::mvwaddstring(area, top + height, depthoffset + 5, 1, linestyle->HEnd());
			}

			oldh = height;
			height += DrawNode(i, top + height);

			if (i != node.back()) {
				for (j = top + oldh + 1; j < top + height ; j++)
					Curses::mvwaddstring(area, j, depthoffset + 1, 1, linestyle->V());
			}
		}
	}
	return height;
}

bool TreeView::SetFocusChild(Widget* child)
{
	TheTree::pre_order_iterator i;

	g_assert(child != NULL);

	if (ScrollPane::SetFocusChild(child)) {
		//TODO do we want to use widget.data for this? or is
		//this better?
		for (i = thetree.begin(); i != thetree.end(); i++) {
			if ((*i).widget == child) {
				focus_node = i;
				break;
			}
		}

		return true;
	}

	return false;
}

bool TreeView::StealFocus(void)
{
	if (ScrollPane::StealFocus()) {
		focus_node = thetree.begin();
		return true;
	}

	return false;
}

void TreeView::Collapse(const NodeReference node)
{
	if (node->open && node->collapsable) {
		node->open = false;
		Redraw();
	}
}

void TreeView::Expand(const NodeReference node)
{
	if (!(*node).open && (*node).collapsable) {
		(*node).open = true;
		Redraw();
	}
}

void TreeView::ToggleCollapsed(const NodeReference node)
{
	if ((*node).collapsable) {
		(*node).open = !(*node).open;
		Redraw();
	}
}

void TreeView::ActionCollapse(void)
{
	Collapse(focus_node);
}

void TreeView::ActionExpand(void)
{
	Expand(focus_node);
}

void TreeView::ActionToggleCollapsed(void)
{
	ToggleCollapsed(focus_node);
}

const TreeView::NodeReference TreeView::AddNode(const NodeReference &parent, Widget *widget, void *data)
{
	int newwidth = 0, newheight = 0;
	TheTree::pre_order_iterator iter;

	//TODO check input and throw some errors (or return -1?)
	g_assert(widget != NULL);

	/* construct the new node */
	TreeNode node;
	node.widget = widget;
	node.widgetheight = 0;
	node.data = data;
	node.collapsable = true;
	node.open = true;

	iter = thetree.append_child(parent, node);
	(*parent).widgetheight += node.widget->Height();

	if ((*focus_node).widget == NULL) {
		focus_node = iter;
		widget->GrabFocus();
	}

	node.sig_redraw = widget->signal_redraw.connect(sigc::mem_fun(this, &TreeView::OnChildRedraw));
	node.sig_resize = widget->signal_resize.connect(sigc::mem_fun(this, &TreeView::OnChildResize));
	node.sig_focus = widget->signal_focus.connect(sigc::mem_fun(this, &TreeView::OnChildFocus));
	
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

	//ResizeScroll(newwidth, newheight);

	return iter;
}

const TreeView::NodeReference TreeView::AddNode(const NodeReference &parent, TreeNode &node)
{
	TheTree::pre_order_iterator iter;

	iter = thetree.append_child(parent, node);
	(*parent).widgetheight += node.widget->Height();

	return iter;
}

void TreeView::DeleteNode(const NodeReference &node, bool keepchildren)
{
	// @todo This needs more testing.

	/* If we don't keep children then make sure that focus isn't held by a
	 * descendant. */
	bool has_focus = false;
	if (!keepchildren) {
		NodeReference act = focus_node;
		while (act.node) {
			if (act == node) {
				has_focus = true;
				break;
			}
			act = act.node->parent;
		}
	}

	if (node == focus_node // for keepchildren = true
			|| has_focus // for keepchildren = false
			) {
		/* By folding this node and then moving focus forward we are sure that
		 * no child node of the node to be removed will get the focus. */
		(*node).open = false;
		MoveFocus(Container::FocusNext);
	}

	// if we want to keep child nodes we should flatten the tree
	if (keepchildren)
		thetree.flatten(node);

	//TODO does this disconnect the signals properly? i think so.
	thetree.erase(node);
}

void TreeView::DeleteChildren(const NodeReference &node, bool keepchildren)
{
	TheTree::sibling_iterator i;
	//TODO does this disconnect the signals properly? i think so.
	//LOG("/tmp/delete.log", "DeleteChildren %p, %p, %b\n", GetWidget(node), GetData(node), (int)keepchildren);
	
	while ((i=thetree.begin(node)) != thetree.end(node)){
		DeleteNode(i, keepchildren);
	}
/*		
	for (i = thetree.begin(node); i != thetree.end(node); i++) {
		DeleteNode(i, keepchildren);
	}
 */
}

const TreeView::NodeReference& TreeView::GetSelected(void)
{
	return focus_node;
}

int TreeView::GetDepth(const NodeReference &node)
{
	return thetree.depth(node);
}

void* TreeView::SetData(const NodeReference &node, void *newdata)
{
	void *olddata = NULL;

	olddata = (*node).data;
	(*node).data = newdata;

	return olddata;
}

void* TreeView::GetData(const NodeReference &node)
{
	return (*node).data;
}

Widget* TreeView::GetWidget(const NodeReference &node)
{
	return (*node).widget;
}

void TreeView::SetParent(NodeReference node, NodeReference newparent)
{
	if (thetree.parent(node) != newparent)
		thetree.move_ontop(thetree.append_child(newparent), node);
}

void TreeView::OnChildRedraw(Widget *widget)
{
	Redraw();
}

void TreeView::OnChildResize(Widget *widget, Rect &oldsize, Rect &newsize)
{
	//TODO redraw only on height change
	Redraw();
}

void TreeView::OnChildFocus(Widget* widget, bool focus)
{
	/* Only adjust scroll position if the widget got focus. */
	if (focus)
		AdjustScroll(widget->Left(), widget->Top());
}

void TreeView::GetFocusChain(FocusChain& focus_chain, FocusChain::iterator parent)
{
	Widget *widget;
	Container *container;
	FocusChain::pre_order_iterator iter;
	TheTree::pre_order_iterator i;
	TreeNode *node;

	/* The preorder iterator starts with the root so we must skip it. */
	for (i = ++thetree.begin(); i != thetree.end(); i++) {
		node = &(*i); 
		widget = node->widget;
		container = dynamic_cast<Container*>(widget);

		//TODO implement widget visibility.
		//TODO dont filter out widgets in this function (or overriding
		//functions in other classes. filter out somewhere else.
		//eg when sorting before use.
		if (widget->CanFocus() /*&& widget->Visible() */) {
			iter = focus_chain.append_child(parent, widget);
		} else if (container != NULL) {
			iter = focus_chain.append_child(parent, NULL);
		}

		if (!node->open)
			i.skip_children();

		if (container) {
			/* the widget is a container so add its widgets
			 * as well.
			 * */
			container->GetFocusChain(focus_chain, iter);
		}
	}
}
void TreeView::SetActive(int i)
{
	TheTree::pre_order_iterator j;

	if (i < thetree.size()) {
		for (j = thetree.begin(); i > 0 &&j != thetree.end(); j++, i--) {}
		if (j != thetree.end() && (*j).widget)
			(*j).widget->GrabFocus();
	}
}

int TreeView::GetActive(void)
{
	TheTree::pre_order_iterator j;
	int i;

	for (j = thetree.begin(), i = 0; j != thetree.end(); j++, i++) {
		if ((*j).widget  && (*j).widget->HasFocus()) {
			return i;
		}
	}

	return 0;
}
