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

/**
 * @file
 * Window class.
 *
 * @ingroup cppconsui
 */

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "FreeWindow.h"
#include "Panel.h"

namespace CppConsUI
{

/**
 * Window class is the class implementing the root node of the Widget chain
 * defined by Widget::parent.
 *
 * It handles the drawing of its pad and subpads of its children to a @ref
 * realwindow "real window".
 */
class Window
: public FreeWindow
{
public:
  Window(int x, int y, int w, int h, const char *title = NULL,
      Type t = TYPE_NORMAL);
  virtual ~Window() {}

  // Container
  virtual Point getAbsolutePosition(const Container& ref,
      const Widget& child) const;
  virtual Point getAbsolutePosition(const Widget& child) const;
  virtual Curses::Window *getSubPad(const Widget &child, int begin_x,
      int begin_y, int ncols, int nlines);

  virtual void setTitle(const char *text) { panel->setTitle(text); }
  virtual const char *getTitle() const { return panel->getTitle(); }

protected:
  Panel *panel;

  // FreeWindow
  virtual void updateContainer(int realw, int realh);

private:
  CONSUI_DISABLE_COPY(Window);
};

} // namespace CppConsUI

#endif // __WINDOW_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
