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

#include "GeneralMenu.h"
#include "AccountWindow.h"

//TODO remove testing of dialogs
#include <cppconsui/InputDialog.h>

#include <cppconsui/WindowManager.h>

GeneralMenu::GeneralMenu(int x, int y, int w, int h, LineStyle *linestyle)
: MenuWindow(x, y, w, h, linestyle)
{
	AddItem(_("Testing"), sigc::mem_fun(this, &GeneralMenu::OpenTestWindow));
	AddItem(_("Change status"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("Go to contact..."), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("Accounts..."), sigc::mem_fun(this, &GeneralMenu::OpenAccountsWindow));
	AddItem(_("CenterIM config options..."), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddSeperator();
	AddItem(_("Find/add users"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("Join channel/conference"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("Link an RSS feed"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddSeperator();
	AddItem(_("View/edit ignore list"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("View/edit invisible list"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("View/edit visible list"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddSeperator();
	AddItem(_("Show offline users"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("Organize contact groups"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("Mass group move..."), sigc::mem_fun(this, &GeneralMenu::Dummy));
}

GeneralMenu::~GeneralMenu()
{
}

void GeneralMenu::OpenTestWindow(void)
{
	WindowManager *wm = WindowManager::Instance();
	InputDialog *dialog = new InputDialog("Try typing something!", "Type in the area.");
	dialog->Flags(TextEntry::FlagNumeric);
	wm->Add(dialog);
	Close();
}

void GeneralMenu::OpenAccountsWindow(void)
{
	//TODO adding to the windowmanager should be done by the windows themselves?
	WindowManager *wm = WindowManager::Instance();
	wm->Add(new AccountWindow());
	Close();
}
