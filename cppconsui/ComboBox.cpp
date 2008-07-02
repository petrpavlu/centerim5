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

#include "ComboBox.h"

#include "Keys.h"
#include "Button.h"

//TODO remove when show() of window/dialog is implemented
#include "WindowManager.h"

#include <glib.h>
#include <glib/gprintf.h>

#include <string.h>

ComboBox::ComboBox(Widget& parent, int x, int y, int w, int h, const gchar *text)
: TextEntry(parent, x, y, w, h, text)
{
	DeclareBindables();
	BindActions();
}

ComboBox::ComboBox(Widget& parent, int x, int y, const gchar *text)
: TextEntry(parent, x, y, text)
{
	DeclareBindables();
	BindActions();
}

ComboBox::~ComboBox()
{
}

void ComboBox::OnDropDown(void)
{
	std::vector<const gchar*>::iterator i;

	if (dropdown)
		/*TODO this shoudn't happen*/;

	dropdown = new MenuWindow(x, y, w, options.size()+2, LineStyle::LineStyleDefault());
	sig_dropdown_close = dropdown->signal_close.connect(sigc::mem_fun(this, &ComboBox::DropDownClose));

	for (i = options.begin(); i != options.end(); i++) {
		dropdown->AddItem(*i, sigc::bind(sigc::mem_fun(this, &ComboBox::DropDownOk), *i));
	}

	//TODO implement show method of dialogs/windows
	//dropdown->Show();
	WindowManager *wm = WindowManager::Instance();
	wm->Add(dropdown);
}

void ComboBox::DropDownOk(const gchar *selection)
{
	SetText(selection);
	dropdown->Close();
}

void ComboBox::DropDownClose(Window *window)
{
	dropdown = NULL;
}

void ComboBox::Options(std::vector<const gchar*> _options)
{

	options.clear();
	options = _options;
}
void ComboBox::DeclareBindables(void)
{
	const gchar *context = "combobox";

	DeclareBindable(context, "dropdown", sigc::mem_fun(this, &ComboBox::OnDropDown),
		_("Show the dropdown menu."), InputProcessor::Bindable_Override);
}

void ComboBox::BindActions(void)
{
	const gchar *context = "combobox";

	BindAction(context, "dropdown", Keys::Instance()->Key_enter(), false);
}
