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

#ifndef __COMBOBOX_H__
#define __COMBOBOX_H__

#include "TextEntry.h"

#include "MenuWindow.h"

#include <vector>

class ComboBox
: public TextEntry
{
	public:
		ComboBox(Widget& parent, int x, int y, int w, int h, const gchar *text);
		ComboBox(Widget& parent, int x, int y, const gchar *text);

		virtual ~ComboBox();

		void Options(std::vector<const gchar*> options);
		std::vector<const gchar*> Options(void) { return options; }

	protected:
		MenuWindow *dropdown;

		void OnDropDown(void);
		void DropDownOk(const gchar *selection);
		void DropDownClose(Window *window);

		std::vector<const gchar*> options;

	private:
		ComboBox();
		ComboBox(const ComboBox&);

		ComboBox& operator=(const ComboBox&);

		sigc::connection sig_dropdown_close;
};

#endif /* __COMBOBOX_H__ */
