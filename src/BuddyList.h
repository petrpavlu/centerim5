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

#ifndef __BUDDYLIST_H__
#define __BUDDYLIST_H__

#include "BuddyListNode.h"
#include "Log.h"
#include "Conf.h"

#include <cppconsui/Window.h>
#include <cppconsui/TreeView.h>

#include <cppconsui/Label.h>

#include <libpurple/blist.h>

class BuddyList
: public Window
{
	public:
		static BuddyList* Instance(void);
		static void Delete(void);

		void Resize(int neww, int newh);

		static void new_list_(PurpleBuddyList *list)
			{ BuddyList::Instance()->new_list(list); }
		static void new_node_(PurpleBlistNode *node) 
			{ BuddyList::Instance()->new_node(node); }
		static void update_node_(PurpleBuddyList *list, PurpleBlistNode *node)  
			{ BuddyList::Instance()->update_node(list, node); }
		static void remove_node_(PurpleBuddyList *list, PurpleBlistNode *node)
			{ BuddyList::Instance()->remove_node(list, node); }
		static void destroy_list_(PurpleBuddyList *list)
			{ BuddyList::Instance()->destroy_list(list); }
		static void request_add_buddy(PurpleAccount *account, const char *username, const char *group, const char *alias)
			{ /*TODO hand of to Request object*/ ; }
		static void request_add_chat(PurpleAccount *account, PurpleGroup *group, const char *alias, const char *name)
			{ /*TODO hand of to Request object*/ ; }
		static void request_add_group(void)
			{ /*TODO hand of to Request object*/ ; }

		 void new_list(PurpleBuddyList *list);
		void new_node(PurpleBlistNode *node);
		void update_node(PurpleBuddyList *list, PurpleBlistNode *node);
		void remove_node(PurpleBuddyList *list, PurpleBlistNode *node);
		void destroy_list(PurpleBuddyList *list);
	
	protected:

	private:

		BuddyList();
		~BuddyList();

		bool Load(void);

		BuddyListNode* CreateNode(PurpleBlistNode *node);
		void AddNode(BuddyListNode *node);
		void UpdateNode(BuddyListNode *node);
		void RemoveNode(BuddyListNode *node);

		static BuddyList* instance;

		PurpleBuddyList *buddylist;

		Log *log;
		Conf *conf;

		TreeView *treeview;
};

#endif /* __BUDDYLIST_H__ */
