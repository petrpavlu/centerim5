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
 * AbstractListBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "AbstractListBox.h"

#include "Button.h"

AbstractListBox::AbstractListBox(int x, int y, int w, int h)
: ScrollPane(x, y, w, h, 0, 0)
{
}

void AbstractListBox::AddItem(const gchar *title, sigc::slot<void> function)
{
	AddWidget(*(new Button(0, 0, ::width(title), 1, title, function)));
}
