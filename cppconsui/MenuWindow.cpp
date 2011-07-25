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
 * MenuWindow class implementation.
 *
 * @ingroup cppconsui
 */

#include "MenuWindow.h"

namespace CppConsUI
{

MenuWindow::MenuWindow(int x, int y, int w, int h, const char *title,
    LineStyle::Type ltype)
: Window(x, y, w, h, title, TYPE_TOP, ltype), wish_height(3)
{
  wish_width = MENU_WINDOW_WISH_WIDTH;

  listbox = new ListBox(AUTOSIZE, AUTOSIZE);
  listbox->signal_children_height_change.connect(sigc::mem_fun(this,
        &MenuWindow::OnChildrenHeightChange));
  Window::AddWidget(*listbox, 0, 0);
}

void MenuWindow::AddWidget(Widget& widget, int x, int y)
{
  Window::AddWidget(widget, x, y);
}

void MenuWindow::ResizeAndUpdateArea()
{
  int h = listbox->GetChildrenHeight() + 2;
  int max = Curses::getmaxy() - win_y;
  if (h > max)
    SetWishHeight(MAX(3, max));
  else
    SetWishHeight(h);

  Window::ResizeAndUpdateArea();
}

void MenuWindow::OnChildrenHeightChange(ListBox& activator, int new_height)
{
  if (win_h != AUTOSIZE)
    return;

  ResizeAndUpdateArea();
}

} // namespace CppConsUI
