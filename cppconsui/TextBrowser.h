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

#ifndef __TEXT_BROWSER_H__
#define __TEXT_BROWSER_H__

#include "Widget.h"

#include <glibmm/ustring.h>
#include <vector>

class TextBrowser
: public Widget
{
	public:
		TextBrowser(WINDOW* parentarea, int x, int y, int w, int h);
		TextBrowser(WINDOW* parentarea, int x, int y, int w, int h, std::vector<Glib::ustring> &lines);
		~TextBrowser();

		void SetLines(std::vector<Glib::ustring> &lines);
		void AddLines(std::vector<Glib::ustring> &lines);
		void AddLine(Glib::ustring &line);
		void Clear(void);
		int Size(void);

		void RemoveFront(void);
		void RemoveBack(void);

		virtual void Draw(void);

	protected:
		std::vector<Glib::ustring> lines;
		int pos;
		bool follow;

	private:
		TextBrowser();
};

#endif /* __TEXT_BROWSER_H__ */
