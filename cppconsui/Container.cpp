/*
 * Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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

/**
 * @file
 * Container class implementation.
 *
 * @ingroup cppconsui
 */

#include "Container.h"

#include "ColorScheme.h"

#include <cassert>

namespace CppConsUI {

Container::Container(int w, int h)
  : Widget(w, h), scroll_xpos(0), scroll_ypos(0), border(0),
    focus_cycle_scope(FOCUS_CYCLE_GLOBAL), update_focus_chain(false),
    page_focus(false), focus_child(NULL)
{
  declareBindables();
}

Container::~Container()
{
  cleanFocus();

  clear();
}

int Container::draw(Curses::ViewPort area, Error &error)
{
  if (real_width <= 0 || real_height <= 0)
    return 0;

  if (area.getViewWidth() <= 0 || area.getViewHeight() <= 0)
    return 0;

  area.scroll(scroll_xpos, scroll_ypos);
  int attrs;
  DRAW(getAttributes(ColorScheme::CONTAINER_BACKGROUND, &attrs, error));
  DRAW(area.fill(attrs, error));

  for (Children::iterator i = children.begin(); i != children.end(); ++i)
    if ((*i)->isVisible())
      DRAW(drawChild(**i, area, error));

  return 0;
}

Widget *Container::getFocusWidget()
{
  if (focus_child)
    return focus_child->getFocusWidget();
  return NULL;
}

void Container::cleanFocus()
{
  if (!focus_child) {
    // apparently there is no widget with focus because the chain ends here
    return;
  }

  // first propagate focus stealing to the widget with focus
  focus_child->cleanFocus();
  focus_child = NULL;
  clearInputChild();
}

bool Container::restoreFocus()
{
  if (focus_child)
    return focus_child->restoreFocus();
  return false;
}

bool Container::grabFocus()
{
  for (Children::iterator i = children.begin(); i != children.end(); ++i)
    if ((*i)->grabFocus())
      return true;
  return false;
}

void Container::ungrabFocus()
{
  if (focus_child)
    focus_child->ungrabFocus();
}

void Container::setParent(Container &parent)
{
  /* The parent will take care about focus changing and focus chain caching
   * from now on. */
  focus_chain.clear();

  Widget::setParent(parent);
}

void Container::addWidget(Widget &widget, int x, int y)
{
  insertWidget(children.size(), widget, x, y);
}

void Container::removeWidget(Widget &widget)
{
  assert(widget.getParent() == this);
  Children::iterator i = findWidget(widget);
  assert(i != children.end());

  delete *i;
  children.erase(i);
}

void Container::moveWidgetBefore(Widget &widget, Widget &position)
{
  moveWidgetInternal(widget, position, false);
}

void Container::moveWidgetAfter(Widget &widget, Widget &position)
{
  moveWidgetInternal(widget, position, true);
}

void Container::clear()
{
  while (children.size())
    removeWidget(*children.front());
}

bool Container::isWidgetVisible(const Widget & /*widget*/) const
{
  if (!parent || !visible)
    return false;

  return parent->isWidgetVisible(*this);
}

bool Container::setFocusChild(Widget &child)
{
  // focus cannot be set for widget without a parent
  if (!parent || !visible)
    return false;

  bool res = parent->setFocusChild(*this);
  focus_child = &child;
  setInputChild(child);
  updateScroll();
  return res;
}

void Container::getFocusChain(
  FocusChain &focus_chain, FocusChain::iterator parent)
{
  for (Children::iterator i = children.begin(); i != children.end(); ++i) {
    Widget *widget = *i;
    Container *container = dynamic_cast<Container *>(widget);

    if (container && container->isVisible()) {
      // the widget is a container so add its widgets as well
      FocusChain::pre_order_iterator iter =
        focus_chain.append_child(parent, container);
      container->getFocusChain(focus_chain, iter);

      /* If this is not a focusable widget and it has no focusable
       * children, remove it from the chain. */
      if (!focus_chain.number_of_children(iter))
        focus_chain.erase(iter);
    }
    else if ((widget->canFocus() && widget->isVisible()) ||
      widget == focus_child) {
      // widget can be focused or is focused already
      focus_chain.append_child(parent, widget);
    }
  }
}

void Container::updateFocusChain()
{
  if (parent) {
    parent->updateFocusChain();
    return;
  }
  update_focus_chain = true;
}

void Container::moveFocus(FocusDirection direction)
{
  /* Make sure we always start at the root of the widget tree, things are
   * a bit easier then. */
  if (parent) {
    parent->moveFocus(direction);
    return;
  }

  if (update_focus_chain) {
    focus_chain.clear();
    focus_chain.set_head(this);
    getFocusChain(focus_chain, focus_chain.begin());
    update_focus_chain = false;
  }

  FocusChain::pre_order_iterator iter = ++focus_chain.begin();
  Widget *focus_widget = getFocusWidget();

  if (focus_widget) {
    iter = std::find(focus_chain.begin(), focus_chain.end(), focus_widget);

    // we have a focused widget but we couldn't find it
    assert(iter != focus_chain.end());

    Widget *widget = *iter;
    if (!widget->isVisibleRecursive()) {
      /* Currently focused widget is no longer visible, moveFocus() was called
       * to fix it. */

      // try to change focus locally first
      FocusChain::pre_order_iterator parent_iter = focus_chain.parent(iter);
      iter = focus_chain.erase(iter);
      FocusChain::pre_order_iterator i = iter;
      while (i != parent_iter.end()) {
        if ((*i)->canFocus())
          break;
        ++i;
      }
      if (i == parent_iter.end())
        for (i = parent_iter.begin(); i != iter; ++i)
          if ((*i)->canFocus())
            break;
      if (i != parent_iter.end() && (*i)->canFocus()) {
        // local focus change was successful

        // stay sane
        assert((*i)->isVisibleRecursive());

        (*i)->grabFocus();
        return;
      }

      /* Focus widget couldn't be changed in local scope, give focus to
       * any widget. */
      cleanFocus();
      focus_widget = NULL;
    }
  }

  if (!focus_widget) {
    /* There is no node assigned to receive focus so give focus to the
     * first widget. */
    FocusChain::pre_order_iterator i = iter;
    while (i != focus_chain.end()) {
      if ((*i)->canFocus())
        break;
      ++i;
    }
    if (i == focus_chain.end())
      for (i = ++focus_chain.begin(); i != iter; ++i)
        if ((*i)->canFocus())
          break;

    if (i != focus_chain.end() && (*i)->canFocus()) {
      // stay sane
      assert((*i)->isVisibleRecursive());

      (*i)->grabFocus();
    }

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
    Container *c = dynamic_cast<Container *>(*parent_iter);
    assert(c);
    scope = c->getFocusCycle();
    if (scope == FOCUS_CYCLE_LOCAL || scope == FOCUS_CYCLE_NONE)
      break;
    parent_iter = focus_chain.parent(parent_iter);
  }
  Container *container = dynamic_cast<Container *>(*parent_iter);
  assert(container);

  if (direction == FOCUS_PAGE_UP || direction == FOCUS_PAGE_DOWN) {
    /* Get rid off "dummy" containers in the chain, i.e. container that has
     * only one child. This is needed to get a correct container for paging.
     */
    while (parent_iter.number_of_children() == 1 &&
      !(*parent_iter.begin())->canFocus())
      parent_iter = parent_iter.begin();
    container = dynamic_cast<Container *>(*parent_iter);
    assert(container);

    /* Stop here if focus change via paging is requested but container doesn't
     * support it. */
    if (!container->canPageFocus())
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
    max = container->getRealHeight() / 2;
    init = focus_widget->getRelativePosition(*container).getY();
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
      /* If no focus cycling is allowed, stop if the widget with focus is
       * the first/last child. */
      if (scope == FOCUS_CYCLE_NONE && iter == parent_iter.begin())
        goto end;

      if (iter == cycle_begin)
        iter = cycle_end;
      --iter;

      if (direction == FOCUS_PAGE_UP)
        cur = (*iter)->getRelativePosition(*container).getY();
    } while (!(*iter)->canFocus() || init - cur < max);

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
        --tmp;
        if (iter == tmp)
          goto end;
      }

