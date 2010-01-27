/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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

/** @file Panel.cpp Panel class implementation
 * @ingroup cppconsui
 */

#include "ConsuiCurses.h"
#include "Panel.h"
#include "LineStyle.h"
#include "Widget.h"

Panel::Panel(Widget& parent, const int x, const int y, const int w, const int h)
: Widget(parent, x, y, w, h)
, LineStyle(LineStyle::DEFAULT)
, label(NULL)
{
	label = new Label(*this, 2, 0, w-4, 1, "");
}

Panel::Panel(Widget& parent, const int x, const int y, const int w, const int h, LineStyle::Type ltype)
: Widget(parent, x, y, w, h)
, LineStyle(ltype)
, label(NULL)
{
	label = new Label(*this, 2, 0, w-4, 1, "");
}

Panel::~Panel()
{
	delete label;
}

void Panel::Draw()
{
	if (!area || Width() == 0 || Height() == 0)
		return; //TODO and throw an exception/log a warning?

	// @todo optimize
	for (int i = 1; i < Width() - 1; i++) {
		Curses::mvwaddstring(area, 0, i, 1, H());
		Curses::mvwaddstring(area, Height() - 1, i, 1, H());
	}

	for (int i = 1; i < Height() - 1; i++) {
		Curses::mvwaddstring(area, i, 0, 1, V());
		Curses::mvwaddstring(area, i, Width() - 1, 1, V());
	}
	Curses::mvwaddstring(area, 0, 0, 1, CornerTL());
	Curses::mvwaddstring(area, Height() - 1, 0, 1, CornerBL());
	Curses::mvwaddstring(area, 0, Width() - 1, 1, CornerTR());
	Curses::mvwaddstring(area, Height() - 1, Width() - 1, 1, CornerBR());
}

void Panel::SetText(const gchar *str)
{
	label->SetText(str);
}

Glib::ustring Panel::GetText(void)
{
	return label->GetText();
}
