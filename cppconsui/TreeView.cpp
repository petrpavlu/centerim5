/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2013 by CenterIM developers
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

#include <cassert>

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

void TreeView::ToggleCollapseButton::setParent(Container& parent)
{
  TreeView *tree = dynamic_cast<TreeView*>(&parent);
  assert(tree);

  Button::setParent(parent);
  signal_activate.connect(sigc::hide(sigc::mem_fun(tree,
          &TreeView::actionToggleCollapsed)));
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

  declareBindables();
}

TreeView::~TreeView()
{
  clear();
}

void TreeView::draw()
{
  proceedUpdateArea();
  // set virtual scroll area width
  if (screen_area)
    setScrollWidth(screen_area->getmaxx());
  proceedUpdateVirtualArea();

  if (!area) {
    // scrollpane will clear the scroll (real) area
    ScrollPane::draw();
    return;
  }

  area->fill(getColorPair("container", "background"));

  drawNode(thetree.begin(), 0);

  // make sure that currently focused widget is visible
  if (focus_child) {
    int w = focus_child->getWidth();
    if (w == AUTOSIZE)
      w = focus_child->getWishWidth();
    if (w == AUTOSIZE)
      w = 1;
    int h = focus_child->getHeight();
    if (h == AUTOSIZE)
      h = focus_child->getWishHeight();
    if (h == AUTOSIZE)
      h = 1;

    makeVisible(focus_child->getLeft(), focus_child->getTop(), w, h);
  }

  ScrollPane::drawEx(false);
}

void TreeView::cleanFocus()
{
  ScrollPane::cleanFocus();
  focus_node = thetree.begin();
}

bool TreeView::grabFocus()
{
  for (TheTree::pre_order_iterator i = ++thetree.begin();
      i != thetree.end(); i++)
    if (i->widget->grabFocus())
      return true;
  return false;
}

void TreeView::clear()
{
  TheTree::pre_order_iterator root = thetree.begin();
  while (root.number_of_children())
    deleteNode(++thetree.begin(), false);

  // stay sane
  assert(children.empty());
  assert(!getScrollHeight());
}

bool TreeView::isWidgetVisible(const Widget& child) const
{
  if (!parent || !visible)
    return false;

  NodeReference node = findNode(child);
  if (!isNodeVisible(node))
    return false;

  return parent->isWidgetVisible(*this);
}

bool TreeView::setFocusChild(Widget& child)
{
  NodeReference node = findNode(child);
  if (!isNodeVisible(node))
    return false;

  bool res = ScrollPane::setFocusChild(child);
  focus_node = node;
  return res;
}

void TreeView::getFocusChain(FocusChain& focus_chain,
    FocusChain::iterator parent)
{
  /* It's possible that the predecessor of focused node was just made
   * invisible and moveFocus() is called so other widget can take the focus.
   * In this case we find top invisible predecessor of focused node and
   * later the focused node is placed into the focus chain when this
   * predecessor is reached. */
  NodeReference act = focus_node;
  NodeReference top = thetree.begin();
  while (act != thetree.begin()) {
    if (!act->widget->isVisible())
      top = act;
    act = thetree.parent(act);
  }

  // the preorder iterator starts with the root so we must skip it
  for (TheTree::pre_order_iterator i = ++thetree.begin();
      i != thetree.end(); i++) {
    Widget *widget = i->widget;
    Container *container = dynamic_cast<Container*>(widget);

    if (container && container->isVisible()) {
      // the widget is a container so add its widgets as well
      FocusChain::pre_order_iterator iter = focus_chain.append_child(parent,
          container);
      container->getFocusChain(focus_chain, iter);

      /* If this is not a focusable widget and it has no focusable
       * children, remove it from the chain. */
      if (!focus_chain.number_of_children(iter))
        focus_chain.erase(iter);
    }
    else if (widget->canFocus() && widget->isVisible()) {
      // widget can be focused
      focus_chain.append_child(parent, widget);
    }
    else if (i == top) {
      /* This node is the focused node or the focused node is in a subtree of
       * this node. */

      Container *focus_cont = dynamic_cast<Container*>(focus_child);
      if (focus_cont) {
        /* The focused node is actually a Container. First add the Container,
         * then the focused widget. */
        FocusChain::pre_order_iterator iter = focus_chain.append_child(parent,
            focus_cont);
        focus_chain.append_child(iter, focus_cont->getFocusWidget());
      }
      else
        focus_chain.append_child(parent, focus_child);
    }

    if (i->collapsed || !widget->isVisible())
      i.skip_children();
  }
}