      ++iter;
      if (iter == cycle_end)
        iter = cycle_begin;

      if (direction == FOCUS_PAGE_DOWN)
        cur = (*iter)->getRelativePosition(*container).getY();
    } while (!(*iter)->canFocus() || cur - init < max);

    break;
  case FOCUS_BEGIN:
    iter = parent_iter.begin();
    while (iter != parent_iter.end()) {
      if ((*iter)->canFocus())
        goto end;
      ++iter;
    }
    /* There is always one widget that can get the focus so this code is
     * unreachable. */
    assert(0);
    break;
  case FOCUS_END:
    iter = parent_iter.end();
    --iter;
    break;
  }

end:
  // grab the focus
  (*iter)->grabFocus();
}

Point Container::getRelativePosition(
  const Container &ref, const Widget &child) const
{
  assert(child.getParent() == this);

  int child_x = child.getRealLeft();
  int child_y = child.getRealTop();

  if (child_x != UNSETPOS && child_y != UNSETPOS) {
    child_x -= scroll_xpos;
    child_y -= scroll_ypos;

    if (!parent || this == &ref)
      return Point(child_x, child_y);

    Point p = parent->getRelativePosition(ref, *this);
    if (p.getX() != UNSETPOS && p.getY() != UNSETPOS)
      return Point(p.getX() + child_x, p.getY() + child_y);
  }
  return Point(UNSETPOS, UNSETPOS);
}

