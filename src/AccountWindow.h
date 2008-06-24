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

#ifndef __ACCOUNTSWINDOW_H__
#define __ACCOUNTSWINDOW_H__

#include "Log.h"
#include "Conf.h"

#include <cppconsui/Window.h>
#include <cppconsui/TreeView.h>
#include <cppconsui/HorizontalListBox.h>
#include <cppconsui/HorizontalLine.h>
#include <cppconsui/Panel.h>
#include <cppconsui/Button.h>
#include <cppconsui/InputDialog.h>

#include <libpurple/account.h>
#include <libpurple/accountopt.h>

class AccountWindow
: public Window
{
	public:
		AccountWindow();

	protected:

	private:
		class AccountOption
		: public Button
		{
			public:
				AccountOption(Widget& parent, int x, int y,
					PurpleAccount *account, PurpleAccountOption *option);
				~AccountOption();

			protected:
				PurpleAccount *account;
				PurpleAccountOption *option;

				const char *setting;
				const char *text;

			private:
				AccountOption(void);
				AccountOption(const AccountOption&);

				AccountOption& operator=(const AccountOption&);

				virtual void UpdateText(void) = 0;
				virtual void OnActivate(void) = 0;
		};

		class AccountOptionBool
		: public AccountOption
		{
			public:
				AccountOptionBool(Widget& parent, int x, int y,
					PurpleAccount *account, PurpleAccountOption *option);
				~AccountOptionBool();

			protected:

			private:
				AccountOptionBool(void);
				AccountOptionBool(const AccountOptionBool&);

				AccountOptionBool& operator=(const AccountOptionBool&);

				virtual void UpdateText(void);
				virtual void OnActivate(void);

				gboolean value;
		};

		class AccountOptionString
		: public AccountOption
		{
			public:
				AccountOptionString(Widget& parent, int x, int y,
					PurpleAccount *account, PurpleAccountOption *option);
				~AccountOptionString();

			protected:

			private:
				AccountOptionString(void);
				AccountOptionString(const AccountOptionString&);

				AccountOptionString& operator=(const AccountOptionString&);

				void UpdateText(void);
				void OnActivate(void);

				void ResponseHandler(Dialog::ResponseType response);

				const char *value;
				InputDialog *dialog;

				sigc::connection sig_response;
		};

		~AccountWindow();

		void Populate(void);

		void Add(void);
		void Change(void);
		void Delete(void);

		//void FocusCycleLeftRight(Container::FocusDirection direction);
		//void FocusCycleUpDown(Container::FocusDirection direction);
		void MoveFocus(FocusDirection direction);

		Log *log;
		Conf *conf;

		TreeView *accounts;
		HorizontalListBox *menu;
		HorizontalLine *line;
		Panel *border;

		unsigned int menu_index;
		unsigned int accounts_index;
};

#endif /* __ACCOUNTSWINDOW_H__ */
