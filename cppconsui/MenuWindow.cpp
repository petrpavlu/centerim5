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
 * MenuWindow class implementation.
 *
 * @ingroup cppconsui
 */

#include "MenuWindow.h"

MenuWindow::MenuWindow(int x, int y, int w, int h, LineStyle::Type ltype)
: Window(x, y, w, h, ltype)
{
	listbox = new ListBox(width, height);
	Window::AddWidget(*listbox, 0, 0);
}

void MenuWindow::MoveResize(int newx, int newy, int neww, int newh)
{
	Window::MoveResize(newx, newy, neww, newh);

	listbox->MoveResize(0, 0, width, height);
}
