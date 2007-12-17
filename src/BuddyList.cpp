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

//TODO: configurable path using ./configure
#define CIM_CONFIG_PATH		".centerim"

#define CONF_PLUGIN_SAVE_PREF	"/centerim/plugins/loaded"

#define EXCEPTION_NONE			0
#define EXCEPTION_PURPLE_CORE_INIT	100

#include "BuddyList.h"

#include "Log.h"
#include "Conf.h"

//TODO remove this include. non-cppconsui classes may not touch area member
#include <cppconsui/Curses.h>


#include <cppconsui/Keys.h>
#include <cppconsui/Window.h>
#include <cppconsui/TreeView.h>
//TODO remove when signals are used
#include <cppconsui/WindowManager.h>

#include <cppconsui/Label.h>

//TODO remove stuff we dont need here
#include <libpurple/blist.h>
#include <libpurple/prefs.h>
#include <libpurple/core.h>
#include <libpurple/plugin.h>
#include <libpurple/util.h>
#include <libpurple/pounce.h>
#include <libpurple/debug.h>
#include <libpurple/savedstatuses.h>

//#include <glib.h>
#include <glibmm/main.h>

BuddyList* BuddyList::instance = NULL;

BuddyList* BuddyList::Instance(void)
{
	if (!instance) instance = new BuddyList();
	return instance;
}

void BuddyList::Delete(void)
{
	if (instance) {
		delete instance;
		instance = NULL;
	}
}

//TODO move this struct inside the buddylist object
static PurpleBlistUiOps centerim_blist_uiops =
{
	BuddyList::new_list_,
	BuddyList::new_node_,
	NULL, /* show */
	BuddyList::update_node_,
	BuddyList::remove_node_,
	BuddyList::destroy_list_,
	NULL, /* set_visible */
	BuddyList::request_add_buddy,
	BuddyList::request_add_chat,
	BuddyList::request_add_group,
	NULL,
	NULL,
	NULL,
	NULL,
};

BuddyList::BuddyList()
: Window(0, 0, 80, 24, NULL)
{
	log = Log::Instance();
	conf = Conf::Instance();

	SetBorder(new Border());
	MoveResize(conf->GetBuddyListDimensions());

	//TODO check if this has been moved to purple_blist_init
	//renove these lines if it was
	//as this will probably move to purple_init, the buddylist 
	//object should be available a lot more early and the uiops should
	//be set a lot more early. (all in all a lot of work)
	buddylist = purple_blist_new();
	buddylist->ui_data = this;
	purple_set_blist(buddylist);

	/* setup the callbacks for the buddylist */
	purple_blist_set_ui_ops(&centerim_blist_uiops);

	Glib::signal_timeout().connect(sigc::mem_fun(this, &BuddyList::Load), 0);

	//TODO get linestyle from conf
	treeview = new TreeView(*this, 1, 1, w-2, h-2, LineStyle::LineStyleDefault());
	AddWidget(treeview);
}

bool BuddyList::Load(void)
{
	/* Loads the buddy list from ~/.cim/blist.xml. */
	purple_blist_load();

	return false;
}

BuddyList::~BuddyList()
{
	/* Schedule a save of the blist.xml file. 
	 * this is done automatically by libpurple
	 * when any change occurs, but lets do that anyway
	 */
	purple_blist_schedule_save(); //TODO: will this go wrong?! (probably)

	delete GetBorder(); //TODO what if NULL?
}

void BuddyList::AddNode(BuddyListNode *node)
{
	BuddyListNode *parent = node->GetParent();
	node->id = treeview->AddNode(parent ? parent->id : -1, node, NULL);

	Redraw();
}

void BuddyList::UpdateNode(BuddyListNode *node)
{
	BuddyListNode *parent = node->GetParent();
	/* The parent could have changed, so re-parent the node */
	if (parent)
		treeview->SetParent(node->id, parent->id);

	node->Update();

	Redraw();
}

void BuddyList::RemoveNode(BuddyListNode *node)
{
	//TODO check for subnodes (if this is a group for instance)
	treeview->DeleteNode(node->id, false);
	Redraw();
}

void BuddyList::new_list(PurpleBuddyList *list)
{
	if (buddylist != list) {
		//TODO if this happens, then the first todo in Load should
		//be checked again.
		log->Write(PURPLE_DEBUG_ERROR, "Different Buddylist detected!\n");
	}
	if (buddylist->ui_data != this)
	{
		//TODO actually, this amounts to the same as the error above this one :).
		log->Write(PURPLE_DEBUG_ERROR, "New Buddylist detected, but we only support one buddylist.\n");
	}
}

void BuddyList::new_node(PurpleBlistNode *node)
{
	BuddyListNode *bnode;

	if (!node->ui_data) {
		node->ui_data = bnode = BuddyListNode::CreateNode(*treeview, node);
		AddNode((BuddyListNode*)node->ui_data);

		Redraw();

		if (PURPLE_BLIST_NODE_IS_CONTACT(node)) {
			/* The core seems to expect the UI to add the buddies.
			 * (according to finch)
			 * */
			for (node = node->child; node; node = node->next)
				new_node(node);
		}
	}
}

void BuddyList::update_node(PurpleBuddyList *list, PurpleBlistNode *node)
{
	/* finch does this */
	g_return_if_fail(node != NULL);
	
	if (!node->ui_data) {
		//TODO remove when this never happens :) (yeah, try to catch that one! :)
		log->Write(PURPLE_DEBUG_MISC, "BuddyList::update called before BuddyList::new_node\n");
		new_node(node);
	}

	/* Update the node data */
	UpdateNode((BuddyListNode*)node->ui_data);

	/* if the node is a buddy, we also have to update the contact
	 * this buddy belongs to.
	 * */
	if (PURPLE_BLIST_NODE_IS_BUDDY(node)) {
		update_node(list, node->parent);
		//TODO is it possible for a nodes parent not to be a contact?
		//if so, then this call could use some checks
	}

	Redraw();
}

void BuddyList::remove_node(PurpleBuddyList *list, PurpleBlistNode *node)
{
	BuddyListNode *bnode;

	if (!node->ui_data) return; /* nothing to remove */

	bnode = (BuddyListNode*)node->ui_data;
	RemoveNode(bnode);
	delete bnode;
}

void BuddyList::destroy_list(PurpleBuddyList *list)
{
}
