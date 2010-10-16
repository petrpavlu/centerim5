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

#ifndef __GENERALMENU_H__
#define __GENERALMENU_H__

#include <cppconsui/MenuWindow.h>

class GeneralMenu
: public MenuWindow
{
public:
	GeneralMenu(int x, int y, int w, int h);
	virtual ~GeneralMenu() {}

	// FreeWindow
	virtual void ScreenResized();

protected:

private:
	GeneralMenu();
	GeneralMenu(const GeneralMenu&);
	GeneralMenu& operator=(const GeneralMenu&);

	void OpenAccountsWindow(Button& activator);
	void OpenAddBuddyRequest(Button& activator);
	void RequestTest(Button& activator);

	static void input_ok_cb_(void *data, const gchar *text)
		{ reinterpret_cast<GeneralMenu *>(data)->input_ok_cb(text); }
	void input_ok_cb(const gchar *text);

	static void choice_ok_cb_(void *data, int selected)
		{ reinterpret_cast<GeneralMenu *>(data)->choice_ok_cb(selected); }
	void choice_ok_cb(int selected);
};

#endif /* __GENERALMENU_H__ */
