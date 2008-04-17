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

#include "Curses.h"
#include "Button.h"
#include "Keys.h"

Button::Button(Widget& parent, int x, int y, int w, int h, Glib::ustring &text, sigc::slot<void> function)
: Label(parent, x, y, w, h, text)
, function(function)
{
	canfocus = true;
	AddBindables();
}

Button::Button(Widget& parent, int x, int y, int w, int h, const char *text, sigc::slot<void> function)
: Label(parent, x, y, w, h, text)
, function(function)
{
	canfocus = true;
	AddBindables();
}

Button::Button(Widget& parent, int x, int y, const char *text, sigc::slot<void> function)
: Label(parent, x, y, text)
, function(function)
{
	canfocus = true;
	AddBindables();
}

Button::~Button()
{
}

void Button::Draw(void)
{
	if (focus)
		colorscheme->On(area, ColorScheme::Focus);

	Label::Draw();

	if (focus)
		colorscheme->Off(area, ColorScheme::Focus);
}


void Button::AddBindables(void)
{
	const gchar *context = "button";

	//DeclareBindable(context, "push", sigc::mem_fun(this, &Window::Close),
	DeclareBindable(context, "push", function,
		_("Push the button"), InputProcessor::Bindable_Normal);

	//TODO get real binding from config
	BindAction(context, "push", Keys::Instance()->Key_enter(), false);
}
