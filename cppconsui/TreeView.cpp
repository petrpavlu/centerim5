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

#include "Keys.h"

#include "gettext.h"

#define CONTEXT_TREEVIEW "treeview"

TreeView::TreeView(int w, int h, LineStyle::Type ltype)
: ScrollPane(w, h, 0, 0)
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
}

void TreeView::DeclareBindables()
{
  DeclareBindable(CONTEXT_TREEVIEW, "fold-subtree",
      sigc::mem_fun(this, &TreeView::ActionCollapse),
      InputProcessor::BINDABLE_NORMAL);
  DeclareBindable(CONTEXT_TREEVIEW, "unfold-subtree",
      sigc::mem_fun(this, &TreeView::ActionExpand),
      InputProcessor::BINDABLE_NORMAL);
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

void TreeView::UpdateArea()
{
  ScrollPane::UpdateArea();

  // set virtual scroll area width
  if (scrollarea)
    SetScrollWidth(scrollarea->getmaxx());
}

void TreeView::Draw()
{
  if (!area) {
    // scrollpane will clear the scroll (real) area
    ScrollPane::Draw();
    return;
  }

  area->fill(GetColorPair("container", "background"));

  DrawNode(thetree.begin(), 0);

  // make sure that currently focused widget is visible
  if (focus_child)
    MakeVisible(focus_child->Left(), focus_child->Top());

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
    Container *container = dynamic_cast<Container *>(widget);

    if (container && container->IsVisible()) {
      // the widget is a container so add its widgets as well
      FocusChain::pre_order_iterator iter
        = focus_chain.append_child(parent, NULL);
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

    if (!i->open || !widget->IsVisible())
      i.skip_children();
  }
}

bool TreeView::SetActive(int i)
{
  if (i < 0 || (int) thetree.size() - 1 <= i)
    i = 0;

  TheTree::pre_order_iterator j;
  for (j = ++thetree.begin(); i > 0 && j != thetree.end(); j++, i--)
    ;
  if (j != thetree.end() && j->widget)
    if (j->widget->GrabFocus())
      return true;
  return false;
}

int TreeView::GetActive() const
{
  TheTree::pre_order_iterator j;
  int i;

  for (j = ++thetree.begin(), i = 0; j != thetree.end(); j++, i++)
    if (j->widget && j->widget->HasFocus())
      return i;

  return -1;
}

Curses::Window *TreeView::GetSubPad(const Widget& child, int begin_x,
    int begin_y, int ncols, int nlines)
{
  // if height is set to autosize then return height `1'
  if (nlines == AUTOSIZE)
    nlines = 1;

  return ScrollPane::GetSubPad(child, begin_x, begin_y, ncols, nlines);
}

void TreeView::Collapse(const NodeReference node)
{
  if (node->open) {
    node->open = false;
    FocusFix();
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

  // if we want to keep child nodes we should flatten the tree
  if (keepchildren)
    thetree.flatten(node);

  if (node->widget) {
    // focus gets moved if this hides the focused node
    node->widget->SetVisibility(false);
  }

  int shrink = 0;
  if (node->widget) {
    int h = node->widget->Height();
    if (h == AUTOSIZE)
      h = 1;
    shrink += h;
  }

  for (TheTree::pre_order_iterator i = thetree.begin(node);
      i != thetree.end(node); i++) {
    int h = i->widget->Height();
    if (h == AUTOSIZE)
      h = 1;
    shrink += h;
    delete i->widget;
  }

  if (node->widget)
    delete node->widget;

  thetree.erase(node);
  SetScrollHeight(GetScrollHeight() - shrink);
  signal_redraw(*this);
}

void TreeView::DeleteChildren(const NodeReference node, bool keepchildren)
{
  SiblingIterator i;
  while ((i = node.begin()) != node.end())
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
    FocusFix();
    signal_redraw(*this);
  }
}

void TreeView::MoveAfter(const NodeReference node, const NodeReference position)
{
  if (thetree.next_sibling(position) != node) {
    thetree.move_after(position, node);
    FocusFix();
    signal_redraw(*this);
  }
}

void TreeView::SetParent(const NodeReference node, const NodeReference newparent)
{
  if (thetree.parent(node) != newparent) {
    thetree.move_ontop(thetree.append_child(newparent), node);
    FocusFix();
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

void TreeView::RemoveWidget(Widget& widget)
{
  ScrollPane::RemoveWidget(widget);
}

void TreeView::Clear()
{
  ScrollPane::Clear();
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

  if (node->open && IsNodeOpenable(node)) {
    int attrs = GetColorPair("treeview", "line");
    area->attron(attrs);
    for (j = top + 1; j < top + height; j++)
      area->mvaddstring(depthoffset, j, linestyle.V());

    /* Note: it would be better to start from end towards begin but for some
     * reason it doesn't seem to work. */
    SiblingIterator last = node.begin();
    for (i = node.begin(); i != node.end(); i++)
      if (i->widget && i->widget->Height() && i->widget->IsVisible())
        last = i;
    SiblingIterator end = last;
    end++;
    for (i = node.begin(); i != end; i++) {
      if (i != last)
        area->mvaddstring(depthoffset, top + height, linestyle.VRight());
      else
        area->mvaddstring(depthoffset, top + height, linestyle.CornerBL());

      area->mvaddstring(depthoffset + 1, top + height, linestyle.H());

      if (i->style == STYLE_NORMAL && IsNodeOpenable(i)) {
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
  if (widget.Height() == AUTOSIZE)
    new_height += 1;
  else
    new_height += widget.Height();
  SetScrollHeight(new_height);

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
  Widget *w = iter->widget;

  w->SetParent(*this);

  iter->sig_redraw = w->signal_redraw.connect(sigc::mem_fun(this,
        &TreeView::OnChildRedraw));
  iter->sig_moveresize = w->signal_moveresize.connect(sigc::mem_fun(this,
        &TreeView::OnChildMoveResize));

  signal_redraw(*this);
}

void TreeView::FocusFix()
{
  /* This function is called when a widget tree is reorganized (a node was
   * moved in another position in the tree). In this case it's possible that
   * there can be revealed a widget that could grab the focus (if there is no
   * focused widget yet) or it's also possible that currently focused widget
   * was hidden by this reorganization (then the focus has to be handled to
   * another widget). */

  Container *t = GetTopContainer();
  Widget *focus = t->GetFocusWidget();
  if (!focus) {
    // try to grab the focus
    t->MoveFocus(Container::FOCUS_NEXT);
  }
  else if (!focus->IsVisibleRecursive()) {
    // move focus
    t->MoveFocus(Container::FOCUS_NEXT);
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

bool TreeView::IsNodeOpenable(const SiblingIterator& node) const
{
  for (SiblingIterator i = node.begin(); i != node.end(); i++)
    if (i->widget && i->widget->Height() && i->widget->IsVisible())
      return true;
  return false;
}

bool TreeView::IsNodeVisible(const NodeReference& node) const
{
  // node is visible if all predecessors are visible and open
  NodeReference act = node;
  bool first = true;
  while (act != thetree.begin()) {
    if (!act->widget->IsVisible() || (!first && !act->open))
      return false;
    first = false;
    act = thetree.parent(act);
  }
  return true;
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
    if (old_height == AUTOSIZE)
      old_height = 1;
    if (new_height == AUTOSIZE)
      new_height = 1;

    SetScrollHeight(GetScrollHeight() - old_height + new_height);
  }
}

void TreeView::ActionCollapse()
{
  Collapse(focus_node);
}

void TreeView::ActionExpand()
{
  Expand(focus_node);
}
