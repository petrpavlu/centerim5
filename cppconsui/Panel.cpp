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
 * Panel class implementation.
 *
 * @ingroup cppconsui
 */

#include "Panel.h"

#include "ConsuiCurses.h"
#include "ColorScheme.h"

Panel::Panel(int x, int y, int w, int h, LineStyle::Type ltype)
: Widget(x, y, w, h)
, linestyle(ltype)
, label(NULL)
{
	label = new Label(2, 0, w - 4, 1, "");
}

Panel::~Panel()
{
	delete label;
}

void Panel::Draw()
{
	int realw, realh;

	if (!area || (realw = area->getmaxx()) == 0 || (realh = area->getmaxy()) == 0)
		return;

	int attrs = COLORSCHEME->GetColorPair(GetColorScheme(), "panel", "line");
	area->attron(attrs);

	// draw top and bottom horizontal line
	for (int i = 1; i < realw - 1; i++) {
		area->mvaddstring(i, 0, linestyle.H());
		area->mvaddstring(i, realh - 1, linestyle.H());
	}

	// draw left and right vertical line
	for (int i = 1; i < realh - 1; i++) {
		area->mvaddstring(0, i, linestyle.V());
		area->mvaddstring(realw - 1, i, linestyle.V());
	}

	// draw corners
	area->mvaddstring(0, 0, linestyle.CornerTL());
	area->mvaddstring(0, realh - 1, linestyle.CornerBL());
	area->mvaddstring(realw - 1, 0, linestyle.CornerTR());
	area->mvaddstring(realw - 1, realh - 1, linestyle.CornerBR());

	area->attroff(attrs);
}

void Panel::SetText(const gchar *str)
{
	label->SetText(str);
}

const gchar *Panel::GetText()
{
	return label->GetText();
}

void Panel::SetBorderStyle(LineStyle::Type ltype)
{
	linestyle.SetStyle(ltype);
	signal_redraw(*this);
}

LineStyle::Type Panel::GetBorderStyle()
{
	return linestyle.GetStyle();
}
