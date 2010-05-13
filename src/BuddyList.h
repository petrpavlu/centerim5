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

#ifndef __BUDDYLIST_H__
#define __BUDDYLIST_H__

#include "BuddyListNode.h"

#include <cppconsui/ComboBox.h>
#include <cppconsui/InputDialog.h>
#include <cppconsui/TreeView.h>
#include <cppconsui/Window.h>

#include <libpurple/blist.h>

#define BUDDYLIST (BuddyList::Instance())

class BuddyList
: public Window
{
	public:
		static BuddyList *Instance();

		// Window
		void Close();
		virtual void MoveResize(int newx, int newy, int neww, int newh);
		virtual void ScreenResized();
	
	protected:

	private:
		class AccountsBox
		: public ComboBox
		{
			public:
				AccountsBox(PurpleAccount *account);
				virtual ~AccountsBox() {}

				PurpleAccount *GetSelected() { return selected; }

			protected:
				PurpleAccount *selected;

				void UpdateText();

			private:
				AccountsBox(const AccountsBox&);
				AccountsBox& operator=(const AccountsBox&);

				void OnAccountChanged(const ComboBox::ComboBoxEntry& new_entry);
		};

		class NameButton
		: public Button
		{
			public:
				NameButton(bool alias, const gchar *val);
				virtual ~NameButton();

				const gchar *GetValue() { return value; }

			protected:
				const gchar *text;
				gchar *value;
				InputDialog *dialog;

				void UpdateText();

			private:
				NameButton(const NameButton&);
				NameButton& operator=(const NameButton&);

				virtual void OnActivate();

				void ResponseHandler(Dialog::ResponseType response);
		};

		class GroupBox
		: public ComboBox
		{
			public:
				GroupBox(const gchar *group);
				virtual ~GroupBox();

				const gchar *GetSelected() { return selected; }

			protected:
				gchar *selected;

				void UpdateText();

			private:
				GroupBox(const GroupBox&);
				GroupBox& operator=(const GroupBox&);

				void OnAccountChanged(const ComboBox::ComboBoxEntry& new_entry);
		};

		class AddBuddyWindow
		: public Window
		{
			public:
				AddBuddyWindow(PurpleAccount *account, const char *username,
						const char *group, const char *alias);
				virtual ~AddBuddyWindow() {}

			protected:
				AccountsBox *accounts_box;
				NameButton *name_button;
				NameButton *alias_button;
				GroupBox *group_box;
				HorizontalLine *line;
				HorizontalListBox *menu;

			private:
				AddBuddyWindow(const AddBuddyWindow&);
				AddBuddyWindow& operator=(const AddBuddyWindow&);

				void Add();
		};

		PurpleBlistUiOps centerim_blist_ui_ops;

		PurpleBuddyList *buddylist;

		TreeView *treeview;

		BuddyList();
		BuddyList(const BuddyList&);
		BuddyList& operator=(const BuddyList&);
		~BuddyList() {}

		static gboolean timeout_once_load(gpointer data);

		static void new_list_(PurpleBuddyList *list)
			{ BUDDYLIST->new_list(list); }
		static void new_node_(PurpleBlistNode *node) 
			{ BUDDYLIST->new_node(node); }
		static void update_(PurpleBuddyList *list, PurpleBlistNode *node)  
			{ BUDDYLIST->update(list, node); }
		static void remove_(PurpleBuddyList *list, PurpleBlistNode *node)
			{ BUDDYLIST->remove(list, node); }
		static void destroy_(PurpleBuddyList *list)
			{ BUDDYLIST->destroy(list); }
		static void request_add_buddy_(PurpleAccount *account,
				const char *username, const char *group, const char *alias)
			{ BUDDYLIST->request_add_buddy(account, username, group, alias); }
		static void request_add_chat_(PurpleAccount *account,
				PurpleGroup *group, const char *alias, const char *name)
			{ /* TODO */ ; }
		static void request_add_group_()
			{ /* TODO */ ; }

		void new_list(PurpleBuddyList *list);
		void new_node(PurpleBlistNode *node);
		void update(PurpleBuddyList *list, PurpleBlistNode *node);
		void remove(PurpleBuddyList *list, PurpleBlistNode *node);
		void destroy(PurpleBuddyList *list);
		void request_add_buddy(PurpleAccount *account, const char *username,
				const char *group, const char *alias);
};

#endif /* __BUDDYLIST_H__ */
