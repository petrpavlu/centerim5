/*
 * Copyright (C) 2008 by Mark Pustjens <pustjens@dds.nl>
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
 * ColorScheme class implementation.
 *
 * @ingroup cppconsui
 */

#include "ColorScheme.h"

ColorScheme *ColorScheme::Instance()
{
	static ColorScheme instance;
	return &instance;
}

int ColorScheme::GetColorPair(const char *scheme, const char *widget,
		const char *property)
{
	g_assert(widget);
	g_assert(property);

	if (scheme != NULL && schemes.find(scheme) != schemes.end()
			&& schemes[scheme].find(widget) != schemes[scheme].end()
			&& schemes[scheme][widget].find(property) != schemes[scheme][widget].end()) {
		Color c = schemes[scheme][widget][property];
		return Curses::getcolorpair(c.foreground, c.background) | c.attrs;
	}
	return 0;
}

bool ColorScheme::SetColorPair(const char *scheme, const char *widget,
		const char *property, int foreground, int background, int attrs,
		bool overwrite)
{
	g_assert(widget);
	g_assert(property);

	if (!overwrite && scheme != NULL && schemes.find(scheme) != schemes.end()
			&& schemes[scheme].find(widget) != schemes[scheme].end()
			&& schemes[scheme][widget].find(property) != schemes[scheme][widget].end())
		return false;

	schemes[scheme][widget][property] = Color(foreground, background, attrs);
	return true;
}