Point Container::getAbsolutePosition(const Widget &child) const
{
  assert(child.getParent() == this);

  int child_x = child.getRealLeft();
  int child_y = child.getRealTop();

  if (child_x != UNSETPOS && child_y != UNSETPOS) {
    child_x -= scroll_xpos;
    child_y -= scroll_ypos;

    if (parent) {
      Point p = parent->getAbsolutePosition(*this);
      if (p.getX() != UNSETPOS && p.getY() != UNSETPOS)
        return Point(p.getX() + child_x, p.getY() + child_y);
    }
  }

  return Point(UNSETPOS, UNSETPOS);
}

void Container::onChildMoveResize(
  Widget &activator, const Rect & /*oldsize*/, const Rect &newsize)
{
  // can be called only by a real child
  assert(activator.getParent() == this);

  // update child real size
  activator.setRealPosition(newsize.getLeft(), newsize.getTop());
  updateChildArea(activator);
}

void Container::onChildWishSizeChange(
  Widget &activator, const Size & /*oldsize*/, const Size & /*newsize*/)
{
  // can be called only by a real child
  assert(activator.getParent() == this);

  // update child real size
  updateChildArea(activator);
}

void Container::onChildVisible(Widget &activator, bool /*visible*/)
{
  // can be called only by a real child
  assert(activator.getParent() == this);
}

void Container::updateArea()
{
  // update all child areas
  for (Children::iterator i = children.begin(); i != children.end(); ++i)
    updateChildArea(**i);
}

void Container::updateChildArea(Widget &child)
{
  int child_x = child.getRealLeft();
  int child_y = child.getRealTop();

  if (child_x == UNSETPOS || child_y == UNSETPOS) {
    child.setRealSize(0, 0);
    return;
  }

  int max_width = real_width - border;
  int max_height = real_height - border;

  int child_width = child.getWidth();
  int child_height = child.getHeight();

  if (child_width == AUTOSIZE)
    child_width = child.getWishWidth();
  if (child_height == AUTOSIZE)
    child_height = child.getWishHeight();

  /* Extend the requested area to the whole parent area or shrink the
   * requested area if necessary. */
  if (child_width == AUTOSIZE || child_width > max_width - child_x)
    child_width = max_width - child_x;

  if (child_height == AUTOSIZE || child_height > max_height - child_y)
    child_height = max_height - child_y;

  if (child_width > 0 && child_height > 0)
    child.setRealSize(child_width, child_height);
  else
    child.setRealSize(0, 0);
}

int Container::drawChild(Widget &child, Curses::ViewPort area, Error &error)
{
  int view_x = area.getViewLeft();
  int view_y = area.getViewTop();
  int view_width = area.getViewWidth();
  int view_height = area.getViewHeight();

  int view_x2 = view_x + view_width;
  int view_y2 = view_y + view_height;

  int child_x = child.getRealLeft();
  int child_y = child.getRealTop();
  int child_width = child.getRealWidth();
  int child_height = child.getRealHeight();
  int child_x2 = child_x + child_width;
  int child_y2 = child_y + child_height;

  int child_screen_x = area.getScreenLeft() + child_x - view_x;
  int child_screen_y = area.getScreenTop() + child_y - view_y;

  int child_view_x = 0;
  int child_view_y = 0;
  int child_view_width = child_width;
  int child_view_height = child_height;

  // calculate a viewport for the child
  if (view_x > child_x) {
    child_view_x = view_x - child_x;
    child_screen_x += child_view_x;
    if (child_view_x > child_width)
      child_view_x = child_width;
    child_view_width -= child_view_x;
  }
  if (view_y > child_y) {
    child_view_y = view_y - child_y;
    child_screen_y += child_view_y;
    if (child_view_y > child_height)
      child_view_y = child_height;
    child_view_height -= child_view_y;
  }

  if (child_x2 > view_x2) {
    child_view_width -= child_x2 - view_x2;
    if (child_view_width < 0)
      child_view_width = 0;
  }
  if (child_y2 > view_y2) {
    child_view_height -= child_y2 - view_y2;
    if (child_view_height < 0)
      child_view_height = 0;
  }

  Curses::ViewPort child_area(child_screen_x, child_screen_y, child_view_x,
    child_view_y, child_view_width, child_view_height);
  return child.draw(child_area, error);
}

