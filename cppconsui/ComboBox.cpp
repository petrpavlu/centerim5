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
 * ComboBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "ComboBox.h"

#include "Keys.h"

#include "gettext.h"

#define CONTEXT_COMBOBOX "combobox"

ComboBox::ComboBoxEntry::ComboBoxEntry(const gchar *text_, const void *data_)
: data(data_)
{
	// text has to be always valid
	g_assert(text_);

	text = g_strdup(text_);
}

ComboBox::ComboBoxEntry::ComboBoxEntry(const ComboBoxEntry& other)
: data(other.data)
{
	text = g_strdup(other.text);
}

ComboBox::ComboBoxEntry::ComboBoxEntry &ComboBox::ComboBoxEntry::operator=(const ComboBoxEntry& other)
{
	if (this != &other) {
		g_free(text);
		text = g_strdup(other.text);
		data = other.data;
	}

	return *this;
}

ComboBox::ComboBoxEntry::~ComboBoxEntry()
{
	g_free(text);
}

void ComboBox::ComboBoxEntry::SetText(gchar *new_text)
{
	g_assert(new_text);

	g_free(text);
	g_strdup(new_text);
}

const gchar *ComboBox::ComboBoxEntry::GetText() const
{
	return text;
}

ComboBox::ComboBox(int w, int h, const gchar *text)
: Button(w, h, text)
, dropdown(NULL)
, selected_entry(options.end())
, max_option_width(0)
{
	signal_activate.connect(sigc::mem_fun(this, &ComboBox::OnDropDown));
	DeclareBindables();
}

ComboBox::ComboBox(const gchar *text)
: Button(text)
, dropdown(NULL)
, selected_entry(options.end())
, max_option_width(0)
{
	signal_activate.connect(sigc::mem_fun(this, &ComboBox::OnDropDown));
	DeclareBindables();
}

ComboBox::~ComboBox()
{
	// WindowManager will take care about freeing dropdown menu
}

void ComboBox::DeclareBindables()
{
	DeclareBindable(CONTEXT_COMBOBOX, "dropdown",
			sigc::mem_fun(this, &ComboBox::OnDropDown),
			InputProcessor::Bindable_Override);
}

DEFINE_SIG_REGISTERKEYS(ComboBox, RegisterKeys);
bool ComboBox::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_COMBOBOX, "dropdown", _("Show the dropdown menu."),
			Keys::SymbolTermKey(TERMKEY_SYM_ENTER));
	return true;
}

void ComboBox::SetOptions(const ComboBoxEntries& new_options)
{
	ClearOptions();

	for (ComboBoxEntries::const_iterator i = new_options.begin(); i != new_options.end(); i++)
		AddOption(*i);
}

void ComboBox::ClearOptions()
{
	options.clear();
	selected_entry = options.end();
	max_option_width = 0;
}

void ComboBox::AddOption(const gchar *text, const void *data)
{
	AddOption(ComboBoxEntry(text, data));
}

void ComboBox::AddOption(const ComboBoxEntry& option)
{
	int w = ::width(option.GetText());
	if (w > max_option_width)
		max_option_width = w;

	options.push_back(option);
}

const ComboBox::ComboBoxEntry *ComboBox::GetSelected() const
{
	if (selected_entry == options.end())
		return NULL;
	return &(*selected_entry);
}

void ComboBox::SetSelected(void *data)
{
}

void ComboBox::OnDropDown()
{
	/// @todo Position correctly according to absolute coords.
	/// @todo Make sure that requested MenuWindow size can fit into the screen.
	dropdown = new MenuWindow(0, 0, max_option_width + 2, options.size() + 2);
	dropdown->signal_close.connect(sigc::mem_fun(this, &ComboBox::DropDownClose));

	for (ComboBoxEntries::const_iterator i = options.begin(); i != options.end(); i++)
		dropdown->AppendItem(i->GetText(), sigc::bind(sigc::mem_fun(this, &ComboBox::DropDownOk), i));

	dropdown->Show();
}

void ComboBox::DropDownOk(ComboBoxEntries::const_iterator new_entry)
{
	dropdown->Close();
	dropdown = NULL;

	// selected option didn't change
	if (selected_entry == new_entry)
		return;

	selected_entry = new_entry;
	SetText(new_entry->GetText());
	signal_selection_changed(*new_entry);
}

void ComboBox::DropDownClose(FreeWindow& window)
{
	dropdown = NULL;
}
