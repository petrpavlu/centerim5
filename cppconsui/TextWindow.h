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

#ifndef __TEXTWINDOW_H__
#define __TEXTWINDOW_H__

#include "TextBrowser.h"
#include "Window.h"

#include <glibmm/ustring.h>

class TextWindow
: public Window
{
	public:
		TextWindow(int x, int y, int w, int h, Border *border);
		virtual ~TextWindow();

		void SetLines(std::vector<Glib::ustring> &lines);
		void AddLines(std::vector<Glib::ustring> &lines);
		void AddLine(Glib::ustring line); //TODO could be more effecient with reference to string?
		void Clear(void);
		int Size(void);

		void RemoveFront(void);
		void RemoveBack(void);

		virtual void Resize(int neww, int newh);

	protected:
		TextBrowser *browser;

	private:
		TextWindow();
		TextWindow(const TextWindow&);

		TextWindow& operator=(const TextWindow&);
};

#endif /* __TEXTWINDOW_H__ */
