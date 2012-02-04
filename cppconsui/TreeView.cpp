/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2012 by CenterIM developers
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
 */

#include "TreeView.h"

namespace CppConsUI
{

TreeView::ToggleCollapseButton::ToggleCollapseButton(int w, int h,
    const char *text_)
: Button(w, h, text_)
{
}

TreeView::ToggleCollapseButton::ToggleCollapseButton(const char *text_)
: Button(text_)
{
}

void TreeView::ToggleCollapseButton::SetParent(Container& parent)
{
  TreeView *tree = dynamic_cast<TreeView*>(&parent);
  g_assert(tree);

  Button::SetParent(parent);
  signal_activate.connect(sigc::hide(sigc::mem_fun(tree,
          &TreeView::ActionToggleCollapsed)));
}

TreeView::TreeView(int w, int h)
: ScrollPane(w, h, 0, 0)
{
  // allow fast focus changing (paging) using PageUp/PageDown keys
  page_focus = true;

  // initialise the tree
  TreeNode root;
  root.treeview = this;
  root.collapsed = false;
  root.style = STYLE_NORMAL;
  root.widget = NULL;
  thetree.set_head(root);
  focus_node = thetree.begin();

  DeclareBindables();
}

TreeView::~TreeView()
{
  DeleteNode(thetree.begin(), false);
}

void TreeView::Draw()
{
  ProceedUpdateArea();
  // set virtual scroll area width
  if (screen_area)
    SetScrollWidth(screen_area->getmaxx());
  ProceedUpdateVirtualArea();

  if (!area) {
    // scrollpane will clear the scroll (real) area
    ScrollPane::Draw();
    return;
  }

  area->fill(GetColorPair("container", "background"));

  DrawNode(thetree.begin(), 0);

  // make sure that currently focused widget is visible
  if (focus_child) {
    int w = focus_child->GetWidth();
    if (w == AUTOSIZE)
      w = focus_child->GetWishWidth();
    if (w == AUTOSIZE)
      w = 1;
    int h = focus_child->GetHeight();
    if (h == AUTOSIZE)
      h = focus_child->GetWishHeight();
    if (h == AUTOSIZE)
      h = 1;

    MakeVisible(focus_child->GetLeft(), focus_child->GetTop(), w, h);
  }

  ScrollPane::DrawEx(false);
}

void TreeView::CleanFocus()
{
  ScrollPane::CleanFocus();
  focus_node = thetree.begin();
}

bool TreeView::GrabFocus()
{
  for (TheTree::pre_order_iterator i = ++thetree.begin();
      i != thetree.end(); i++)
    if (i->widget->GrabFocus())
      return true;
  return false;
}

bool TreeView::IsWidgetVisible(const Widget& child) const
{
  if (!parent || !visible)
    return false;

  NodeReference node = FindNode(child);
  if (!IsNodeVisible(node))
    return false;

  return parent->IsWidgetVisible(*this);
}

bool TreeView::SetFocusChild(Widget& child)
{
  NodeReference node = FindNode(child);
  if (!IsNodeVisible(node))
    return false;

  bool res = ScrollPane::SetFocusChild(child);
  focus_node = node;
  return res;
}

void TreeView::GetFocusChain(FocusChain& focus_chain,
    FocusChain::iterator parent)
{
  /* It's possible that the predecessor of focused node was just made
   * invisible and MoveFocus() is called so other widget can take the focus.
   * In this case we find top invisible predecessor of focused node and
   * later the focused node is placed into the focus chain when this
   * predecessor is reached. */
  NodeReference act = focus_node;
  NodeReference top = thetree.begin();
  while (act != thetree.begin()) {
    if (!act->widget->IsVisible())
      top = act;
    act = thetree.parent(act);
  }

  // the preorder iterator starts with the root so we must skip it
  for (TheTree::pre_order_iterator i = ++thetree.begin();
      i != thetree.end(); i++) {
    Widget *widget = i->widget;
    Container *container = dynamic_cast<Container*>(widget);

    if (container && container->IsVisible()) {
      // the widget is a container so add its widgets as well
      FocusChain::pre_order_iterator iter = focus_chain.append_child(parent,
          container);
      container->GetFocusChain(focus_chain, iter);

      /* If this is not a focusable widget and it has no focusable
       * children, remove it from the chain. */
      if (!focus_chain.number_of_children(iter))
        focus_chain.erase(iter);
    }
    else if (widget->CanFocus() && widget->IsVisible()) {
      // widget can be focused
      focus_chain.append_child(parent, widget);
    }
    else if (i == top) {
      // focused node is in subtree of this node
      focus_chain.append_child(parent, focus_child);
    }

    if (i->collapsed || !widget->IsVisible())
      i.skip_children();
  }
}

Curses::Window *TreeView::GetSubPad(const Widget& child, int begin_x,
    int begin_y, int ncols, int nlines)
{
  // if height is set to autosize then use widget's wish height
  if (nlines == AUTOSIZE)
    nlines = child.GetWishHeight();
  if (nlines == AUTOSIZE)
    nlines = 1;

  return ScrollPane::GetSubPad(child, begin_x, begin_y, ncols, nlines);
}

void TreeView::SetCollapsed(NodeReference node, bool collapsed)
{
  g_assert(node->treeview == this);

  if (node->collapsed == collapsed)
    return;

  node->collapsed = collapsed;
  FixFocus();
  Redraw();
}

void TreeView::ToggleCollapsed(NodeReference node)
{
  g_assert(node->treeview == this);

  node->collapsed = !node->collapsed;
  FixFocus();
  Redraw();
}

void TreeView::ActionToggleCollapsed()
{
  ToggleCollapsed(focus_node);
}

TreeView::NodeReference TreeView::InsertNode(NodeReference position,
    Widget& widget)
{
  g_assert(position->treeview == this);

  TreeNode node = AddNode(widget);
  NodeReference iter = thetree.insert(position, node);
  AddWidget(widget, 0, 0);
  return iter;
}

TreeView::NodeReference TreeView::InsertNodeAfter(NodeReference position,
    Widget& widget)
{
  g_assert(position->treeview == this);

  TreeNode node = AddNode(widget);
  NodeReference iter = thetree.insert_after(position, node);
  AddWidget(widget, 0, 0);
  return iter;
}

TreeView::NodeReference TreeView::PrependNode(NodeReference parent,
    Widget& widget)
{
  g_assert(parent->treeview == this);

  TreeNode node = AddNode(widget);
  NodeReference iter = thetree.prepend_child(parent, node);
  AddWidget(widget, 0, 0);
  return iter;
}

TreeView::NodeReference TreeView::AppendNode(NodeReference parent,
    Widget& widget)
{
  g_assert(parent->treeview == this);

  TreeNode node = AddNode(widget);
  NodeReference iter = thetree.append_child(parent, node);
  AddWidget(widget, 0, 0);
  return iter;
}

void TreeView::DeleteNode(NodeReference node, bool keepchildren)
{
  g_assert(node->treeview == this);

  /// @todo This needs more testing.

  // if we want to keep child nodes we should flatten the tree
  if (keepchildren)
    thetree.flatten(node);

  int shrink = 0;
  if (node->widget) {
    int h = node->widget->GetHeight();
    if (h == AUTOSIZE)
      h = node->widget->GetWishHeight();
    if (h == AUTOSIZE)
      h = 1;
    shrink += h;
  }

  for (TheTree::pre_order_iterator i = thetree.begin(node);
      i != thetree.end(node); i++) {
    int h = i->widget->GetHeight();
    if (h == AUTOSIZE)
      h = i->widget->GetWishHeight();
    if (h == AUTOSIZE)
      h = 1;
    shrink += h;
    RemoveWidget(*i->widget);
  }

  if (node->widget)
    RemoveWidget(*node->widget);

  thetree.erase(node);
  SetScrollHeight(GetScrollHeight() - shrink);
  Redraw();
}

void TreeView::DeleteNodeChildren(NodeReference node, bool keepchildren)
{
  g_assert(node->treeview == this);

  SiblingIterator i;
  while ((i = node.begin()) != node.end())
    DeleteNode(i, keepchildren);
}

TreeView::NodeReference TreeView::GetSelectedNode() const
{
  return focus_node;
}

int TreeView::GetNodeDepth(NodeReference node) const
{
  g_assert(node->treeview == this);

  return thetree.depth(node);
}

void TreeView::MoveNodeBefore(NodeReference node, NodeReference position)
{
  g_assert(node->treeview == this);
  g_assert(position->treeview == this);

  if (thetree.previous_sibling(position) != node) {
    thetree.move_before(position, node);
    FixFocus();
    Redraw();
  }
}

void TreeView::MoveNodeAfter(NodeReference node, NodeReference position)
{
  g_assert(node->treeview == this);
  g_assert(position->treeview == this);

  if (thetree.next_sibling(position) != node) {
    thetree.move_after(position, node);
    FixFocus();
    Redraw();
  }
}

void TreeView::SetNodeParent(NodeReference node, NodeReference newparent)
{
  g_assert(node->treeview == this);
  g_assert(newparent->treeview == this);

  if (thetree.parent(node) != newparent) {
    thetree.move_ontop(thetree.append_child(newparent), node);
    FixFocus();
    Redraw();
  }
}

void TreeView::SetNodeStyle(NodeReference node, Style s)
{
  g_assert(node->treeview == this);

  if (node->style != s) {
    node->style = s;
    Redraw();
  }
}

TreeView::Style TreeView::GetNodeStyle(NodeReference node) const
{
  g_assert(node->treeview == this);

  return node->style;
}

void TreeView::AddWidget(Widget& widget, int x, int y)
{
  ScrollPane::AddWidget(widget, x, y);
}

void TreeView::RemoveWidget(Widget& widget)
{
  ScrollPane::RemoveWidget(widget);
}

void TreeView::MoveWidgetBefore(Widget& widget, Widget& position)
{
  ScrollPane::MoveWidgetBefore(widget, position);
}

void TreeView::MoveWidgetAfter(Widget& widget, Widget& position)
{
  ScrollPane::MoveWidgetAfter(widget, position);
}

void TreeView::Clear()
{
  ScrollPane::Clear();
}

int TreeView::DrawNode(SiblingIterator node, int top)
{
  int height = 0, j;
  SiblingIterator i;
  int depthoffset = thetree.depth(node) * 2;
  int realw = area->getmaxx();

  // draw the node Widget first
  if (node->widget) {
    if (!node->widget->IsVisible())
      return 0;
    if (node->style == STYLE_NORMAL && IsNodeOpenable(node))
      node->widget->Move(depthoffset + 3, top);
    else
      node->widget->Move(depthoffset + 1, top);
    node->widget->Draw();
    int h = node->widget->GetHeight();
    if (h == AUTOSIZE)
      h = node->widget->GetWishHeight();
    if (h == AUTOSIZE)
      h = 1;
    height += h;
  }

  if (!node->collapsed && IsNodeOpenable(node)) {
    int attrs = GetColorPair("treeview", "line");
    area->attron(attrs);
    if (depthoffset < realw)
      for (j = top + 1; j < top + height; j++)
        area->mvaddlinechar(depthoffset, j, Curses::LINE_VLINE);

    /* Note: it would be better to start from end towards begin but for some
     * reason it doesn't seem to work. */
    SiblingIterator last = node.begin();
    for (i = node.begin(); i != node.end(); i++) {
      if (!i->widget)
        continue;
      int h = i->widget->GetHeight();
      if (h == AUTOSIZE)
        h = i->widget->GetWishHeight();
      if (h == AUTOSIZE)
        h = 1;
      if (h && i->widget->IsVisible())
        last = i;
    }
    SiblingIterator end = last;
    end++;
    for (i = node.begin(); i != end; i++) {
      if (depthoffset < realw) {
        if (i != last)
          area->mvaddlinechar(depthoffset, top + height, Curses::LINE_LTEE);
        else
          area->mvaddlinechar(depthoffset, top + height, Curses::LINE_LLCORNER);
      }

      /*
      if (depthoffset + 1 < realw)
        area->mvaddlinechar(depthoffset + 1, top + height, Curses::LINE_HLINE);
        */

      if (i->style == STYLE_NORMAL && IsNodeOpenable(i)) {
        if (depthoffset + 1 < realw)
          area->mvaddstring(depthoffset + 1, top + height, "[");
        if (depthoffset + 2 < realw)
          area->mvaddstring(depthoffset + 2, top + height, i->collapsed ? "+" : "-");
        if (depthoffset + 3 < realw)
          area->mvaddstring(depthoffset + 3, top + height, "]");
      }
      else if (depthoffset + 1 < realw)
        area->mvaddlinechar(depthoffset + 1, top + height, Curses::LINE_HLINE);

      area->attroff(attrs);
      int oldh = height;
      height += DrawNode(i, top + height);
      area->attron(attrs);

      if (i != last && depthoffset < realw)
        for (j = top + oldh + 1; j < top + height ; j++)
          area->mvaddlinechar(depthoffset, j, Curses::LINE_VLINE);
    }
    area->attroff(attrs);
  }

  return height;
}

TreeView::TreeNode TreeView::AddNode(Widget& widget)
{
  // make room for this widget
  int new_height = GetScrollHeight();
  int h = widget.GetHeight();
  if (h == AUTOSIZE)
    h = widget.GetWishHeight();
  if (h == AUTOSIZE)
    h = 1;
  new_height += h;
  SetScrollHeight(new_height);

  // construct the new node
  TreeNode node;
  node.treeview = this;
  node.collapsed = false;
  node.style = STYLE_NORMAL;
  node.widget = &widget;

  return node;
}

void TreeView::FixFocus()
{
  /* This function is called when a widget tree is reorganized (a node was
   * moved in another position in the tree). In this case it's possible that
   * there can be revealed a widget that could grab the focus (if there is no
   * focused widget yet) or it's also possible that currently focused widget
   * was hidden by this reorganization (then the focus has to be handled to
   * another widget). */

  UpdateFocusChain();

  Container *t = GetTopContainer();
  Widget *focus = t->GetFocusWidget();
  if (!focus) {
    // try to grab the focus
    t->MoveFocus(FOCUS_DOWN);
  }
  else if (!focus->IsVisibleRecursive()) {
    // move focus
    t->MoveFocus(FOCUS_DOWN);
  }
}

TreeView::NodeReference TreeView::FindNode(const Widget& child) const
{
  /// @todo Speed up this algorithm.
  TheTree::pre_order_iterator i;
  for (i = thetree.begin(); i != thetree.end(); i++)
    if (i->widget == &child)
      break;
  g_assert(i != thetree.end());
  return i;
}

bool TreeView::IsNodeOpenable(SiblingIterator& node) const
{
  for (SiblingIterator i = node.begin(); i != node.end(); i++) {
    if (!i->widget)
      continue;
    int h = i->widget->GetHeight();
    if (h == AUTOSIZE)
      h = i->widget->GetWishHeight();
    if (h == AUTOSIZE)
      h = 1;
    if (h && i->widget->IsVisible())
      return true;
  }
  return false;
}

bool TreeView::IsNodeVisible(NodeReference& node) const
{
  // node is visible if all predecessors are visible and open
  NodeReference act = node;
  bool first = true;
  while (act != thetree.begin()) {
    if (!act->widget->IsVisible() || (!first && act->collapsed))
      return false;
    first = false;
    act = thetree.parent(act);
  }
  return true;
}

void TreeView::OnChildMoveResize(Widget& activator, const Rect &oldsize,
    const Rect &newsize)
{
  int old_height = oldsize.GetHeight();
  int new_height = newsize.GetHeight();
  if (old_height != new_height) {
    if (old_height == AUTOSIZE)
      old_height = activator.GetWishHeight();
    if (old_height == AUTOSIZE)
      old_height = 1;
    if (new_height == AUTOSIZE)
      new_height = activator.GetWishHeight();
    if (new_height == AUTOSIZE)
      new_height = 1;

    SetScrollHeight(GetScrollHeight() - old_height + new_height);
  }
}

void TreeView::OnChildWishSizeChange(Widget& activator, const Size& oldsize,
    const Size& newsize)
{
  if (activator.GetHeight() != AUTOSIZE)
    return;

  int old_height = oldsize.GetHeight();
  int new_height = newsize.GetHeight();

  if (old_height != new_height)
    SetScrollHeight(GetScrollHeight() - old_height + new_height);
}

void TreeView::ActionCollapse()
{
  SetCollapsed(focus_node, true);
}

void TreeView::ActionExpand()
{
  SetCollapsed(focus_node, false);
}

void TreeView::DeclareBindables()
{
  DeclareBindable("treeview", "fold-subtree", sigc::mem_fun(this,
        &TreeView::ActionCollapse), InputProcessor::BINDABLE_NORMAL);
  DeclareBindable("treeview", "unfold-subtree", sigc::mem_fun(this,
        &TreeView::ActionExpand), InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 tw=78 expandtab : */
