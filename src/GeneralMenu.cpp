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
#include "CIMWindowManager.h"

//TODO remove testing stuff
#include "Log.h"
#include <cppconsui/MessageDialog.h>
#include <cppconsui/ComboBox.h>
#include <cppconsui/TextView.h>

//TODO remove when show() of window/dialog is implemented
#include <cppconsui/WindowManager.h>

GeneralMenu::GeneralMenu(int x, int y, int w, int h, LineStyle *linestyle)
: MenuWindow(x, y, w, h, linestyle)
{
	AddItem(_("Testing"), sigc::mem_fun(this, &GeneralMenu::OpenTestWindow));
	AddItem(_("Change status"), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("Go to contact..."), sigc::mem_fun(this, &GeneralMenu::Dummy));
	AddItem(_("Accounts..."), sigc::mem_fun(this, &GeneralMenu::OpenAccountsWindow));
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

	//TODO remove testing stuff
	ComboBox *combobox = new ComboBox(*(this->GetListBox()), 0, 0, w-2, 1, "Combo Test!");
	std::vector<const gchar*> options;
        combobox->AddOption("onne", NULL);
        combobox->AddOption("twoo", NULL);
        combobox->AddOption("theree", NULL);
	AddWidget(combobox);
}

GeneralMenu::~GeneralMenu()
{
}

void GeneralMenu::OpenTestWindow(void)
{
	TextView view(*this, 0, 0, w, h);
	TextIter start_iter, end_iter, insert;
	Rect r;
	Rect c;
	Log* log = Log::Instance();

	Glib::ustring s("the foo was bar in the spring.");

	view.insert_at_cursor(s.c_str());
	view.insert_at_cursor(s.c_str());

	view.get_visible_rect(&r);
	view.get_cursor_location(&c);
	log->Write(Log::Type_cim, Log::Level_debug, "rect: %d %d %d %d cursor: %d %d %d %d (x,y,w,h)", 
			r.Left(), r.Top(), r.Width(), r.Height(),
			c.Left(), c.Top(), c.Width(), c.Height());


	view.select_all(true);
	view.backspace();

	view.get_visible_rect(&r);
	view.get_cursor_location(&c);
	log->Write(Log::Type_cim, Log::Level_debug, "rect: %d %d %d %d cursor: %d %d %d %d", 
			r.Left(), r.Top(), r.Width(), r.Height(),
			c.Left(), c.Top(), c.Width(), c.Height());


	/*
	WindowManager *wm = WindowManager::Instance();
	MessageDialog *dialog = new MessageDialog("Message.");
	wm->Add(dialog);*/
	Close();
}

void GeneralMenu::OpenAccountsWindow(void)
{
	//TODO adding to the windowmanager should be done by the windows themselves?
	WindowManager *wm = WindowManager::Instance();
	wm->Add(new AccountWindow());
	Close();
}

void GeneralMenu::ScreenResized()
{
	Rect chat = (CIMWindowManager::Instance())->ScreenAreaSize(CIMWindowManager::Chat);
	Move(chat.x, chat.y);
}
