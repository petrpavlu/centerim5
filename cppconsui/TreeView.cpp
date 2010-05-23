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

#include "TreeView.h"

#include "ColorScheme.h"
#include "ConsuiCurses.h"
#include "Keys.h"

#include "gettext.h"

#define CONTEXT_TREEVIEW "treeview"

TreeView::TreeView(int w, int h, LineStyle::Type ltype)
: ScrollPane(w, h, w, h)
, linestyle(ltype)
, itemswidth(0)
, itemsheight(0)
{
	/* initialise the tree */
	TreeNode root;
	root.widget = NULL;
	root.widgetheight = 0;
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
	DeleteNode(thetree.begin(), false);
}

void TreeView::DeclareBindables()
{
	DeclareBindable(CONTEXT_TREEVIEW, "fold-subtree",
			sigc::mem_fun(this, &TreeView::OnActionCollapse),
			InputProcessor::Bindable_Normal);
	DeclareBindable(CONTEXT_TREEVIEW, "unfold-subtree",
			sigc::mem_fun(this, &TreeView::OnActionExpand),
			InputProcessor::Bindable_Normal);
}

DEFINE_SIG_REGISTERKEYS(TreeView, RegisterKeys);
bool TreeView::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_TREEVIEW, "fold-subtree",
			_("Collapse the selected subtree"),
			Keys::UnicodeTermKey("-"));
	RegisterKeyDef(CONTEXT_TREEVIEW, "unfold-subtree",
			_("Expand the selected subtree"),
			Keys::UnicodeTermKey("+"));
	return true;
}

void TreeView::Draw()
{
	if (!area)
		return;

	area->erase();

	DrawNode(thetree.begin(), 0);
	ScrollPane::Draw();
}

bool TreeView::SetFocusChild(Widget& child)
{
	if (ScrollPane::SetFocusChild(child)) {
		/// @todo Speed up this algorithm using extra node data.
		for (TheTree::pre_order_iterator i = thetree.begin(); i != thetree.end(); i++)
			if (i->widget == &child) {
				focus_node = i;
				break;
			}

		return true;
	}

	return false;
}

bool TreeView::StealFocus()
{
	if (ScrollPane::StealFocus()) {
		focus_node = thetree.begin();
		return true;
	}

	return false;
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
	g_assert(i >= 0);
	g_assert(i < (int) thetree.size());

	TheTree::pre_order_iterator j;
	for (j = thetree.begin(); i > 0 && j != thetree.end(); j++, i--)
		;
	if (j != thetree.end() && j->widget)
		j->widget->GrabFocus();
}

int TreeView::GetActive() const
{
	TheTree::pre_order_iterator j;
	int i;

	for (j = thetree.begin(), i = 0; j != thetree.end(); j++, i++)
		if (j->widget && j->widget->HasFocus())
			return i;

	return -1;
}

void TreeView::Collapse(const NodeReference node)
{
	if (node->open) {
		node->open = false;
		signal_redraw(*this);
	}
}

void TreeView::Expand(const NodeReference node)
{
	if (!node->open) {
		node->open = true;
		signal_redraw(*this);
	}
}

void TreeView::ToggleCollapsed(const NodeReference node)
{
	node->open = !node->open;
	signal_redraw(*this);
}

void TreeView::OnActionToggleCollapsed()
{
	ToggleCollapsed(focus_node);
}

const TreeView::NodeReference TreeView::AddNode(const NodeReference parent, Widget& widget)
{
	int newwidth = 0, newheight = 0;
	TheTree::pre_order_iterator iter;

	widget.SetParent(*this);

	/* construct the new node */
	TreeNode node;
	node.open = true;
	node.style = STYLE_NORMAL;
	node.widget = &widget;
	node.widgetheight = 0;

	iter = thetree.append_child(parent, node);
	parent->widgetheight += node.widget->Height();

	if (focus_node->widget == NULL) {
		focus_node = iter;
		widget.GrabFocus();
	}

	node.sig_redraw = widget.signal_redraw.connect(sigc::mem_fun(this, &TreeView::OnChildRedraw));
	node.sig_moveresize = widget.signal_moveresize.connect(sigc::mem_fun(this, &TreeView::OnChildMoveResize));
	node.sig_focus = widget.signal_focus.connect(sigc::mem_fun(this, &TreeView::OnChildFocus));
	
	itemswidth += widget.Width();
	itemsheight += widget.Height();

	/* only really resize if needed and 
	 * add a bit (read: a lot) of slack space
	 * */
	//TODO change width calculation for configurable tree drawing
	if (scroll_width < itemswidth)
		newwidth = itemswidth * 2;
	if (scroll_height < itemsheight)
		newheight = itemsheight * 2;

	//ResizeScroll(newwidth, newheight);

	return iter;
}

