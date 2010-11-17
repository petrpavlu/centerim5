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

/**
 * @file
 * Container class.
 *
 * @ingroup cppconsui
 */

#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include <vector>

#include "Widget.h"
#include "tree.hh"

/**
 * The generic widget container class.
 *
 * It implements @ref MoveFocus "moving focus" in different @ref
 * FocusDirection "directions".
 */
class Container
: public Widget
{
public:
  /**
   * Type to keep a tree of "focusable" widgets as leaves and Containers as
   * internal nodes.
   */
  typedef tree<Widget*> FocusChain;
  enum FocusCycleScope {
    /**
     * The focus doesn't cycle, it ends at the last widget from the focus
     * chain.
     */
    FOCUS_CYCLE_NONE,
    /**
     * The focus cycles only locally.
     */
    FOCUS_CYCLE_LOCAL,
    /**
     * The focus cycles also through the other containers windows.
     */
    FOCUS_CYCLE_GLOBAL
  };

  enum FocusDirection {
    FOCUS_NEXT,
    FOCUS_PREVIOUS,
    FOCUS_DOWN,
    FOCUS_UP,
    FOCUS_RIGHT,
    FOCUS_LEFT
  };

  Container(int w, int h);
  virtual ~Container();

  // Widget
  virtual void MoveResize(int newx, int newy, int neww, int newh);
  virtual void Draw();
  virtual Widget *GetFocusWidget();
  virtual void CleanFocus();
  virtual void RestoreFocus();
  virtual bool GrabFocus();
  virtual void SetParent(Container& parent);

  /**
   * Adds a widget to the children list. The Container takes ownership of the
   * widget. It means that the widget will be deleted by the Container.
   */
  virtual void AddWidget(Widget& widget, int x, int y);
  /**
   * Removes the widget from the children list but it doesn't delete the
   * widget. The caller must ensure proper deletion of the widget.
   */
  virtual void RemoveWidget(Widget& widget);

  /**
   * Removes (and deletes) all children widgets.
   */
  virtual void Clear();

  /**
   * Returns true if a widget is visible in current context.
   */
  virtual bool IsWidgetVisible(const Widget& widget) const;

  /**
   * Resets the focus child by @ref CleanFocus "stealing" the focus from the
   * current chain and also ensures the focus goes also UP the chain to the
   * root widget (normally a Window).
   */
  virtual bool SetFocusChild(Widget& child);
  Widget *GetFocusChild() const { return focus_child; }
  /**
   * Guilds a tree of the focus chain starting from this container and puts it
   * into the focus_chain tree as a subtree of @ref parent.
   */
  virtual void GetFocusChain(FocusChain& focus_chain,
      FocusChain::iterator parent);
  /**
   * @todo Have a return value (to see if focus was moved successfully or
   * not)?
   */
  virtual void MoveFocus(FocusDirection direction);

  void SetFocusCycle(FocusCycleScope scope) { focus_cycle_scope = scope; }
  FocusCycleScope GetFocusCycle() const { return focus_cycle_scope; }

  /**
   * Set focused widget.
   */
  virtual bool SetActive(int i);

  /**
   * Returns index of focused widget or -1 if there is not any.
   */
  virtual int GetActive() const;

  /**
   * Returns a subpad of current widget with given coordinates.
   */
  virtual Curses::Window *GetSubPad(const Widget& child, int begin_x,
      int begin_y, int ncols, int nlines);

protected:
  /**
   * Structure to keep a child widget.
   */
  struct Child {
    Widget *widget;

    // signal connection to the widget
    sigc::connection sig_moveresize;
    sigc::connection sig_redraw;
    sigc::connection sig_visible;
  };
  typedef std::vector<Child> Children;

  FocusCycleScope focus_cycle_scope;

  /**
   * This defines a chain of focus
   * @todo explain the difference between this chain and @ref
   * InputProcessor::inputchild Isn't this a duplication of functionality
   * from inputchild ?
   */
  Widget *focus_child;

  Children children;

  /**
   * Inserts a widget in the children list at a given position. The Container
   * takes ownership of the widget. It means that the widget will be deleted
   * by the Container. This function is intended to be used by derived classes
   * that needs to keep child widgets in order (see ListBox and
   * HorizontalListBox).
   */
  virtual void InsertWidget(size_t pos, Widget& widget, int x, int y);

  virtual void OnChildMoveResize(Widget& widget, Rect& oldsize,
      Rect& newsize);
  virtual void OnChildRedraw(Widget& widget);
  virtual void OnChildVisible(Widget& widget, bool visible);

private:
  Container(const Container&);
  Container& operator=(const Container&);

  /**
   * Automatic registration of defined keys.
   */
  DECLARE_SIG_REGISTERKEYS();
  static bool RegisterKeys();
  void DeclareBindables();
};

#endif // __CONTAINER_H__
