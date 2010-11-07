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
 * HorizontalListBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "HorizontalListBox.h"

HorizontalListBox::HorizontalListBox(int w, int h)
: AbstractListBox(w, h)
, children_width(0)
, autosize_children(0)
, autosize_width(0)
{
}

void HorizontalListBox::UpdateArea()
{
  AbstractListBox::UpdateArea();

  // set virtual scroll area height
  if (scrollarea)
    SetScrollHeight(scrollarea->getmaxy());
  UpdateScrollWidth();
}

void HorizontalListBox::Draw()
{
  if (!area) {
    // scrollpane will clear the scroll (real) area
    AbstractListBox::Draw();
    return;
  }

  autosize_width = 1;
  int autosize_width_extra = 0;
  int realw = area->getmaxx();
  if (autosize_children && children_width < realw) {
    int space = realw - (children_width - autosize_children);
    autosize_width = space / autosize_children;
    autosize_width_extra = space % autosize_children;
  }
  autosize_extra.clear();

  int x = 0;
  for (Children::iterator i = children.begin(); i != children.end(); i++) {
    Widget *widget = i->widget;
    if (widget->IsVisible()) {
      int w = widget->Width();
      if (w == AUTOSIZE) {
        w = autosize_width;
        if (autosize_width_extra) {
          autosize_extra.insert(widget);
          autosize_width_extra--;
          w++;
        }
      }

      widget->MoveResize(x, 0, widget->Width(), widget->Height());
      x += w;
    }
  }

  // make sure that currently focused widget is visible
  if (focus_child)
    MakeVisible(focus_child->Left(), focus_child->Top());

  AbstractListBox::Draw();
}

VerticalLine *HorizontalListBox::InsertSeparator(size_t pos)
{
  VerticalLine *l = new VerticalLine(AUTOSIZE);
  InsertWidget(pos, *l);
  return l;
}

VerticalLine *HorizontalListBox::AppendSeparator()
{
  VerticalLine *l = new VerticalLine(AUTOSIZE);
  AppendWidget(*l);
  return l;
}

void HorizontalListBox::InsertWidget(size_t pos, Widget& widget)
{
  if (widget.IsVisible()) {
    int w = widget.Width();
    if (w == AUTOSIZE) {
      w = 1;
      autosize_children++;
    }
    children_width += w;
    UpdateScrollWidth();
  }

  // note: widget is moved to a correct position in Draw() method
  ScrollPane::InsertWidget(pos, widget, 0, 0);
}

void HorizontalListBox::AppendWidget(Widget& widget)
{
  InsertWidget(children.size(), widget);
}

Curses::Window *HorizontalListBox::GetSubPad(const Widget& child, int begin_x,
    int begin_y, int ncols, int nlines)
{
  // autosize
  if (ncols == AUTOSIZE) {
    ncols = autosize_width;
    if (autosize_extra.find(&child) != autosize_extra.end())
      ncols++;
  }

  return AbstractListBox::GetSubPad(child, begin_x, begin_y, ncols, nlines);
}

void HorizontalListBox::OnChildMoveResize(Widget& widget, Rect& oldsize, Rect& newsize)
{
  int old_width = oldsize.Width();
  int new_width = newsize.Width();
  if (old_width != new_width) {
    if (old_width == AUTOSIZE) {
      old_width = 1;
      autosize_children--;
    }
    if (new_width == AUTOSIZE) {
      new_width = 1;
      autosize_children++;
    }
    children_width += new_width - old_width;
    UpdateScrollWidth();
  }
}

void HorizontalListBox::OnChildVisible(Widget& widget, bool visible)
{
  // the widget is being hidden or deleted

  int width = widget.Width();

  int sign = 1;
  if (!visible)
    sign = -1;

  if (width == AUTOSIZE) {
    autosize_children += sign;
    width = 1;
  }

  children_width += sign * width;
  UpdateScrollWidth();
}

void HorizontalListBox::UpdateScrollWidth()
{
  int realw = 0;
  if (scrollarea)
    realw = scrollarea->getmaxx();

  SetScrollWidth(MAX(realw, children_width));
}