void TreeView::DeleteNode(const NodeReference node, bool keepchildren)
{
	// @todo This needs more testing.

	/* If we don't keep children then make sure that focus isn't held by a
	 * descendant. */
	bool has_focus = false;
	if (!keepchildren && node != thetree.begin()) {
		NodeReference act = focus_node;
		while (thetree.is_valid(act)) {
			if (act == node) {
				has_focus = true;
				break;
			}
			act = thetree.parent(act);
		}
	}

	if (node != thetree.begin()
			&& (node == focus_node // for keepchildren = true
				|| has_focus)) { // for keepchildren = false
		/* By folding this node and then moving focus forward we are sure that
		 * no child node of the node to be removed will get the focus. */
		node->open = false;
		MoveFocus(Container::FocusNext);

		// clear focus/input child if the move was not successful
		if (node == focus_node) {
			focus_child = NULL;
			ClearInputChild();
		}
	}

	// if we want to keep child nodes we should flatten the tree
	if (keepchildren)
		thetree.flatten(node);

	delete node->widget;
	thetree.erase(node);
}

void TreeView::DeleteChildren(const NodeReference node, bool keepchildren)
{
	TheTree::sibling_iterator i;
	
	while ((i = thetree.begin(node)) != thetree.end(node)){
		DeleteNode(i, keepchildren);
	}
}

const TreeView::NodeReference TreeView::GetSelected()
{
	return focus_node;
}

int TreeView::GetDepth(const NodeReference node)
{
	return thetree.depth(node);
}

Widget *TreeView::GetWidget(const NodeReference node)
{
	return node->widget;
}

void TreeView::SetParent(const NodeReference node, const NodeReference newparent)
{
	if (thetree.parent(node) != newparent)
		thetree.move_ontop(thetree.append_child(newparent), node);
}

void TreeView::SetStyle(const NodeReference node, Style s)
{
	node->style = s;
	/// @todo Send signal only if necessary.
	signal_redraw(*this);
}

TreeView::Style TreeView::GetStyle(const NodeReference node) const
{
	return node->style;
}

void TreeView::AddWidget(Widget& widget, int x, int y)
{
	ScrollPane::AddWidget(widget, x, y);
}

int TreeView::DrawNode(TheTree::sibling_iterator node, int top)
{
	int height = 0, j = top, oldh, depthoffset;
	TheTree::sibling_iterator i;

	depthoffset = thetree.depth(node) * 3;

	// draw the node Widget first
	if (node->widget) {
		node->widget->MoveResize(depthoffset + 3, top,
				node->widget->Width(),
				node->widget->Height());
		node->widget->Draw();
		height += node->widget->Height();
	}

	if (node->open) {
		int attrs = COLORSCHEME->GetColorPair(GetColorScheme(), "treeview", "line");
		area->attron(attrs);
		if (thetree.number_of_children(node) > 0) {
			for (j = top + 1; j < top + height; j++)
				area->mvaddstring(depthoffset, j, linestyle.V());
		}

		for (i = node.begin(); i != node.end(); i++) {
			if (i != --node.end())
				area->mvaddstring(depthoffset, top + height, linestyle.VRight());
			else
				area->mvaddstring(depthoffset, top + height, linestyle.CornerBL());

			area->mvaddstring(depthoffset + 1, top + height, linestyle.H());
			
			if (i->style == STYLE_NORMAL && thetree.number_of_children(i) > 0) {
				area->mvaddstring(depthoffset + 2, top + height, "[");
				area->mvaddstring(depthoffset + 3, top + height, i->open ? "-" : "+");
				area->mvaddstring(depthoffset + 4, top + height, "]");
			}
			else {
				area->mvaddstring(depthoffset + 2, top + height, linestyle.H());
				area->mvaddstring(depthoffset + 3, top + height, linestyle.H());
				area->mvaddstring(depthoffset + 4, top + height, linestyle.HEnd());
			}

			area->attroff(attrs);
			oldh = height;
			height += DrawNode(i, top + height);
			area->attron(attrs);

			if (i != --node.end())
				for (j = top + oldh + 1; j < top + height ; j++)
					area->mvaddstring(depthoffset, j, linestyle.V());
		}
		area->attroff(attrs);
	}

	return height;
}

void TreeView::OnChildRedraw(Widget& widget)
{
	signal_redraw(*this);
}

void TreeView::OnChildMoveResize(Widget& widget, Rect &oldsize, Rect &newsize)
{
	//TODO redraw only on height change
	signal_redraw(*this);
}

void TreeView::OnChildFocus(Widget& widget, bool focus)
{
	/* Only adjust scroll position if the widget got focus. */
	if (focus)
		AdjustScroll(widget.Left(), widget.Top());
}

void TreeView::OnActionCollapse()
{
	Collapse(focus_node);
}

void TreeView::OnActionExpand()
{
	Expand(focus_node);
}
