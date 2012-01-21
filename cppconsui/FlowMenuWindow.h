/*
 * Copyright (C) 2012 by CenterIM developers
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Softwareee the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * */

/**
 * @file
 * FlowMenuWindow class.
 *
 * @ingroup cppconsui
 */

#ifndef __FLOWMENUWINDOW_H__
#define __FLOWMENUWINDOW_H__

#include "MenuWindow.h"

namespace CppConsUI
{

/**
 * FlowMenuWindow is a MenuWindow that follows a reference widget.
 */
class FlowMenuWindow
: public MenuWindow
{
public:
  FlowMenuWindow(Widget& ref_, int w, int h, const char *title = NULL);
  virtual ~FlowMenuWindow() {}

  // Widget
  virtual void Draw();

  virtual void SetRef(Widget& ref_);

  virtual int GetLeftShift() const { return xshift; }
  virtual int GetTopShift() const { return yshift; }

  virtual void SetLeftShift(int x);
  virtual void SetTopShift(int y);

protected:
  Widget *ref;
  int xshift, yshift;

  sigc::connection ref_visible_conn;

  // MenuWindow
  virtual void UpdateSmartPositionAndSize();

  virtual void OnRefVisible(Widget& activator, bool visible);

private:
  FlowMenuWindow(const FlowMenuWindow&);
  FlowMenuWindow& operator=(const FlowMenuWindow&);
};

} // namespace CppConsUI

#endif // __FLOWMENUWINDOW_H__

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
