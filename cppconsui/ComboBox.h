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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Softwareee the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * */

/**
 * @file
 * ComboBox class.
 *
 * @ingroup cppconsui
 */

#ifndef __COMBOBOX_H__
#define __COMBOBOX_H__

#include "Button.h"
#include "MenuWindow.h"

#include <vector>

/**
 * This class should be used when the user must choose one value from several
 * options.
 */
class ComboBox
: public Button
{
	public:
		/**
		 * Keeps a pair of {display text, value}.
		 */
		class ComboBoxEntry
		{
			public:
				ComboBoxEntry(const gchar *text_, const void *data_);
				ComboBoxEntry(const ComboBoxEntry& other);
				ComboBoxEntry& operator=(const ComboBoxEntry& other);
				virtual ~ComboBoxEntry();

				void SetText(gchar *new_text);
				const gchar *GetText() const;

				const void *data;

			protected:

			private:
				gchar *text;
		};
		typedef std::vector<ComboBoxEntry> ComboBoxEntries;

		ComboBox(int w, int h, const gchar *text = "");
		ComboBox(const gchar *text = "");

		virtual ~ComboBox();

		/**
		 * Sets new options.
		 */
		void SetOptions(const ComboBoxEntries& new_options);
		/**
		 * Returns current options.
		 */
		const ComboBoxEntries *GetOptions() const { return &options; }
		/**
		 * Removes all options.
		 */
		void ClearOptions();

		/**
		 * Appends a new option.
		 */
		void AddOption(const gchar *text, const void *data = NULL);
		/**
		 * @overload AddOption(const gchar *text, const void *data);
		 */
		void AddOption(const ComboBoxEntry& option);

		/**
		 * Returns last selested option.
		 */
		const ComboBoxEntry *GetSelected() const;

		/** @todo implement this */
		void SetSelected(void *data);

		sigc::signal<void, const ComboBoxEntry&> signal_selection_changed;

	protected:
		MenuWindow *dropdown;
		/**
		 * Prepares and displays the dropdown MenuWindow.
		 */
		void OnDropDown();
		void DropDownOk(ComboBoxEntries::const_iterator new_entry);
		void DropDownClose(Window *window);

		/**
		 * Holds reference to currently selected entry.
		 */
		ComboBoxEntries::const_iterator selected_entry;
		/**
		 * All options.
		 */
		ComboBoxEntries options;

		/**
		 * Maximal option width. Used for dropdown menu width.
		 */
		int max_option_width;

	private:
		ComboBox(const ComboBox&);
		ComboBox& operator=(const ComboBox&);

		/** it handles the automatic registration of defined keys */
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

#endif /* __COMBOBOX_H__ */
