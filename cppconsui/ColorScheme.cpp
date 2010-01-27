/*
 * Copyright (C) 2008 by Mark Pustjens <pustjens@dds.nl>
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

#include "ColorScheme.h"

ColorScheme::ColorScheme()
{
	for (int i = 0; i <= Disabled; i++) {
		schemes[i] = 0;
	}
}

ColorScheme::ColorScheme(const ColorScheme *colorscheme)
{
	for (int i = 0; i <= Disabled; i++) {
		schemes[i] = colorscheme->schemes[i];
	}
}

ColorScheme::~ColorScheme(void)
{
}

ColorScheme* ColorScheme::ColorSchemeNormal(void)
{
	ColorScheme *scheme = new ColorScheme();
	scheme->schemes[Normal]   = Curses::Attr::A_NORMAL_rename();
	scheme->schemes[Focus]    = Curses::Attr::A_REVERSE_rename();
	scheme->schemes[Disabled] = Curses::Attr::A_DIM_rename();
	return scheme;
}

void ColorScheme::On(Curses::Window *area, const ColorType type)
{
	Curses::wattron(area, schemes[type]);
}

void ColorScheme::Off(Curses::Window *area, const ColorType type)
{
	Curses::wattroff(area, schemes[type]);
}

void ColorScheme::SetColor(Curses::Window *area, const int x, const int y, const int n, const ColorType type)
{
	Curses::mvwchgat(area, y, x, n, schemes[type], 0, NULL);
}