Curses::Window *TreeView::getSubPad(const Widget& child, int begin_x,
    int begin_y, int ncols, int nlines)
{
  // if height is set to autosize then use widget's wish height
  if (nlines == AUTOSIZE)
    nlines = child.getWishHeight();
  if (nlines == AUTOSIZE)
    nlines = 1;

  return ScrollPane::getSubPad(child, begin_x, begin_y, ncols, nlines);
}

void TreeView::setCollapsed(NodeReference node, bool collapsed)
{
  assert(node->treeview == this);

  if (node->collapsed == collapsed)
    return;

  node->collapsed = collapsed;
  fixFocus();
  redraw();
}

void TreeView::toggleCollapsed(NodeReference node)
{
  assert(node->treeview == this);

  node->collapsed = !node->collapsed;
  fixFocus();
  redraw();
}

void TreeView::actionToggleCollapsed()
{
  toggleCollapsed(focus_node);
}

TreeView::NodeReference TreeView::insertNode(NodeReference position,
    Widget& widget)
{
  assert(position->treeview == this);

  TreeNode node = addNode(widget);
  NodeReference iter = thetree.insert(position, node);
  addWidget(widget, 0, 0);
  return iter;
}

TreeView::NodeReference TreeView::insertNodeAfter(NodeReference position,
    Widget& widget)
{
  assert(position->treeview == this);

  TreeNode node = addNode(widget);
  NodeReference iter = thetree.insert_after(position, node);
  addWidget(widget, 0, 0);
  return iter;
}

TreeView::NodeReference TreeView::prependNode(NodeReference parent,
    Widget& widget)
{
  assert(parent->treeview == this);

  TreeNode node = addNode(widget);
  NodeReference iter = thetree.prepend_child(parent, node);
  addWidget(widget, 0, 0);
  return iter;
}

TreeView::NodeReference TreeView::appendNode(NodeReference parent,
    Widget& widget)
{
  assert(parent->treeview == this);

  TreeNode node = addNode(widget);
  NodeReference iter = thetree.append_child(parent, node);
  addWidget(widget, 0, 0);
  return iter;
}

void TreeView::deleteNode(NodeReference node, bool keepchildren)
{
  assert(node->treeview == this);

  // if we want to keep child nodes we should flatten the tree
  if (keepchildren)
    thetree.flatten(node);

  int shrink = 0;
  if (node->widget) {
    int h = node->widget->getHeight();
    if (h == AUTOSIZE)
      h = node->widget->getWishHeight();
    if (h == AUTOSIZE)
      h = 1;
    shrink += h;
  }

  while (thetree.number_of_children(node)) {
    TheTree::pre_order_iterator i = thetree.begin_leaf(node);
    int h = i->widget->getHeight();
    if (h == AUTOSIZE)
      h = i->widget->getWishHeight();
    if (h == AUTOSIZE)
      h = 1;
    shrink += h;

    // remove the widget and instantly remove it from the tree
    removeWidget(*i->widget);
    thetree.erase(i);
  }

  if (node->widget)
    removeWidget(*node->widget);

  thetree.erase(node);
  setScrollHeight(getScrollHeight() - shrink);
  redraw();
}

