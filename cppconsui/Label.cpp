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
 * Label class implementation.
 *
 * @ingroup cppconsui
 */

#include "Label.h"

#include "ConsuiCurses.h"
#include "ColorScheme.h"

Label::Label(int w, int h, const gchar *text_)
: Widget(w, h)
, text(NULL)
{
	RealSetText(text_);
}

Label::Label(const gchar *text_)
: Widget(AUTOSIZE, 1)
, text(NULL)
{
	RealSetText(text_);
}

Label::~Label()
{
	g_free(text);
}

void Label::Draw()
{
	DrawEx(true);
}

void Label::DrawEx(bool color)
{
	if (!area)
		return;

	/**
	 * @todo Though this is not a widget for long text there are some cases in
	 * cim where we use it for a short but multiline text, so we should threat
	 * LF specially here.
	 */

	area->erase();

	int attrs;
	if (color) {
		attrs = COLORSCHEME->GetColorPair(GetColorScheme(), "label", "text");
		area->attron(attrs);
	}

	int max = area->getmaxx() * area->getmaxy();
	area->mvaddstring(0, 0, max, text);

	if (color)
		area->attroff(attrs);
}

void Label::SetText(const gchar *text_)
{
	RealSetText(text_);

	signal_redraw(*this);
}

const gchar *Label::GetText() const
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
