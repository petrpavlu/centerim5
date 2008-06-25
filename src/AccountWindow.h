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
		class AccountOptionSplit;
		typedef std::list<AccountOptionSplit*> SplitWidgets;
		typedef std::map<PurpleAccount*, SplitWidgets> SplitAccounts;

		class AccountOption
		: public Button
		{
			public:
				AccountOption(Widget& parent, PurpleAccount *account,
					PurpleAccountOption *option);
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
				AccountOptionBool(Widget& parent, PurpleAccount *account,
					PurpleAccountOption *option);
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
				AccountOptionString(Widget& parent, PurpleAccount *account,
					PurpleAccountOption *option);
				~AccountOptionString();

			protected:

			private:
				AccountOptionString(void);
				AccountOptionString(const AccountOptionString&);

				AccountOptionString& operator=(const AccountOptionString&);

				void UpdateText(void);
				void OnActivate(void);

				void ResponseHandler(Dialog::ResponseType response);

				const gchar *value;
				InputDialog *dialog;

				sigc::connection sig_response;
		};

		class AccountOptionInt
		: public AccountOption
		{
			public:
				AccountOptionInt(Widget& parent, PurpleAccount *account,
					PurpleAccountOption *option);
				~AccountOptionInt();

			protected:

			private:
				AccountOptionInt(void);
				AccountOptionInt(const AccountOptionInt&);

				AccountOptionInt& operator=(const AccountOptionInt&);

				void UpdateText(void);
				void OnActivate(void);

				void ResponseHandler(Dialog::ResponseType response);

				gchar *value;
				InputDialog *dialog;

				sigc::connection sig_response;
		};

		class AccountOptionSplit
		: public Button
		{
			public:
				AccountOptionSplit(Widget& parent, PurpleAccount *account,
					PurpleAccountUserSplit *split, SplitAccounts *split_accounts);
				~AccountOptionSplit();

				void SetValue(const gchar *value);
				const gchar* GetValue(void) { return value; }
				void UpdateText(void);

			protected:
				PurpleAccount *account;
				PurpleAccountUserSplit *split;

				const char *text;
				const gchar *value;
				InputDialog *dialog;

			private:
				AccountOptionSplit(void);
				AccountOptionSplit(const AccountOptionSplit&);

				AccountOptionSplit& operator=(const AccountOptionSplit&);

				void OnActivate(void);
				void UpdateSplits(void);

				void ResponseHandler(Dialog::ResponseType response);

				sigc::connection sig_response;

				SplitAccounts *split_accounts;
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

		SplitAccounts split_accounts;

		unsigned int menu_index;
		unsigned int accounts_index;
};

#endif /* __ACCOUNTSWINDOW_H__ */
