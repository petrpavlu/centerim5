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

#ifndef __MENUWINDOW_H__
#define __MENUWINDOW_H__

#include "MenuWindow.h"
#include "Window.h"
#include "Panel.h"
#include "ListBox.h"

#include <glibmm/ustring.h>

class MenuWindow
: public Window
{
	public:
		MenuWindow(int x, int y, int w, int h, LineStyle::Type ltype);
		virtual ~MenuWindow();
	
		virtual void Resize(int neww, int newh);

		void AddItem(const char *text, sigc::slot<void> callback)
			{ listbox->AddItem(text, callback); }
		void AddSeparator()
			{ listbox->AddSeparator(); }

		void AddWidget(Widget *widget)
			{ listbox->AddWidget(widget); }
		void RemoveWidget(Widget *widget)
			{ listbox->RemoveWidget(widget); }

		ListBox* GetListBox(void)
			{ return listbox; }

	protected:
		Panel *border;
		ListBox *listbox;

	private:
		MenuWindow();
		MenuWindow(const MenuWindow&);

		MenuWindow& operator=(const MenuWindow&);
	
		/** it handles the automatic registration of defined keys */
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
	
};

#endif /* __MENUWINDOW_H__ */
