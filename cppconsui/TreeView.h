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

#ifndef __TREEVIEW_H__
#define __TREEVIEW_H__

#include "Button.h"
#include "ScrollPane.h"

#include "tree.hh"

namespace CppConsUI
{

class TreeView
: public ScrollPane
{
protected:
  class TreeNode;

public:
  class ToggleCollapseButton
  : public Button
  {
  public:
    ToggleCollapseButton(int w, int h, const char *text_ = NULL);
    explicit ToggleCollapseButton(const char *text_ = NULL);
    virtual ~ToggleCollapseButton() {}

    // Widget
    virtual void setParent(Container& parent);

  protected:

  private:
    CONSUI_DISABLE_COPY(ToggleCollapseButton);
  };

  /**
   * Node drawing style.
   */
  enum Style {
    STYLE_NORMAL, ///< Draw "[+]" if the node is collapsed.
    STYLE_VOID ///< Don't draw any extra information.
  };

  typedef tree<TreeNode> TheTree;
  typedef TheTree::pre_order_iterator NodeReference;
  typedef TheTree::sibling_iterator SiblingIterator;

  TreeView(int w, int h);
  virtual ~TreeView();

  // Widget
  virtual void updateArea();
  virtual void draw();
  virtual void cleanFocus();
  virtual bool grabFocus();

  // Container
  virtual void clear();
  virtual bool isWidgetVisible(const Widget& widget) const;
  virtual bool setFocusChild(Widget& child);
  virtual void getFocusChain(FocusChain& focus_chain,
      FocusChain::iterator parent);
  virtual Curses::Window *getSubPad(const Widget& child, int begin_x,
      int begin_y, int ncols, int nlines);

  /**
   * Folds/unfolds given node.
   */
  virtual void setCollapsed(NodeReference node, bool collapsed);
  /**
   * Toggles folding for given node.
   */
  virtual void toggleCollapsed(NodeReference node);
  /**
   * Convenient method to toggle folding of the current active node.
   */
  virtual void actionToggleCollapsed();

  /**
   * Returns root node reference.
   */
  virtual NodeReference getRootNode() const { return thetree.begin(); }

  /**
   * Inserts a widget before a specified position. TreeView takes ownership of
   * the widget.
   */
  virtual NodeReference insertNode(NodeReference position,
      Widget& widget);
  /**
   * Inserts a widget after a specified position. TreeView takes ownership of
   * the widget.
   */
  virtual NodeReference insertNodeAfter(NodeReference position,
      Widget& widget);
  /**
   * Prepends a widget to a specified parent. TreeView takes ownership of the
   * widget.
   */
  virtual NodeReference prependNode(NodeReference parent, Widget& widget);
  /**
   * Appends a widget to a specified parent. TreeView takes ownership of the
   * widget.
   */
  virtual NodeReference appendNode(NodeReference parent, Widget& widget);

  /**
   * Deletes given node.
   */
  virtual void deleteNode(NodeReference node, bool keepchildren);
  /**
   * Deletes all children of given node.
   */
  virtual void deleteNodeChildren(NodeReference node, bool keepchildren);

  /**
   * Returns reference to currently focused node/widget.
   */
  virtual NodeReference getSelectedNode() const;

  /**
   * Returns node depth.
   */
  virtual int getNodeDepth(NodeReference node) const;

  /**
   * Detaches a given node from its current location and moves it before
   * a given position.
   */
  virtual void moveNodeBefore(NodeReference node, NodeReference position);
  /**
   * Detaches a given node from its current location and moves it after
   * a given position.
   */
  virtual void moveNodeAfter(NodeReference node, NodeReference position);
  /**
   * Detaches a given node from its current location and appents it to a new
   * parent.
   */
  virtual void setNodeParent(NodeReference node, NodeReference newparent);
  virtual void setNodeStyle(NodeReference node, Style s);
  virtual Style getNodeStyle(NodeReference node) const;

protected:
  class TreeNode
  {
  /* Note: If TreeNode is just protected/private and all its variables are
   * public, then variables can be accessed from outside using NodeReference.
   */
  friend class TreeView;

  public:
    TreeView *getTreeView() const { return treeview; }
    bool isCollapsed() const { return collapsed; }
    Style getStyle() const { return style; }
    Widget *getWidget() const { return widget; }

  protected:

  private:
    /**
     * Pointer to TreeView this node belongs to.
     */
    TreeView *treeview;

    /**
     * Flag whether the subtree is folded or not.
     */
    bool collapsed;

    /**
     * Selected node drawing style.
     */
    Style style;

    /**
     * Widget to show. Not const because width is changed to fit. E.g. labels
     * can show '...' when the text does not fit in the given space.
     */
    Widget *widget;
  };

  TheTree thetree;
  NodeReference focus_node;

  // Container
  using ScrollPane::addWidget;
  using ScrollPane::removeWidget;
  using ScrollPane::moveWidgetBefore;
  using ScrollPane::moveWidgetAfter;

  virtual int drawNode(SiblingIterator node, int top);

  virtual TreeNode addNode(Widget& widget);

  virtual void fixFocus();

  virtual NodeReference findNode(const Widget& child) const;

  virtual bool isNodeOpenable(SiblingIterator& node) const;
  virtual bool isNodeVisible(NodeReference& node) const;

  // signal handlers
  virtual void onChildMoveResize(Widget& activator, const Rect& oldsize,
      const Rect& newsize);
  virtual void onChildWishSizeChange(Widget& activator, const Size& oldsize,
      const Size& newsize);

private:
  CONSUI_DISABLE_COPY(TreeView);

  void actionCollapse();
  void actionExpand();

  void declareBindables();
};

} // namespace CppConsUI

#endif // __TREEVIEW_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
