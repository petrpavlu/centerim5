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

#include "Label.h"

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif

Label::Label(WINDOW* parentarea, int x, int y, int w, int h, Glib::ustring text)
: Widget(parentarea, x, y, w, h)
, text(text)
{
}

Label::~Label()
{
}

void Label::Draw(void)
{
	//TODO unicode drawing
	//TODO focus stuff is for testing *only*
	if (focus) {
		mvwaddstr(area, 0, 0, "*");
		mvwaddstr(area, 0, 1, text.c_str());
	} else
		mvwaddstr(area, 0, 0, text.c_str());
}

void Label::SetText(const Glib::ustring str)
{
	if (text == str) return;

	text = str;
	wclear(area);
	signal_redraw();
}

Glib::ustring Label::GetText(void)
{
	return text;
}
