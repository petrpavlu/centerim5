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

#include <cstring>

#include <glib.h>
#include <glib/gprintf.h>

Label::Label(Widget& parent, int x, int y, int w, int h, const char *text_)
: Widget(parent, x, y, w, h)
, text_max_length(MAX_SIZE)
, text(NULL)
{
	_SetText(text_);
}

Label::Label(Widget& parent, int x, int y, const char *text_)
: Widget(parent, x, y, 0, 0)
, text_max_length(MAX_SIZE)
, text(NULL)
{
	_SetText(text_);
	Resize(width(text), 1);
}

Label::~Label()
{
	g_free(text); // it is always allocated
}

void Label::Draw(void)
{
	//TODO unicode drawing
	//or draw with current locale?
	mvwaddstr(area->w, 0, 0, text);
	wclrtoeol(area->w);

	Widget::Draw();
}

void Label::SetText(const gchar *text_)
{
	g_free(text); // it always needs to be allocated
	_SetText(text_);

	wclear(area->w);
	signal_redraw(this);
}

const gchar* Label::GetText(void)
{
	return text;
}

void Label::_SetText(const gchar *text_)
{
	assert(text_); 
	//@TODO handle reallocation
	text = g_strndup(text_,text_max_length);
	text_size = strlen(text);
	n_bytes = text_size;
	text_length = g_utf8_strlen(text, text_size);
}