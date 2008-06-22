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

//TODO add spaces around the label st there is always some room between the label and the left/right
//side of the button.

Button::Button(Widget& parent, int x, int y, int w, int h, const gchar *text, sigc::slot<void> callback)
: Label(parent, x, y, w, h, text)
, callback(callback)
{
	can_focus = true;
	AddBindables();
}

Button::Button(Widget& parent, int x, int y, const char *text, sigc::slot<void> callback)
: Label(parent, x, y, text)
, callback(callback)
{
	can_focus = true;
	AddBindables();
}

Button::Button(Widget& parent, int x, int y, const char *text)
: Label(parent, x, y, text)
{
	can_focus = true;
	AddBindables();
}

Button::~Button()
{
}

void Button::Draw(void)
{
	if (has_focus)
		colorscheme->On(area, ColorScheme::Focus);

	Label::Draw();

	if (has_focus)
		colorscheme->Off(area, ColorScheme::Focus);

	//TODO try setcolor, its shorter
	//colorscheme->SetColor(area, 0, 0, w, ColorScheme::Focus);
}

void Button::SetFunction(sigc::slot<void> callback)
{
	this->callback = callback;
}

void Button::AddBindables(void)
{
	const gchar *context = "button";

	//DeclareBindable(context, "push", sigc::mem_fun(this, &Window::Close),
	DeclareBindable(context, "push", sigc::mem_fun(this, &Button::OnActivate),
		_("Push the button"), InputProcessor::Bindable_Normal);

	//TODO get real binding from config
	BindAction(context, "push", Keys::Instance()->Key_enter(), false);
}

void Button::OnActivate(void)
{
	callback();
}
