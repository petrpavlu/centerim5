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

void BuddyListNode::Draw(void)
{
	Button::Draw();
}

BuddyListNode* BuddyListNode::CreateNode(PurpleBlistNode *node)
{
	BuddyListNode *bnode = NULL;

	if (PURPLE_BLIST_NODE_IS_BUDDY(node)) {
		bnode = new BuddyListBuddy(node);
	} else if (PURPLE_BLIST_NODE_IS_CHAT(node)) {
		bnode = new BuddyListChat(node);
	} else if (PURPLE_BLIST_NODE_IS_CONTACT(node)) {
		bnode = new BuddyListContact(node);
	} else if (PURPLE_BLIST_NODE_IS_GROUP(node)) {
		bnode = new BuddyListGroup(node);
	} //TODO log some error if no match here

	return bnode;
}

void BuddyListNode::GiveFocus(void)
{
	has_focus = true;
	Update();
	Draw();
}

void BuddyListNode::TakeFocus(void)
{
	has_focus = false;
	Update();
	Draw();
}

BuddyListNode* BuddyListNode::GetParentNode(void)
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

BuddyListBuddy::BuddyListBuddy(PurpleBlistNode *node)
: BuddyListNode(node)
{
	SetColorScheme("buddylistbuddy");

	buddy = (PurpleBuddy*)node;
	node->ui_data = this;
	Update();
}

void BuddyListBuddy::Draw(void)
{
	BuddyListNode::Draw();
}

void BuddyListBuddy::Update(void)
{
	//TODO this doesn't seem optimal
	//add a Width function to Label class and
	//clean this file up?
	const gchar *text;
	text = purple_buddy_get_alias(buddy);
	MoveResize(xpos, ypos, ::width(text), 1);
	SetText(text);
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

void BuddyListChat::Draw(void)
{
	BuddyListNode::Draw();
}

void BuddyListChat::Update(void)
{
	const gchar *text;
	text = purple_chat_get_name(chat);
	MoveResize(xpos, ypos, ::width(text), 1);
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

void BuddyListContact::Draw(void)
{
	BuddyListNode::Draw();
}

void BuddyListContact::Update(void)
{
	PurpleBlistNode *bnode;
	const gchar *text;

	if (contact->alias) {
		text = contact->alias;
	} else {
		for (bnode = ((PurpleBlistNode*)contact)->child; bnode != NULL; bnode = bnode->next) {
			if (PURPLE_BLIST_NODE_IS_BUDDY(bnode))
				break;
		}

		if (!bnode) {
			//TODO error message or is this too common?
			text = _("New Contact");
		} else {
			text = purple_buddy_get_alias((PurpleBuddy*)bnode);
		}
	}

	MoveResize(xpos, ypos, ::width(text), 1);
	SetText(text);
}

void BuddyListContact::OnActivate(void)
{
	LOG->Write(Log::Level_debug, "Contact activated!\n");
}

BuddyListGroup::BuddyListGroup(PurpleBlistNode *node)
: BuddyListNode(node)
{
	SetColorScheme("buddylistgroup");

	group = (PurpleGroup*)node;
	node->ui_data = this;
	Update();
}

void BuddyListGroup::Draw(void)
{
	BuddyListNode::Draw();
}

void BuddyListGroup::Update(void)
{
	const gchar *text;
	text = purple_group_get_name(group);
	MoveResize(xpos, ypos, ::width(text), 1);
	SetText(text);
}

void BuddyListGroup::OnActivate(void)
{
	LOG->Write(Log::Level_debug, "Group activated!\n");
	TreeView *t = dynamic_cast<TreeView *>(parent);
	if (t) // should be always true
		t->ActionToggleCollapsed();
}
