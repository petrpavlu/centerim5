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

#include <glib.h>
#include <glib/gprintf.h>

#include <string.h>

ComboBox::ComboBox(Widget& parent, int x, int y, int w, int h, const gchar *text)
: TextEntry(parent, x, y, w, h, text)
{
}

ComboBox::ComboBox(Widget& parent, int x, int y, const gchar *text)
: TextEntry(parent, x, y, text)
{
}

ComboBox::~ComboBox()
{
}

void ComboBox::OnDropDown(void)
{
	std::vector<const gchar*>::iterator i;

	if (dropdown)
		/*TODO this shoudn't happen*/;

	dropdown = new MenuWindow(x, y, w, options.size(), LineStyle::LineStyleDefault());
	sig_dropdown_close = dropdown->signal_close.connect(sigc::mem_fun(this, &ComboBox::DropDownClose));

	for (i = options.begin(); i != options.end(); i++) {
		dropdown->AddWidget(new Button(*dropdown, 0, 0, *i, 
				sigc::bind(sigc::mem_fun(this, &ComboBox::DropDownOk), *i)));
	}

	dropdown->Show();
}

void ComboBox::DropDownOk(const gchar *selection)
{
	SetText(selection);
}

void ComboBox::DropDownClose(Window *window)
{
	dropdown->Close();
	dropdown = NULL;
}

void ComboBox::Options(std::vector<const gchar*> _options)
{

	options.clear();
	options = _options;
}