Container::Children::iterator Container::findWidget(const Widget &widget)
{
  return std::find(children.begin(), children.end(), &widget);
}

void Container::insertWidget(size_t pos, Widget &widget, int x, int y)
{
  assert(pos <= children.size());

  widget.move(x, y);

  /* Insert a widget early into children vector so the widget can grab the
   * focus in setParent() method if it detects that there isn't any focused
   * widget. */
  children.insert(children.begin() + pos, &widget);
  widget.setParent(*this);
  widget.setRealPosition(widget.getLeft(), widget.getTop());
  updateChildArea(widget);
}

void Container::moveWidgetInternal(Widget &widget, Widget &position, bool after)
{
  assert(widget.getParent() == this);
  assert(position.getParent() == this);

  // remove widget from the children..
  Children::iterator widget_iter = findWidget(widget);
  assert(widget_iter != children.end());
  children.erase(widget_iter);

  // ..put it back into a correct position
  Children::iterator position_iter = findWidget(position);
  assert(position_iter != children.end());
  if (after)
    ++position_iter;
  children.insert(position_iter, &widget);

  updateFocusChain();

  // need redraw if the widgets overlap
  redraw();
}

void Container::updateScroll()
{
  if (!focus_child)
    return;

  int x = focus_child->getRealLeft();
  int y = focus_child->getRealTop();
  if (x == UNSETPOS || y == UNSETPOS)
    return;

  int w = focus_child->getRealWidth();
  int h = focus_child->getRealHeight();
  bool scrolled_a, scrolled_b;
  scrolled_a = makePointVisible(x + w - 1, y + h - 1);
  scrolled_b = makePointVisible(x, y);

  if (!scrolled_a && !scrolled_b)
    return;

  redraw();
  signalAbsolutePositionChange();
}

bool Container::makePointVisible(int x, int y)
{
  bool scrolled_x = true;
  if (real_width == 0)
    scroll_xpos = 0;
  else if (x < scroll_xpos)
    scroll_xpos = x;
  else if (x > scroll_xpos + real_width - 1)
    scroll_xpos = x - real_width + 1;
  else
    scrolled_x = false;

  bool scrolled_y = true;
  if (real_height == 0)
    scroll_ypos = 0;
  else if (y < scroll_ypos)
    scroll_ypos = y;
  else if (y > scroll_ypos + real_height - 1)
    scroll_ypos = y - real_height + 1;
  else
    scrolled_y = false;

  return scrolled_x || scrolled_y;
}

void Container::declareBindables()
{
  declareBindable("container", "focus-previous",
    sigc::bind(sigc::mem_fun(this, &Container::moveFocus),
                    Container::FOCUS_PREVIOUS),
    InputProcessor::BINDABLE_NORMAL);
  declareBindable("container", "focus-next",
    sigc::bind(sigc::mem_fun(this, &Container::moveFocus),
                    Container::FOCUS_NEXT),
    InputProcessor::BINDABLE_NORMAL);
  declareBindable("container", "focus-up",
    sigc::bind(sigc::mem_fun(this, &Container::moveFocus), Container::FOCUS_UP),
    InputProcessor::BINDABLE_NORMAL);
  declareBindable("container", "focus-down",
    sigc::bind(sigc::mem_fun(this, &Container::moveFocus),
                    Container::FOCUS_DOWN),
    InputProcessor::BINDABLE_NORMAL);
  declareBindable("container", "focus-left",
    sigc::bind(sigc::mem_fun(this, &Container::moveFocus),
                    Container::FOCUS_LEFT),
    InputProcessor::BINDABLE_NORMAL);
  declareBindable("container", "focus-right",
    sigc::bind(sigc::mem_fun(this, &Container::moveFocus),
                    Container::FOCUS_RIGHT),
    InputProcessor::BINDABLE_NORMAL);
  declareBindable("container", "focus-page-up",
    sigc::bind(sigc::mem_fun(this, &Container::moveFocus),
                    Container::FOCUS_PAGE_UP),
    InputProcessor::BINDABLE_NORMAL);
  declareBindable("container", "focus-page-down",
    sigc::bind(sigc::mem_fun(this, &Container::moveFocus),
                    Container::FOCUS_PAGE_DOWN),
    InputProcessor::BINDABLE_NORMAL);
  declareBindable("container", "focus-begin",
    sigc::bind(sigc::mem_fun(this, &Container::moveFocus),
                    Container::FOCUS_BEGIN),
    InputProcessor::BINDABLE_NORMAL);
  declareBindable("container", "focus-end",
    sigc::bind(sigc::mem_fun(this, &Container::moveFocus),
                    Container::FOCUS_END),
    InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
