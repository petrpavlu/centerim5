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

Panel::Panel(Widget& parent, const int x, const int y, const int w, const int h)
: Widget(parent, x, y, w, h)
, linestyle(NULL)
, label(NULL)
{
	linestyle = new LineStyle(LineStyle::DEFAULT);
	label = new Label(*this, 2, 0, w - 4, 1, "");
}

Panel::Panel(Widget& parent, const int x, const int y, const int w, const int h, LineStyle::Type ltype)
: Widget(parent, x, y, w, h)
, linestyle(NULL)
, label(NULL)
{
	linestyle = new LineStyle(ltype);
	label = new Label(*this, 2, 0, w - 4, 1, "");
}

Panel::~Panel()
{
	delete linestyle;
	delete label;
}

void Panel::Draw()
{
	int realw, realh;

	if (!area || (realw = area->getmaxx()) == 0 || (realh = area->getmaxy()) == 0)
		return;

	for (int i = 1; i < realw - 1; i++) {
		area->mvaddstring(i, 0, linestyle->H());
		area->mvaddstring(i, realh - 1, linestyle->H());
	}

	for (int i = 1; i < realh - 1; i++) {
		area->mvaddstring(0, i, linestyle->V());
		area->mvaddstring(realw - 1, i, linestyle->V());
	}
	area->mvaddstring(0, 0, linestyle->CornerTL());
	area->mvaddstring(0, realh - 1, linestyle->CornerBL());
	area->mvaddstring(realw - 1, 0, linestyle->CornerTR());
	area->mvaddstring(realw - 1, realh - 1, linestyle->CornerBR());
}

void Panel::SetText(const gchar *str)
{
	label->SetText(str);
}

Glib::ustring Panel::GetText(void)
{
	return label->GetText();
}

void Panel::SetBorderStyle(LineStyle::Type ltype)
{
	linestyle->SetStyle(ltype);
}

LineStyle::Type Panel::GetBorderStyle()
{
	return linestyle->GetStyle();
}
