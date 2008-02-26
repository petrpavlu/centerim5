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

#include "AccountsWindow.h"

#include <cppconsui/Keys.h>

AccountsWindow::AccountsWindow()
: Window(0, 0, 80, 24, NULL)
{
	log = Log::Instance();
	conf = Conf::Instance();

	MoveResize(conf->GetAccountsWindowDimensions());

	//TODO get linestyle from conf
	border = new Panel(*this,  LineStyle::LineStyleDefault(), 0, 0, w, h);
	accounts = new ListBox(*this, 1, 1, w-2, h-4);
	menu = new HorizontalListBox(*this, 1, h-2, w-2, 1);
	line = new HorizontalLine(*this, 1, h-3, w-2);

	menu->AddItem(_("Add"), sigc::mem_fun(this, &AccountsWindow::Add));
	menu->AddSeperator();
	menu->AddItem(_("Edit"), sigc::mem_fun(this, &AccountsWindow::Edit));
	menu->AddSeperator();
	menu->AddItem(_("Delete"), sigc::mem_fun(this, &AccountsWindow::Delete));
	menu->AddSeperator();

	AddWidget(border);
	AddWidget(accounts);
	AddWidget(menu);
	AddWidget(line);
	SetInputChild(accounts);

	Populate();
}

AccountsWindow::~AccountsWindow()
{
}

void AccountsWindow::Populate(void)
{
}

void AccountsWindow::Add(void)
{
}

void AccountsWindow::Edit(void)
{
}

void AccountsWindow::Delete(void)
{
}
