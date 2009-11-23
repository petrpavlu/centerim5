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

/** @file ComboBox.h ComboBox class
 * @ingroup cppconsui
 */

#ifndef __COMBOBOX_H__
#define __COMBOBOX_H__

#include "TextEntry.h"

#include "MenuWindow.h"

#include <vector>

/** This class should be used when the user must choose 
 * one value from several options. 
 * It is derived from TextEntry, as it's display text can change.
 * @todo not sure if it should be derived from TextEntry, as the 
 *  functionality of a TextEntry is not used here (except for the fact 
 *  that the label text could change).
 *  A better solution could be to modify the Label class so that it's text 
 *  may change, and derive directly from there.
 *  Also, the text as the default value for label is not good to be given
 *  to the constructor, it should be chosen as the first value from the 
 *  options, or a selected value.
 */
class ComboBox
: public TextEntry
{
	public:
		/** Keeps a pair of {display text, value}
		 * @todo handle memory allocation and destruction
		 *   or make sure the pointers are not destroyed 
		 *   while being used by the combo box
		 * @todo add constructor to this struct to build it easier
		 */
		typedef struct {
			const gchar *text;
			const void *data;
		} ComboBoxEntry;
		typedef std::vector<ComboBoxEntry> ComboBoxEntries;

		ComboBox(Widget& parent, int x, int y, int w, int h, const gchar *text);
		ComboBox(Widget& parent, int x, int y, const gchar *text);

		virtual ~ComboBox();
		/** @todo maybe call these methods SetOptions and GetOptions
		 * also, use references instead of value as parameter/return values
		 */
		void Options(const ComboBoxEntries options);
		const ComboBoxEntries Options(void) { return options; }
		/** @todo add option given as ComboBoxEntry ? 
		 */
		void AddOption(const gchar *text, const void *data);
		/** @todo return reference ? 
		 */
		ComboBoxEntry GetSelected(void) { return selected_entry; }
		/** @todo implement this */
		void SetSelected(void *data);

		sigc::signal<void, const ComboBox*,
			const ComboBoxEntry, const ComboBoxEntry> signal_selection_changed;

	protected:
		MenuWindow *dropdown;
		/** Prepares and displays the dropdown MenuWindow 
		 */
		void OnDropDown(void);
		/** @todo use references ? */
		void DropDownOk(const ComboBox *combo_box, ComboBoxEntry new_entry);
		void DropDownClose(Window *window);
		
		/** @todo implement this as an iterator of ComboBoxEntries ? */
		ComboBoxEntry selected_entry;
		/** all options */
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
