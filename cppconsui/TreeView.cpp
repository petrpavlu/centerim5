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
: ScrollPane(w, h, w, 0)
, linestyle(ltype)
{
	/* initialise the tree */
	TreeNode root;
	root.widget = NULL;
	root.open = true;
	thetree.set_head(root);
	focus_node = thetree.begin();

	DeclareBindables();
}

TreeView::~TreeView()
{
	DeleteNode(thetree.begin(), false);

	/// @todo Remove later.
	g_assert_cmpint(GetScrollHeight(), ==, 0);
}

void TreeView::DeclareBindables()
{
	DeclareBindable(CONTEXT_TREEVIEW, "fold-subtree",
			sigc::mem_fun(this, &TreeView::ActionCollapse),
			InputProcessor::Bindable_Normal);
	DeclareBindable(CONTEXT_TREEVIEW, "unfold-subtree",
			sigc::mem_fun(this, &TreeView::ActionExpand),
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

void TreeView::MoveResize(int newx, int newy, int neww, int newh)
{
	SetScrollWidth(neww);

	ScrollPane::MoveResize(newx, newy, neww, newh);
}

void TreeView::Draw()
{
	if (!area)
		return;

	area->erase();

	DrawNode(thetree.begin(), 0);

	// make sure that currently focused widget is visible
	if (focus_node->widget)
		MakeVisible(focus_node->widget->Left(), focus_node->widget->Top());

	ScrollPane::Draw();
}

bool TreeView::SetFocusChild(Widget& child)
{
	if (ScrollPane::SetFocusChild(child)) {
		/// @todo Speed up this algorithm.
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

Curses::Window *TreeView::GetSubPad(const Widget& child, int begin_x,
		int begin_y, int ncols, int nlines)
{
	if (!area)
		return NULL;

	int realw = area->getmaxx();

	// if height is set to negative number (autosize) then return height `1'
	if (nlines < 0)
		nlines = 1;

	/* Extend requested subpad to whole parent area or shrink requested area
	 * if necessary. */
	if (ncols < 0 || ncols > realw - begin_x)
		ncols = realw - begin_x;

	return area->subpad(begin_x, begin_y, ncols, nlines);
}

void TreeView::GetFocusChain(FocusChain& focus_chain,
		FocusChain::iterator parent)
{
	// the preorder iterator starts with the root so we must skip it
	for (TheTree::pre_order_iterator i = ++thetree.begin();
			i != thetree.end(); i++) {
		Widget *widget = i->widget;
		Container *container = dynamic_cast<Container *>(widget);

		FocusChain::pre_order_iterator iter;
		if (widget->CanFocus() && widget->IsVisible())
			iter = focus_chain.append_child(parent, widget);
		else if (container) {
			// the widget is a container so add its widgets as well
			iter = focus_chain.append_child(parent, NULL);
			container->GetFocusChain(focus_chain, iter);

			/* If this is not a focusable widget and it has no focusable
			 * children, remove it from the chain. */
			if (!focus_chain.number_of_children(iter))
				focus_chain.erase(iter);
		}

		if (!i->open)
			i.skip_children();
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

void TreeView::ActionToggleCollapsed()
{
	ToggleCollapsed(focus_node);
}

TreeView::NodeReference TreeView::InsertNode(
		const NodeReference position, Widget& widget)
{
	TreeNode node = AddNodeInit(widget);
	NodeReference iter = thetree.insert(position, node);
	AddNodeFinalize(iter);
	return iter;
}

TreeView::NodeReference TreeView::InsertNodeAfter(
		const NodeReference position, Widget& widget)
{
	TreeNode node = AddNodeInit(widget);
	NodeReference iter = thetree.insert_after(position, node);
	AddNodeFinalize(iter);
	return iter;
}

TreeView::NodeReference TreeView::PrependNode(
		const NodeReference parent, Widget& widget)
{
	TreeNode node = AddNodeInit(widget);
	NodeReference iter = thetree.prepend_child(parent, node);
	AddNodeFinalize(iter);
	return iter;
}

TreeView::NodeReference TreeView::AppendNode(
		const NodeReference parent, Widget& widget)
{
	TreeNode node = AddNodeInit(widget);
	NodeReference iter = thetree.append_child(parent, node);
	AddNodeFinalize(iter);
	return iter;
}

void TreeView::DeleteNode(const NodeReference node, bool keepchildren)
{
	/// @todo This needs more testing.

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

	int shrink = 0;
	if (node->widget) {
		int h = node->widget->Height();
		shrink += h < 0 ? 1 : h;
	}

	// if we want to keep child nodes we should flatten the tree
	if (keepchildren)
		thetree.flatten(node);
	else
		for (TheTree::pre_order_iterator i = thetree.begin(node);
				i != thetree.end(node); i++) {
			int h = i->widget->Height();
			shrink += h < 0 ? 1 : h;
			delete i->widget;
		}

	if (node->widget)
		delete node->widget;
	thetree.erase(node);

	SetScrollHeight(GetScrollHeight() - shrink);
}

void TreeView::DeleteChildren(const NodeReference node, bool keepchildren)
{
	for (SiblingIterator i = node.begin(); i != node.end(); i++)
		DeleteNode(i, keepchildren);
}

const TreeView::NodeReference TreeView::GetSelected() const
{
	return focus_node;
}

int TreeView::GetDepth(const NodeReference node) const
{
	return thetree.depth(node);
}

void TreeView::MoveBefore(const NodeReference node, const NodeReference position)
{
	if (thetree.previous_sibling(position) != node) {
		thetree.move_before(position, node);
		signal_redraw(*this);
	}
}

void TreeView::MoveAfter(const NodeReference node, const NodeReference position)
{
	if (thetree.next_sibling(position) != node) {
		thetree.move_after(position, node);
		signal_redraw(*this);
	}
}

void TreeView::SetParent(const NodeReference node, const NodeReference newparent)
{
	if (thetree.parent(node) != newparent) {
		thetree.move_ontop(thetree.append_child(newparent), node);
		signal_redraw(*this);
	}
}

void TreeView::SetStyle(const NodeReference node, Style s)
{
	if (node->style != s) {
		node->style = s;
		signal_redraw(*this);
	}
}

TreeView::Style TreeView::GetStyle(const NodeReference node) const
{
	return node->style;
}

void TreeView::AddWidget(Widget& widget, int x, int y)
{
	ScrollPane::AddWidget(widget, x, y);
}

int TreeView::DrawNode(SiblingIterator node, int top)
{
	int height = 0, j = top, oldh, depthoffset;
	SiblingIterator i;

	depthoffset = thetree.depth(node) * 3;

	// draw the node Widget first
	if (node->widget) {
		if (!node->widget->IsVisible())
			return 0;
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

		SiblingIterator last = --node.end();
		for (i = node.begin(); i != node.end(); i++) {
			if (i != last)
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

			if (i != last)
				for (j = top + oldh + 1; j < top + height ; j++)
					area->mvaddstring(depthoffset, j, linestyle.V());
		}
		area->attroff(attrs);
	}

	return height;
}

TreeView::TreeNode TreeView::AddNodeInit(Widget& widget)
{
	// make room for this widget
	int new_height = GetScrollHeight();
	if (widget.Height() < 0)
		new_height += 1;
	else
		new_height += widget.Height();
	SetScrollHeight(new_height);

	widget.SetParent(*this);

	// construct the new node
	TreeNode node;
	node.treeview = this;
	node.open = true;
	node.style = STYLE_NORMAL;
	node.widget = &widget;

	return node;
}

void TreeView::AddNodeFinalize(NodeReference& iter)
{
	if (!focus_node->widget) {
		focus_node = iter;
		iter->widget->GrabFocus();
	}

	iter->sig_redraw = iter->widget->signal_redraw.connect(sigc::mem_fun(this, &TreeView::OnChildRedraw));
	iter->sig_moveresize = iter->widget->signal_moveresize.connect(sigc::mem_fun(this, &TreeView::OnChildMoveResize));
	iter->sig_focus = iter->widget->signal_focus.connect(sigc::mem_fun(this, &TreeView::OnChildFocus));

	signal_redraw(*this);
}

void TreeView::OnChildRedraw(Widget& widget)
{
	signal_redraw(*this);
}

void TreeView::OnChildMoveResize(Widget& widget, Rect &oldsize, Rect &newsize)
{
	int old_height = oldsize.Height();
	int new_height = newsize.Height();
	if (old_height != new_height) {
		old_height = old_height < 0 ? 1 : old_height;
		new_height = new_height < 0 ? 1 : new_height;

		SetScrollHeight(GetScrollHeight() - old_height + new_height);
	}

	if (oldsize.Height() != newsize.Height()
			|| oldsize.Width() != newsize.Width())
		signal_redraw(*this);
}

void TreeView::OnChildFocus(Widget& widget, bool focus)
{
	// only adjust scroll position if the widget got focus
	if (focus)
		MakeVisible(widget.Left(), widget.Top());
}

void TreeView::ActionCollapse()
{
	Collapse(focus_node);
}

void TreeView::ActionExpand()
{
	Expand(focus_node);
}
