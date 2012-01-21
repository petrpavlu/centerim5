/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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
 * Container class implementation.
 *
 * @ingroup cppconsui
 */

#include "Container.h"

namespace CppConsUI
{

Container::Container(int w, int h)
: Widget(w, h), focus_cycle_scope(FOCUS_CYCLE_GLOBAL)
, update_focus_chain(false), page_focus(false), focus_child(NULL)
{
  DeclareBindables();
}

Container::~Container()
{
  CleanFocus();

  Clear();
}

void Container::UpdateArea()
{
  if (!update_area)
    for (Children::iterator i = children.begin(); i != children.end(); i++)
      i->widget->UpdateArea();

  Widget::UpdateArea();
}

void Container::Draw()
{
  ProceedUpdateArea();

  if (!area)
    return;

  area->fill(GetColorPair("container", "background"));

  for (Children::iterator i = children.begin(); i != children.end(); i++)
    if (i->widget->IsVisible())
      i->widget->Draw();
}

Widget *Container::GetFocusWidget()
{
  if (focus_child)
    return focus_child->GetFocusWidget();
  return NULL;
}

void Container::CleanFocus()
{
  if (!focus_child) {
    // apparently there is no widget with focus because the chain ends here
    return;
  }

  // first propagate focus stealing to the widget with focus
  focus_child->CleanFocus();
  focus_child = NULL;
  ClearInputChild();
}

bool Container::RestoreFocus()
{
  if (focus_child)
    return focus_child->RestoreFocus();
  return false;
}

bool Container::GrabFocus()
{
  for (Children::iterator i = children.begin(); i != children.end(); i++)
    if (i->widget->GrabFocus())
      return true;
  return false;
}

void Container::UngrabFocus()
{
  if (focus_child)
    focus_child->UngrabFocus();
}

void Container::SetParent(Container& parent)
{
  /* The parent will take care about focus changing and focus chain caching
   * from now on. */
  focus_chain.clear();

  Widget::SetParent(parent);
}

void Container::AddWidget(Widget& widget, int x, int y)
{
  InsertWidget(children.size(), widget, x, y);
}

void Container::RemoveWidget(Widget& widget)
{
  g_assert(widget.GetParent() == this);
  Children::iterator i = FindWidget(widget);
  g_assert(i != children.end());

  delete i->widget;
  children.erase(i);
}

void Container::MoveWidgetBefore(Widget& widget, Widget& position)
{
  MoveWidgetInternal(widget, position, false);
}

void Container::MoveWidgetAfter(Widget& widget, Widget& position)
{
  MoveWidgetInternal(widget, position, true);
}

void Container::Clear()
{
  for (Children::iterator i = children.begin(); i != children.end(); i++)
    delete i->widget;
  children.clear();
}

bool Container::IsWidgetVisible(const Widget& widget) const
{
  if (!parent || !visible)
    return false;

  return parent->IsWidgetVisible(*this);
}

bool Container::SetFocusChild(Widget& child)
{
  // focus cannot be set for widget without a parent
  if (!parent || !visible)
    return false;

  bool res = parent->SetFocusChild(*this);
  focus_child = &child;
  SetInputChild(child);
  return res;
}

void Container::GetFocusChain(FocusChain& focus_chain,
    FocusChain::iterator parent)
{
  for (Children::iterator i = children.begin(); i != children.end(); i++) {
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
    else if ((widget->CanFocus() && widget->IsVisible())
        || widget == focus_child) {
      // widget can be focused or is focused already
      focus_chain.append_child(parent, widget);
    }
  }
}

void Container::UpdateFocusChain()
{
  if (parent) {
    parent->UpdateFocusChain();
    return;
  }
  update_focus_chain = true;
}

void Container::MoveFocus(FocusDirection direction)
{
  /* Make sure we always start at the root of the widget tree, things are
   * a bit easier then. */
  if (parent) {
    parent->MoveFocus(direction);
    return;
  }

  if (update_focus_chain) {
    focus_chain.clear();
    focus_chain.set_head(this);
    GetFocusChain(focus_chain, focus_chain.begin());
    update_focus_chain = false;
  }

  FocusChain::pre_order_iterator iter = ++focus_chain.begin();
  Widget *focus_widget = GetFocusWidget();

  if (focus_widget) {
    iter = std::find(focus_chain.begin(), focus_chain.end(), focus_widget);

    // we have a focused widget but we couldn't find it
    g_assert(iter != focus_chain.end());

    Widget *widget = *iter;
    if (!widget->IsVisibleRecursive()) {
      /* Currently focused widget is no longer visible, MoveFocus() was called
       * to fix it. */

      // try to change focus locally first
      FocusChain::pre_order_iterator parent_iter = focus_chain.parent(iter);
      iter = focus_chain.erase(iter);
      FocusChain::pre_order_iterator i = iter;
      while (i != parent_iter.end()) {
        if ((*i)->CanFocus())
          break;
        i++;
      }
      if (i == parent_iter.end())
        for (i = parent_iter.begin(); i != iter; i++)
          if ((*i)->CanFocus())
            break;
      if (i != iter && (*i)->CanFocus()) {
        // local focus change was successful
        (*i)->GrabFocus();
        return;
      }

      /* Focus widget couldn't be changed in local scope, give focus to
       * any widget. */
      CleanFocus();
      focus_widget = NULL;
    }
  }

  if (!focus_widget) {
    /* There is no node assigned to receive focus so give focus to the
     * first widget. */
    FocusChain::pre_order_iterator i = iter;
    while (i != focus_chain.end()) {
      if ((*i)->CanFocus())
        break;
      i++;
    }
    if (i == focus_chain.end())
      for (i = ++focus_chain.begin(); i != iter; i++)
        if ((*i)->CanFocus())
          break;

    if (i != iter && (*i)->CanFocus())
      (*i)->GrabFocus();

    return;
  }

  /* At this point, focus_widget is non-NULL and iter represents focus_widget
   * in the focus_chain. Also currently focused widget is visible, so if there
   * isn't any other widget that can take the focus then the focused widget
   * can remain the same. */

  // search for a parent that has set local or none focus cycling
  FocusCycleScope scope = FOCUS_CYCLE_GLOBAL;
  FocusChain::pre_order_iterator cycle_begin, cycle_end, parent_iter;
  parent_iter = focus_chain.parent(iter);
  while (parent_iter != focus_chain.begin()) {
    Container *c = dynamic_cast<Container*>(*parent_iter);
    g_assert(c);
    scope = c->GetFocusCycle();
    if (scope == FOCUS_CYCLE_LOCAL || scope == FOCUS_CYCLE_NONE)
      break;
    parent_iter = focus_chain.parent(parent_iter);
  }
  Container *container = dynamic_cast<Container*>(*parent_iter);
  g_assert(container);

  if (direction == FOCUS_PAGE_UP || direction == FOCUS_PAGE_DOWN) {
    /* Get rid off "dummy" containers in the chain, i.e. container that has
     * only one child. This is needed to get a correct container for paging.
     */
    while (parent_iter.number_of_children() == 1
        && !(*parent_iter.begin())->CanFocus())
      parent_iter = parent_iter.begin();
    container = dynamic_cast<Container*>(*parent_iter);
    g_assert(container);

    /* Stop here if focus change via paging is requested but container doesn't
     * support it. */
    if (!container->CanPageFocus())
      return;

    // paging is requested, force no cycling
    scope = FOCUS_CYCLE_NONE;
  }

  if (scope == FOCUS_CYCLE_GLOBAL) {
    /* Global focus cycling is allowed (cycling amongst all widgets in the
     * window). */
    cycle_begin = ++focus_chain.begin();
    cycle_end = focus_chain.end();
  }
  else if (scope == FOCUS_CYCLE_LOCAL) {
    /* Local focus cycling is allowed (cycling amongs all widgets of the
     * parent container). */
    cycle_begin = parent_iter.begin();
    cycle_end = parent_iter.end();
  }
  else {
    // none focus cycling is allowed
  }

  // find the correct widget to focus
  int max, init, cur;
  if (direction == FOCUS_PAGE_UP || direction == FOCUS_PAGE_DOWN) {
    max = container->GetRealHeight() / 2;
    init = focus_widget->GetRelativePosition(*container).GetY();
  }
  else
    max = init = cur = 0;

  switch (direction) {
    case FOCUS_PREVIOUS:
    case FOCUS_UP:
    case FOCUS_LEFT:
    case FOCUS_PAGE_UP:
      // finally, find the previous widget which will get the focus
      do {
        /* If no focus cycling is allowed, stop if the widget with focus is the
         * first/last child. */
        if (scope == FOCUS_CYCLE_NONE && iter == parent_iter.begin())
          goto end;

        if (iter == cycle_begin)
          iter = cycle_end;
        iter--;

        if (direction == FOCUS_PAGE_UP)
          cur = (*iter)->GetRelativePosition(*container).GetY();
      } while (!(*iter)->CanFocus() || init - cur < max);

      break;
    case FOCUS_NEXT:
    case FOCUS_DOWN:
    case FOCUS_RIGHT:
    case FOCUS_PAGE_DOWN:
      // finally, find the next widget which will get the focus
      do {
        if (scope == FOCUS_CYCLE_NONE) {
          /* parent_iter.end() returns a sibling_iterator, it has to be
           * converted to pre_order_iterator first... */
          FocusChain::pre_order_iterator tmp = parent_iter.end();
          tmp--;
          if (iter == tmp)
            goto end;
        }

        iter++;
        if (iter == cycle_end)
          iter = cycle_begin;

        if (direction == FOCUS_PAGE_DOWN)
          cur = (*iter)->GetRelativePosition(*container).GetY();
      } while (!(*iter)->CanFocus() || cur - init < max);

      break;
    case FOCUS_BEGIN:
      iter = parent_iter.begin();
      while (iter != parent_iter.end()) {
        if ((*iter)->CanFocus())
          goto end;
        iter++;
      }
      /* There is always one widget that can get the focus so this code is
       * unreachable. */
      g_assert_not_reached();
      break;
    case FOCUS_END:
      iter = parent_iter.end();
      iter--;
      break;
  }

end:
  // grab the focus
  (*iter)->GrabFocus();
}

Point Container::GetRelativePosition(const Container& ref,
    const Widget& child) const
{
  g_assert(child.GetParent() == this);

  if (!parent || this == &ref)
    return Point(child.GetLeft(), child.GetTop());

  Point p = parent->GetRelativePosition(ref, *this);
  return Point(p.GetX() + child.GetLeft(), p.GetY() + child.GetTop());
}

Point Container::GetAbsolutePosition(const Widget& child) const
{
  g_assert(child.GetParent() == this);

  if (!parent)
    return Point(child.GetLeft(), child.GetTop());

  Point p = parent->GetAbsolutePosition(*this);
  return Point(p.GetX() + child.GetLeft(), p.GetY() + child.GetTop());
}

Curses::Window *Container::GetSubPad(const Widget& child, int begin_x,
    int begin_y, int ncols, int nlines)
{
  if (!area)
    return NULL;

  int realw = area->getmaxx();
  int realh = area->getmaxy();

  if (nlines == AUTOSIZE)
    nlines = child.GetWishHeight();
  if (ncols == AUTOSIZE)
    ncols = child.GetWishWidth();

  /* Extend requested subpad to whole parent area or shrink requested area
   * if necessary. */
  if (nlines == AUTOSIZE || nlines > realh - begin_y)
    nlines = realh - begin_y;

  if (ncols == AUTOSIZE || ncols > realw - begin_x)
    ncols = realw - begin_x;

  if (nlines <= 0 || ncols <= 0)
    return NULL;

  return area->subpad(begin_x, begin_y, ncols, nlines);
}

Container::Children::iterator Container::FindWidget(const Widget& widget)
{
  Children::iterator i;
  for (i = children.begin(); i != children.end(); i++)
    if (i->widget == &widget)
      break;
  return i;
}

void Container::InsertWidget(size_t pos, Widget& widget, int x, int y)
{
  g_assert(pos <= children.size());

  Child child(widget);
  widget.Move(x, y);

  /* Insert a widget early into children vector so the widget can grab the
   * focus in SetParent() method if it detects that there isn't any focused
   * widget. */
  children.insert(children.begin() + pos, child);
  widget.SetParent(*this);

  children[pos].sig_moveresize = widget.signal_moveresize.connect(
      sigc::mem_fun(this, &Container::OnChildMoveResize));
  children[pos].sig_moveresize = widget.signal_wish_size_change.connect(
      sigc::mem_fun(this, &Container::OnChildWishSizeChange));
  children[pos].sig_visible = widget.signal_visible.connect(
      sigc::mem_fun(this, &Container::OnChildVisible));
}

void Container::MoveWidgetInternal(Widget& widget, Widget& position,
    bool after)
{
  g_assert(widget.GetParent() == this);
  g_assert(position.GetParent() == this);

  // remove widget from the children..
  Children::iterator widget_iter = FindWidget(widget);
  g_assert(widget_iter != children.end());
  Child child = *widget_iter;
  children.erase(widget_iter);

  // ..put it back into a correct position
  Children::iterator position_iter = FindWidget(position);
  g_assert(position_iter != children.end());
  if (after)
    position_iter++;
  children.insert(position_iter, child);

  UpdateFocusChain();

  // need redraw if the widgets overlap
  Redraw();
}

void Container::OnChildMoveResize(Widget& activator, const Rect& oldsize,
    const Rect& newsize)
{
}

void Container::OnChildWishSizeChange(Widget& activator, const Size& oldsize,
    const Size& newsize)
{
}

void Container::OnChildVisible(Widget& activator, bool visible)
{
}

void Container::DeclareBindables()
{
  DeclareBindable("container", "focus-previous",
      sigc::bind(sigc::mem_fun(this, &Container::MoveFocus),
        Container::FOCUS_PREVIOUS), InputProcessor::BINDABLE_NORMAL);
  DeclareBindable("container", "focus-next",
      sigc::bind(sigc::mem_fun(this, &Container::MoveFocus),
        Container::FOCUS_NEXT), InputProcessor::BINDABLE_NORMAL);
  DeclareBindable("container", "focus-up",
      sigc::bind(sigc::mem_fun(this, &Container::MoveFocus),
        Container::FOCUS_UP), InputProcessor::BINDABLE_NORMAL);
  DeclareBindable("container", "focus-down",
      sigc::bind(sigc::mem_fun(this, &Container::MoveFocus),
        Container::FOCUS_DOWN), InputProcessor::BINDABLE_NORMAL);
  DeclareBindable("container", "focus-left",
      sigc::bind(sigc::mem_fun(this, &Container::MoveFocus),
        Container::FOCUS_LEFT), InputProcessor::BINDABLE_NORMAL);
  DeclareBindable("container", "focus-right",
      sigc::bind(sigc::mem_fun(this, &Container::MoveFocus),
        Container::FOCUS_RIGHT), InputProcessor::BINDABLE_NORMAL);
  DeclareBindable("container", "focus-page-up",
      sigc::bind(sigc::mem_fun(this, &Container::MoveFocus),
        Container::FOCUS_PAGE_UP), InputProcessor::BINDABLE_NORMAL);
  DeclareBindable("container", "focus-page-down",
      sigc::bind(sigc::mem_fun(this, &Container::MoveFocus),
        Container::FOCUS_PAGE_DOWN), InputProcessor::BINDABLE_NORMAL);
  DeclareBindable("container", "focus-begin",
      sigc::bind(sigc::mem_fun(this, &Container::MoveFocus),
        Container::FOCUS_BEGIN), InputProcessor::BINDABLE_NORMAL);
  DeclareBindable("container", "focus-end",
      sigc::bind(sigc::mem_fun(this, &Container::MoveFocus),
        Container::FOCUS_END), InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab */
