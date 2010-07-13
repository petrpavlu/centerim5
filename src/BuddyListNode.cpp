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

#include "BuddyListNode.h"

#include "Conversations.h"

#include <cppconsui/Keys.h>

#include "gettext.h"

BuddyListNode::BuddyListNode(PurpleBlistNode *node)
: Button("")
, node(node)
{
	can_focus = true;

	signal_activate.connect(sigc::mem_fun(this, &BuddyListNode::OnActivate));
}

BuddyListNode::~BuddyListNode()
{
	node->ui_data = NULL;
}

BuddyListNode *BuddyListNode::CreateNode(PurpleBlistNode *node)
{
	if (PURPLE_BLIST_NODE_IS_BUDDY(node))
		return new BuddyListBuddy(node);
	if (PURPLE_BLIST_NODE_IS_CHAT(node))
		return new BuddyListChat(node);
	else if (PURPLE_BLIST_NODE_IS_CONTACT(node))
		return new BuddyListContact(node);
	else if (PURPLE_BLIST_NODE_IS_GROUP(node))
		return new BuddyListGroup(node);

	// TODO log some error if no match here
	return NULL;
}

BuddyListNode *BuddyListNode::GetParentNode()
{
	//TODO here I made the assumption that parents are *always* added before children
	//(groups before its contacts/buddies, contacts before its buddies)
	PurpleBlistNode *parent;

	parent = node->parent;

	if (!parent) return NULL; //TODO emit warning?

	if (!parent->ui_data) {
		LOG->Write(Log::Level_error, "child added before its parent\n");
		//TODO how to solve this?
		return NULL;
	}

	return (BuddyListNode*)parent->ui_data;
}

const gchar *BuddyListNode::GetBuddyStatus(PurpleBuddy *buddy)
{
	if (!purple_account_is_connected(purple_buddy_get_account(buddy)))
		return "";

	PurplePresence *presence = purple_buddy_get_presence(buddy);
	PurpleStatus *status = purple_presence_get_active_status(presence);
	PurpleStatusType *status_type = purple_status_get_type(status);
	PurpleStatusPrimitive prim = purple_status_type_get_primitive(status_type);

	switch (prim) {
		case PURPLE_STATUS_UNSET:
			return "[x] ";
		case PURPLE_STATUS_OFFLINE:
			return "";
		case PURPLE_STATUS_AVAILABLE:
			return "[o] ";
		case PURPLE_STATUS_UNAVAILABLE:
			return "[u] ";
		case PURPLE_STATUS_INVISIBLE:
			return "[i] ";
		case PURPLE_STATUS_AWAY:
			return "[a] ";
		case PURPLE_STATUS_EXTENDED_AWAY:
			return "[A] ";
		case PURPLE_STATUS_MOBILE:
			return "[m] ";
		case PURPLE_STATUS_TUNE:
			return "[t] ";
		default:
			return "[X] ";
	}
}

BuddyListBuddy::BuddyListBuddy(PurpleBlistNode *node)
: BuddyListNode(node)
{
	SetColorScheme("buddylistbuddy");

	buddy = (PurpleBuddy*)node;
	node->ui_data = this;
	Update();
}

void BuddyListBuddy::Update(void)
{
	gchar *text = g_strdup_printf("%s%s", GetBuddyStatus(buddy), purple_buddy_get_alias(buddy));
	SetText(text);
	g_free(text);
}

void BuddyListBuddy::OnActivate(void)
{
	LOG->Write(Log::Level_debug, "Buddy activated!\n"); //TODO remove sometime

	CONVERSATIONS->ShowConversation(PURPLE_CONV_TYPE_IM, purple_buddy_get_account(buddy), purple_buddy_get_name(buddy));
}

BuddyListChat::BuddyListChat(PurpleBlistNode *node)
: BuddyListNode(node)
{
	SetColorScheme("buddylistchat");

	chat = (PurpleChat*)node;
	node->ui_data = this;
	Update();
}

void BuddyListChat::Update(void)
{
	const gchar *text;
	text = purple_chat_get_name(chat);
	SetText(text);
}

void BuddyListChat::OnActivate(void)
{
	LOG->Write(Log::Level_debug, "Chat activated!\n");

	CONVERSATIONS->ShowConversation(PURPLE_CONV_TYPE_CHAT, purple_chat_get_account(chat), purple_chat_get_name(chat));
}

BuddyListContact::BuddyListContact(PurpleBlistNode *node)
: BuddyListNode(node)
{
	SetColorScheme("buddylistcontact");

	contact = (PurpleContact*)node;
	node->ui_data = this;
	Update();
}

void BuddyListContact::Update(void)
{
	const gchar *name;
	PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact);

	if (contact->alias)
		name = contact->alias;
	else if (buddy)
		name = purple_buddy_get_alias(buddy);
	else
		// TODO error message or is this too common?
		name = _("New Contact");

	gchar *text = g_strdup_printf("%s%s", GetBuddyStatus(buddy), name);
	SetText(text);
	g_free(text);
}

void BuddyListContact::OnActivate(void)
{
	LOG->Write(Log::Level_debug, "Contact activated!\n");

	PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact);
	CONVERSATIONS->ShowConversation(PURPLE_CONV_TYPE_IM, purple_buddy_get_account(buddy), purple_buddy_get_name(buddy));
}

BuddyListGroup::BuddyListGroup(PurpleBlistNode *node)
: BuddyListNode(node)
{
	SetColorScheme("buddylistgroup");

	group = (PurpleGroup*)node;
	node->ui_data = this;
	Update();
}

void BuddyListGroup::Update(void)
{
	const gchar *text;
	text = purple_group_get_name(group);
	SetText(text);
}

void BuddyListGroup::OnActivate(void)
{
	LOG->Write(Log::Level_debug, "Group activated!\n");
	TreeView *t = dynamic_cast<TreeView *>(parent);
	if (t) // should be always true
		t->ToggleCollapsed(ref);
}
