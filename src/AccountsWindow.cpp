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
#include <cppconsui/Window.h>
#include <cppconsui/TreeView.h>

#include <cppconsui/Label.h>

AccountsWindow* AccountsWindow::instance = NULL;

AccountsWindow* AccountsWindow::Instance(void)
{
	if (!instance) instance = new AccountsWindow();
	return instance;
}

void AccountsWindow::Delete(void)
{
	if (instance) {
		delete instance;
		instance = NULL;
	}
}

AccountsWindow::AccountsWindow()
: Window(0, 0, 80, 24, NULL)
{
	log = Log::Instance();
	conf = Conf::Instance();

	MoveResize(conf->GetAccountsWindowDimensions());

	//TODO get linestyle from conf
	border = new Panel(*this,  LineStyle::LineStyleDefault(), 0, 0, w, h);
	treeview = new TreeView(*this, 1, 1, w-2, h-2, LineStyle::LineStyleDefault());
	AddWidget(border);
	AddWidget(treeview);
	SetInputChild(treeview);
}

AccountsWindow::~AccountsWindow()
{
}
