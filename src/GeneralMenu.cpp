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
#include "CenterIM.h"

//TODO remove when show() of window/dialog is implemented
#include <cppconsui/WindowManager.h>
#include "gettext.h"

GeneralMenu::GeneralMenu(int x, int y, int w, int h)
: MenuWindow(x, y, w, h)
{
	SetColorScheme("generalmenu");

	/*
	AddItem(_("Testing"), sigc::mem_fun(this, &GeneralMenu::OpenTestWindow));
	AddItem(_("Change status"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("Go to contact..."), sigc::mem_fun(this, &GeneralMenu::Dummy));
	*/
	AddItem(_("Accounts..."), sigc::mem_fun(this, &GeneralMenu::OpenAccountsWindow));
	AddItem(_("Add buddy"), sigc::mem_fun(this, &GeneralMenu::OpenAddBuddyRequest));
	/*
	AddItem(_("CenterIM config options..."), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddSeparator();
	AddItem(_("Find/add users"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("Join channel/conference"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("Link an RSS feed"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddSeparator();
	AddItem(_("View/edit ignore list"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("View/edit invisible list"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("View/edit visible list"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddSeparator();
	AddItem(_("Show offline users"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("Organize contact groups"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("Mass group move..."), sigc::mem_fun(this, &GeneralMenu::Dummy));
	*/
	AddSeparator();
	AddItem(_("Quit"), sigc::mem_fun(CenterIM::Instance(), &CenterIM::Quit));
}

void GeneralMenu::ScreenResized()
{
	Rect chat = CenterIM::Instance().ScreenAreaSize(CenterIM::ChatArea);
	MoveResize(chat.x, chat.y, win_w, win_h);
}

void GeneralMenu::OpenAccountsWindow()
{
	//TODO adding to the windowmanager should be done by the windows themselves?
	WindowManager *wm = WindowManager::Instance();
	wm->Add(new AccountWindow());
	Close();
}

void GeneralMenu::OpenAddBuddyRequest()
{
	purple_blist_request_add_buddy(NULL, NULL, NULL, NULL);
}
