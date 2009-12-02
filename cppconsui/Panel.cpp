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
, LineStyle(LineStyleDefault())
, label(NULL)
{
	label = new Label(*this, 2, 0, w-4, 1, "");
}

Panel::Panel(Widget& parent, const int x, const int y, const int w, const int h, LineStyle *linestyle)
: Widget(parent, x, y, w, h)
, LineStyle(linestyle)
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
	if (!area->w || Width() == 0 || Height() == 0)
		return; //TODO and throw an exception/log a warning?


	for (int i = 1; i < Width()-1; i++) {
		mvwadd_wch(area->w, 0, i, H());
		mvwadd_wch(area->w, Height()-1, i, H());
	}

	for (int i = 1; i < Height()-1; i++) {
		mvwadd_wch(area->w, i, 0, V());
		mvwadd_wch(area->w, i, Width()-1, V());
	}
	mvwadd_wch(area->w, 0, 0, CornerTL());
	mvwadd_wch(area->w, Height()-1, 0, CornerBL());
	mvwadd_wch(area->w, 0, Width()-1, CornerTR());
	mvwadd_wch(area->w, Height()-1, Width()-1, CornerBR());
}

void Panel::SetText(const gchar *str)
{
	label->SetText(str);
}

Glib::ustring Panel::GetText(void)
{
	return label->GetText();
}
