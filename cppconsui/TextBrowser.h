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

//TODO add get/set for follow (and pos?)
class TextBrowser
: public Widget
{
	public:
		TextBrowser(int x, int y, int w, int h);
		TextBrowser(int x, int y, int w, int h, std::vector<Glib::ustring> &lines);
		virtual ~TextBrowser();

		//TODO remove hackish interface and add nice functions
		void SetLines(std::vector<Glib::ustring> &lines);
		void AddLines(std::vector<Glib::ustring> &lines);
		//TODO add some nice functions for adding lines like for the log window
		void AddLine(Glib::ustring line);
		void AddBytes(const char *s, int bytes);
		void Clear(void);
		int Size(void);

		void RemoveFront(void);
		void RemoveBack(void);

		Glib::ustring AsString(Glib::ustring seperator = "\n");

		virtual void Draw(void);

	protected:
		std::vector<Glib::ustring> lines;
		int pos;
		bool follow;

	private:
		TextBrowser();
		TextBrowser(const TextBrowser&);

		TextBrowser& operator=(const TextBrowser&);
};

#endif /* __TEXT_BROWSER_H__ */