void TreeView::deleteNodeChildren(NodeReference node, bool keepchildren)
{
  assert(node->treeview == this);

  SiblingIterator i;
  while ((i = node.begin()) != node.end())
    deleteNode(i, keepchildren);
}

TreeView::NodeReference TreeView::getSelectedNode() const
{
  return focus_node;
}

int TreeView::getNodeDepth(NodeReference node) const
{
  assert(node->treeview == this);

  return thetree.depth(node);
}

void TreeView::moveNodeBefore(NodeReference node, NodeReference position)
{
  assert(node->treeview == this);
  assert(position->treeview == this);

  if (thetree.previous_sibling(position) != node) {
    thetree.move_before(position, node);
    fixFocus();
    redraw();
  }
}

void TreeView::moveNodeAfter(NodeReference node, NodeReference position)
{
  assert(node->treeview == this);
  assert(position->treeview == this);

  if (thetree.next_sibling(position) != node) {
    thetree.move_after(position, node);
    fixFocus();
    redraw();
  }
}

void TreeView::setNodeParent(NodeReference node, NodeReference newparent)
{
  assert(node->treeview == this);
  assert(newparent->treeview == this);

  if (thetree.parent(node) != newparent) {
    thetree.move_ontop(thetree.append_child(newparent), node);
    fixFocus();
    redraw();
  }
}

void TreeView::setNodeStyle(NodeReference node, Style s)
{
  assert(node->treeview == this);

  if (node->style != s) {
    node->style = s;
    redraw();
  }
}

TreeView::Style TreeView::getNodeStyle(NodeReference node) const
{
  assert(node->treeview == this);

  return node->style;
}

int TreeView::drawNode(SiblingIterator node, int top)
{
  int height = 0, j;
  SiblingIterator i;
  int depthoffset = thetree.depth(node) * 2;
  int realw = area->getmaxx();

  // draw the node Widget first
  if (node->widget) {
    if (!node->widget->isVisible())
      return 0;
    if (node->style == STYLE_NORMAL && isNodeOpenable(node))
      node->widget->move(depthoffset + 3, top);
    else
      node->widget->move(depthoffset + 1, top);
    node->widget->draw();
    int h = node->widget->getHeight();
    if (h == AUTOSIZE)
      h = node->widget->getWishHeight();
    if (h == AUTOSIZE)
      h = 1;
    height += h;
  }

  if (!node->collapsed && isNodeOpenable(node)) {
    int attrs = getColorPair("treeview", "line");
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
      int h = i->widget->getHeight();
      if (h == AUTOSIZE)
        h = i->widget->getWishHeight();
      if (h == AUTOSIZE)
        h = 1;
      if (h && i->widget->isVisible())
        last = i;
    }
    SiblingIterator end = last;
    end++;
    for (i = node.begin(); i != end; i++) {
      if (depthoffset < realw) {
        if (i != last)
          area->mvaddlinechar(depthoffset, top + height, Curses::LINE_LTEE);
        else
          area->mvaddlinechar(depthoffset, top + height,
              Curses::LINE_LLCORNER);
      }

      if (i->style == STYLE_NORMAL && isNodeOpenable(i)) {
        if (depthoffset + 1 < realw)
          area->mvaddstring(depthoffset + 1, top + height, "[");
        if (depthoffset + 2 < realw)
          area->mvaddstring(depthoffset + 2, top + height,
              i->collapsed ? "+" : "-");
        if (depthoffset + 3 < realw)
          area->mvaddstring(depthoffset + 3, top + height, "]");
      }
      else if (depthoffset + 1 < realw)
        area->mvaddlinechar(depthoffset + 1, top + height,
            Curses::LINE_HLINE);

      area->attroff(attrs);
      int oldh = height;
      height += drawNode(i, top + height);
      area->attron(attrs);

      if (i != last && depthoffset < realw)
        for (j = top + oldh + 1; j < top + height ; j++)
          area->mvaddlinechar(depthoffset, j, Curses::LINE_VLINE);
    }
    area->attroff(attrs);
  }

  return height;
}

