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

#include "GeneralMenu.h"

#include "AccountWindow.h"
#include "CenterIM.h"
#include "Log.h"

#include <libpurple/request.h>

#include "gettext.h"

GeneralMenu::GeneralMenu(int x, int y, int w, int h)
: MenuWindow(x, y, w, h)
{
	SetColorScheme("generalmenu");

	/*
	AppendItem(_("Testing"), sigc::mem_fun(this, &GeneralMenu::OpenTestWindow));
	AppendItem(_("Change status"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AppendItem(_("Go to contact..."), sigc::mem_fun(this, &GeneralMenu::Dummy));
	*/
	AppendItem(_("Accounts..."), sigc::mem_fun(this, &GeneralMenu::OpenAccountsWindow));
	AppendItem(_("Add buddy"), sigc::mem_fun(this, &GeneralMenu::OpenAddBuddyRequest));
	/*
	AppendItem(_("CenterIM config options..."), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AppendSeparator();
	AppendItem(_("Find/add users"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AppendItem(_("Join channel/conference"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AppendItem(_("Link an RSS feed"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AppendSeparator();
	AppendItem(_("View/edit ignore list"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AppendItem(_("View/edit invisible list"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AppendItem(_("View/edit visible list"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AppendSeparator();
	AppendItem(_("Show offline users"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AppendItem(_("Organize contact groups"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AppendItem(_("Mass group move..."), sigc::mem_fun(this, &GeneralMenu::Dummy));
	*/
	AppendSeparator();
	AppendItem(_("Request test"), sigc::mem_fun(this,
				&GeneralMenu::RequestTest));
	AppendSeparator();
	AppendItem(_("Quit"), sigc::mem_fun(CENTERIM, &CenterIM::Quit));
}

void GeneralMenu::ScreenResized()
{
	Rect chat = CENTERIM->ScreenAreaSize(CenterIM::ChatArea);
	MoveResize(chat.x, chat.y, win_w, win_h);
}

void GeneralMenu::OpenAccountsWindow()
{
	AccountWindow *aw = new AccountWindow;
	aw->Show();
	Close();
}

void GeneralMenu::OpenAddBuddyRequest()
{
	purple_blist_request_add_buddy(NULL, NULL, NULL, NULL);
}

void GeneralMenu::RequestTest()
{
	purple_request_input(NULL, "Title", "Primary", "Secondary",
			"default_value", FALSE, FALSE, NULL, "ok_text",
			G_CALLBACK(ok_cb_), "cancel_text", NULL, NULL, NULL, NULL, this);
}

void GeneralMenu::ok_cb(const gchar *text)
{
	LOG->Write(Log::Level_debug, "%s\n", text);
}
