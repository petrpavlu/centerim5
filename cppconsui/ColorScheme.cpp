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

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif


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
	scheme->schemes[Normal]   = A_NORMAL;
	scheme->schemes[Focus]    = A_REVERSE;
	scheme->schemes[Disabled] = A_DIM;
	return scheme;
}

void ColorScheme::On(const curses_imp_t* area, const ColorType type)
{
	wattron(area->w, schemes[type]);
}

void ColorScheme::Off(const curses_imp_t* area, const ColorType type)
{
	wattroff(area->w, schemes[type]);
}

void ColorScheme::SetColor(const curses_imp_t* area, const int x, const int y, const int n, const ColorType type)
{
	mvwchgat(area->w, y, x, n, schemes[type], 0, NULL);
}
