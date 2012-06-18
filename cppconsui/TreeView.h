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
    virtual void SetParent(Container& parent);

  protected:

  private:
    ToggleCollapseButton(const ToggleCollapseButton&);
    ToggleCollapseButton& operator=(const ToggleCollapseButton&);
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
  virtual void Draw();
  virtual void CleanFocus();
  virtual bool GrabFocus();

  // Container
  virtual void Clear();
  virtual bool IsWidgetVisible(const Widget& widget) const;
  virtual bool SetFocusChild(Widget& child);
  virtual void GetFocusChain(FocusChain& focus_chain,
      FocusChain::iterator parent);
  virtual Curses::Window *GetSubPad(const Widget& child, int begin_x,
      int begin_y, int ncols, int nlines);

  /**
   * Folds/unfolds given node.
   */
  virtual void SetCollapsed(NodeReference node, bool collapsed);
  /**
   * Toggles folding for given node.
   */
  virtual void ToggleCollapsed(NodeReference node);
  /**
   * Convenient method to toggle folding of the current active node.
   */
  virtual void ActionToggleCollapsed();

  /**
   * Returns root node reference.
   */
  virtual NodeReference GetRootNode() const { return thetree.begin(); }

  /**
   * Inserts a widget before a specified position. TreeView takes ownership of
   * the widget.
   */
  virtual NodeReference InsertNode(NodeReference position,
      Widget& widget);
  /**
   * Inserts a widget after a specified position. TreeView takes ownership of
   * the widget.
   */
  virtual NodeReference InsertNodeAfter(NodeReference position,
      Widget& widget);
  /**
   * Prepends a widget to a specified parent. TreeView takes ownership of the
   * widget.
   */
  virtual NodeReference PrependNode(NodeReference parent, Widget& widget);
  /**
   * Appends a widget to a specified parent. TreeView takes ownership of the
   * widget.
   */
  virtual NodeReference AppendNode(NodeReference parent, Widget& widget);

  /**
   * Deletes given node.
   */
  virtual void DeleteNode(NodeReference node, bool keepchildren);
  /**
   * Deletes all children of given node.
   */
  virtual void DeleteNodeChildren(NodeReference node, bool keepchildren);

  /**
   * Returns reference to currently focused node/widget.
   */
  virtual NodeReference GetSelectedNode() const;

  /**
   * Returns node depth.
   */
  virtual int GetNodeDepth(NodeReference node) const;

  /**
   * Detaches a given node from its current location and moves it before
   * a given position.
   */
  virtual void MoveNodeBefore(NodeReference node, NodeReference position);
  /**
   * Detaches a given node from its current location and moves it after
   * a given position.
   */
  virtual void MoveNodeAfter(NodeReference node, NodeReference position);
  /**
   * Detaches a given node from its current location and appents it to a new
   * parent.
   */
  virtual void SetNodeParent(NodeReference node, NodeReference newparent);
  virtual void SetNodeStyle(NodeReference node, Style s);
  virtual Style GetNodeStyle(NodeReference node) const;

protected:
  class TreeNode
  {
  /* Note: If TreeNode is just protected/private and all its variables are
   * public, then variables can be accessed from outside using NodeReference.
   */
  friend class TreeView;

  public:
    TreeView *GetTreeView() const { return treeview; }
    bool GetCollapsed() const { return collapsed; }
    Style GetStyle() const { return style; }
    Widget *GetWidget() const { return widget; }

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
  ScrollPane::AddWidget;
  ScrollPane::RemoveWidget;
  ScrollPane::MoveWidgetBefore;
  ScrollPane::MoveWidgetAfter;

  virtual int DrawNode(SiblingIterator node, int top);

  virtual TreeNode AddNode(Widget& widget);

  virtual void FixFocus();

  virtual NodeReference FindNode(const Widget& child) const;

  virtual bool IsNodeOpenable(SiblingIterator& node) const;
  virtual bool IsNodeVisible(NodeReference& node) const;

  // signal handlers
  virtual void OnChildMoveResize(Widget& activator, const Rect& oldsize,
      const Rect& newsize);
  virtual void OnChildWishSizeChange(Widget& activator, const Size& oldsize,
      const Size& newsize);

private:
  TreeView(const TreeView&);
  TreeView& operator=(const TreeView&);

  void ActionCollapse();
  void ActionExpand();

  void DeclareBindables();
};

} // namespace CppConsUI

#endif // __TREEVIEW_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
