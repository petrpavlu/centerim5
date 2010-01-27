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

#include "ConsuiCurses.h"
#include "Label.h"

#include <glib.h>

Label::Label(Widget& parent, int x, int y, int w, int h, const gchar *text_)
: Widget(parent, x, y, w, h)
, text(NULL)
{
	RealSetText(text_);
}

Label::Label(Widget& parent, int x, int y, const gchar *text_)
: Widget(parent, x, y, 0, 0)
, text(NULL)
{
	RealSetText(text_);
	Resize(width(text), 1);
	g_debug("label: w %d\n", w);
}

Label::~Label()
{
	g_free(text);
}

void Label::Draw()
{
	Curses::mvwaddstring(area, 0, 0, -1, text);

	// @todo why is not shown last text character with wclrtobot()
	//Curses::wclrtobot(area);
	Curses::wclrtoeol(area);

	Widget::Draw();
}

void Label::SetText(const gchar *text_)
{
	RealSetText(text_);

	signal_redraw(this);
}

const gchar* Label::GetText()
{
	return text;
}

void Label::RealSetText(const gchar *text_)
{
	g_assert(text_); 

	if (text)
		g_free(text);
	text = g_strdup(text_);
}
