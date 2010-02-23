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
/** @file Button.cpp Button class implementation
 * @ingroup cppconsui
 */

#include "ConsuiCurses.h"
#include "Button.h"
#include "Keys.h"
#include "CppConsUIInternal.h"
#include "ColorScheme.h"

#define CONTEXT_BUTTON "button"

/// @todo add spaces around the label st there is always some room between the label and the left/right
/// side of the button.

Button::Button(Widget& parent, int x, int y, int w, int h, const gchar *text, sigc::slot<void> callback)
: Label(parent, x, y, w, h, text)
, callback(callback)
{
	can_focus = true;
	DeclareBindables();
}

Button::Button(Widget& parent, int x, int y, int w, int h, const gchar *text)
: Label(parent, x, y, w, h, text)
{
	can_focus = true;
	DeclareBindables();
}

Button::Button(Widget& parent, int x, int y, const gchar *text, sigc::slot<void> callback)
: Label(parent, x, y, text)
, callback(callback)
{
	can_focus = true;
	DeclareBindables();
}

Button::Button(Widget& parent, int x, int y, const gchar *text)
: Label(parent, x, y, text)
{
	can_focus = true;
	DeclareBindables();
}

Button::~Button()
{
}

void Button::DeclareBindables(void)
{
	DeclareBindable(CONTEXT_BUTTON, "activate", _("Activate the button"),
			sigc::mem_fun(this, &Button::OnActivate),
			InputProcessor::Bindable_Normal);
}

DEFINE_SIG_REGISTERKEYS(Button, RegisterKeys);
bool Button::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_BUTTON, "activate", Keys::SymbolTermKey(TERMKEY_SYM_ENTER));
	return true;
}

void Button::Draw(void)
{
	if (!area)
		return;

	int attrs;
	if (has_focus) {
		attrs = COLORSCHEME->GetColorPair(GetColorScheme(), "button", "focus");
		area->attron(attrs | Curses::Attr::REVERSE);
	}
	else {
		attrs = COLORSCHEME->GetColorPair(GetColorScheme(), "button", "normal");
		area->attron(attrs);
	}


	Label::DrawEx(false);

	if (has_focus)
		area->attroff(attrs | Curses::Attr::REVERSE);
	else
		area->attroff(attrs);
}

void Button::SetFunction(sigc::slot<void> callback)
{
	this->callback = callback;
}

void Button::OnActivate(void)
{
	signal_activate(this);
	callback();
}
