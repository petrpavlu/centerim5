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

/** @file ComboBox.cpp ComboBox class implementation
 * @ingroup cppconsui
 */

#include "ComboBox.h"

#include "Keys.h"
#include "Button.h"

/// @todo remove when show() of window/dialog is implemented
#include "WindowManager.h"

#include <glib.h>
#include <glib/gprintf.h>

#include <string.h>

ComboBox::ComboBox(Widget& parent, int x, int y, int w, int h, const gchar *text)
: TextEntry(parent, x, y, w, h, text)
, dropdown(NULL)
{
	selected_entry.text = NULL;
	selected_entry.data = NULL;

	DeclareBindables();
	BindActions();
}

ComboBox::ComboBox(Widget& parent, int x, int y, const gchar *text)
: TextEntry(parent, x, y, text)
, dropdown(NULL)
{
	selected_entry.text = NULL;
	selected_entry.data = NULL;

	DeclareBindables();
	BindActions();
}

ComboBox::~ComboBox()
{
}

void ComboBox::OnDropDown(void)
{
	ComboBoxEntries::iterator i;

	if (dropdown)
		{}/*TODO this shoudn't happen*/

	/// @todo position correctly according to absolute coords.
	dropdown = new MenuWindow(0, 0, w, options.size()+2, LineStyle::LineStyleDefault());
	sig_dropdown_close = dropdown->signal_close.connect(sigc::mem_fun(this, &ComboBox::DropDownClose));

	for (i = options.begin(); i != options.end(); i++) {
		dropdown->AddItem((*i).text, sigc::bind(sigc::mem_fun(this, &ComboBox::DropDownOk), this, *i));
	}

	/// @todo implement show method of dialogs/windows
	//dropdown->Show();
	WindowManager *wm = WindowManager::Instance();
	wm->Add(dropdown);
}

void ComboBox::DropDownOk(const ComboBox *combo_box, ComboBoxEntry new_entry)
{
	SetText(new_entry.text); /// @todo call SetText only if really changed
	ComboBoxEntry old_entry;

	old_entry = selected_entry;
	selected_entry = new_entry;

	if (old_entry.data != new_entry.data)
		signal_selection_changed(this, old_entry, new_entry);

	dropdown->Close();
}

void ComboBox::DropDownClose(Window *window)
{
	dropdown = NULL;
}

void ComboBox::Options(const ComboBoxEntries options)
{
	this->options = options;
}

void ComboBox::AddOption(const gchar *text, const void *data)
{
	ComboBoxEntry entry;

	entry.text = text;
	entry.data = data;

	options.push_back(entry);
}

void ComboBox::SetSelected(void *data)
{
	ComboBoxEntries::iterator i;
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