TreeView::TreeNode TreeView::addNode(Widget& widget)
{
  // make room for this widget
  int new_height = getScrollHeight();
  int h = widget.getHeight();
  if (h == AUTOSIZE)
    h = widget.getWishHeight();
  if (h == AUTOSIZE)
    h = 1;
  new_height += h;
  setScrollHeight(new_height);

  // construct the new node
  TreeNode node;
  node.treeview = this;
  node.collapsed = false;
  node.style = STYLE_NORMAL;
  node.widget = &widget;

  return node;
}

void TreeView::fixFocus()
{
  /* This function is called when a widget tree is reorganized (a node was
   * moved in another position in the tree). In this case it's possible that
   * there can be revealed a widget that could grab the focus (if there is no
   * focused widget yet) or it's also possible that currently focused widget
   * was hidden by this reorganization (then the focus has to be handled to
   * another widget). */

  updateFocusChain();

  Container *t = getTopContainer();
  Widget *focus = t->getFocusWidget();
  if (!focus) {
    // try to grab the focus
    t->moveFocus(FOCUS_DOWN);
  }
  else if (!focus->isVisibleRecursive()) {
    // move focus
    t->moveFocus(FOCUS_DOWN);
  }
}

TreeView::NodeReference TreeView::findNode(const Widget& child) const
{
  /// @todo Speed up this algorithm.
  TheTree::pre_order_iterator i;
  for (i = thetree.begin(); i != thetree.end(); i++)
    if (i->widget == &child)
      break;
  assert(i != thetree.end());
  return i;
}

bool TreeView::isNodeOpenable(SiblingIterator& node) const
{
  for (SiblingIterator i = node.begin(); i != node.end(); i++) {
    if (!i->widget)
      continue;
    int h = i->widget->getHeight();
    if (h == AUTOSIZE)
      h = i->widget->getWishHeight();
    if (h == AUTOSIZE)
      h = 1;
    if (h && i->widget->isVisible())
      return true;
  }
  return false;
}

bool TreeView::isNodeVisible(NodeReference& node) const
{
  // node is visible if all predecessors are visible and open
  NodeReference act = node;
  bool first = true;
  while (act != thetree.begin()) {
    if (!act->widget->isVisible() || (!first && act->collapsed))
      return false;
    first = false;
    act = thetree.parent(act);
  }
  return true;
}

void TreeView::onChildMoveResize(Widget& activator, const Rect &oldsize,
    const Rect &newsize)
{
  int old_height = oldsize.getHeight();
  int new_height = newsize.getHeight();
  if (old_height != new_height) {
    if (old_height == AUTOSIZE)
      old_height = activator.getWishHeight();
    if (old_height == AUTOSIZE)
      old_height = 1;
    if (new_height == AUTOSIZE)
      new_height = activator.getWishHeight();
    if (new_height == AUTOSIZE)
      new_height = 1;

    setScrollHeight(getScrollHeight() - old_height + new_height);
  }
}

void TreeView::onChildWishSizeChange(Widget& activator, const Size& oldsize,
    const Size& newsize)
{
  if (activator.getHeight() != AUTOSIZE)
    return;

  int old_height = oldsize.getHeight();
  int new_height = newsize.getHeight();

  if (old_height != new_height)
    setScrollHeight(getScrollHeight() - old_height + new_height);
}

void TreeView::actionCollapse()
{
  setCollapsed(focus_node, true);
}

void TreeView::actionExpand()
{
  setCollapsed(focus_node, false);
}

void TreeView::declareBindables()
{
  declareBindable("treeview", "fold-subtree", sigc::mem_fun(this,
        &TreeView::actionCollapse), InputProcessor::BINDABLE_NORMAL);
  declareBindable("treeview", "unfold-subtree", sigc::mem_fun(this,
        &TreeView::actionExpand), InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
