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

#ifndef _BUDDYLISTNODE_H__
#define _BUDDYLISTNODE_H__

#include <cppconsui/Button.h>
#include <cppconsui/TreeView.h>

#include <libpurple/blist.h>

class BuddyListNode
: public Button
{
	public:
		BuddyListNode(PurpleBlistNode *node);
		virtual ~BuddyListNode();

		static BuddyListNode *CreateNode(PurpleBlistNode *node);

		virtual void Update() = 0;

		// activate action
		virtual void OnActivate() = 0;

		BuddyListNode *GetParentNode();

		// TODO encapsulate
		TreeView::NodeReference ref;

	protected:
		PurpleBlistNode *node;

		/*
		 * Called by BuddyListBuddy and BuddyListContact to get presence
		 * status char. Returned value should be used as a prefix of
		 * buddy/contact name.
		 */
		const gchar *GetBuddyStatus(PurpleBuddy *buddy);

	private:
};

class BuddyListBuddy
: public BuddyListNode
{
	public:
		BuddyListBuddy(PurpleBlistNode *node);
		virtual ~BuddyListBuddy() {}

		// BuddyListNode
		virtual void Update();
		virtual void OnActivate();

	protected:
		PurpleBuddy *buddy;

	private:
		BuddyListBuddy(const BuddyListBuddy&);
		BuddyListBuddy& operator=(const BuddyListBuddy&);
};

class BuddyListChat
: public BuddyListNode
{
	public:
		BuddyListChat(PurpleBlistNode *node);
		virtual ~BuddyListChat() {}

		// BuddyListNode
		virtual void Update();
		virtual void OnActivate();

	protected:
		PurpleChat *chat;

	private:
		BuddyListChat(const BuddyListChat&);
		BuddyListChat& operator=(const BuddyListChat&);
};

class BuddyListContact
: public BuddyListNode
{
	public:
		BuddyListContact(PurpleBlistNode *node);
		virtual ~BuddyListContact() {}

		// BuddyListNode
		virtual void Update();
		virtual void OnActivate();

	protected:
		PurpleContact *contact;

	private:
		BuddyListContact(const BuddyListContact&);
		BuddyListContact& operator=(const BuddyListContact&);
};

class BuddyListGroup
: public BuddyListNode
{
	public:
		BuddyListGroup(PurpleBlistNode *node);
		virtual ~BuddyListGroup() {}

		// BuddyListNode
		virtual void Update();
		virtual void OnActivate();

	protected:
		PurpleGroup *group;

	private:
		BuddyListGroup(const BuddyListGroup&);
		BuddyListGroup& operator=(const BuddyListGroup&);
};

#endif /* _BUDDYLISTNODE_H__ */
