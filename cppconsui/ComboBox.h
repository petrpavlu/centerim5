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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Softwareee the
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
		typedef struct {
			const gchar *text;
			const void *data;
		} ComboBoxEntry;
		typedef std::vector<ComboBoxEntry> ComboBoxEntries;

		ComboBox(Widget& parent, int x, int y, int w, int h, const gchar *text);
		ComboBox(Widget& parent, int x, int y, const gchar *text);

		virtual ~ComboBox();

		void Options(const ComboBoxEntries options);
		ComboBoxEntries Options(void) { return options; }
		void AddOption(const gchar *text, const void *data);

		ComboBoxEntry GetSelected(void) { return selected_entry; }
		void SetSelected(void *data);

		sigc::signal<void, const ComboBox*,
			ComboBoxEntry, ComboBoxEntry> signal_selection_changed;

	protected:
		MenuWindow *dropdown;

		void OnDropDown(void);
		void DropDownOk(const ComboBox *combo_box, ComboBoxEntry new_entry);
		void DropDownClose(Window *window);

		ComboBoxEntry selected_entry;
		ComboBoxEntries options;

	private:
		ComboBox();
		ComboBox(const ComboBox&);

		ComboBox& operator=(const ComboBox&);

		void DeclareBindables(void);
		void BindActions(void);

		void OnActivate(void)
			{ OnDropDown(); }

		sigc::connection sig_dropdown_close;
};

#endif /* __COMBOBOX_H__ */
