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
#include <libpurple/purple.h>

class BuddyListNode
: public Button
{
public:
  static BuddyListNode *CreateNode(PurpleBlistNode *node);

  virtual bool LessThan(const BuddyListNode& other) const = 0;
  virtual void Update();
  virtual void OnActivate(Button& activator) = 0;
  // debugging method
  virtual const gchar *ToString() const = 0;
  /* Sorts in this node. It supposes that all siblings are in correct order.
   * */
  virtual void SortIn();

  BuddyListNode *GetParentNode() const;

  void SetRefNode(TreeView::NodeReference n) { ref = n; }
  TreeView::NodeReference GetRefNode() const { return ref; }

protected:
  TreeView::NodeReference ref;

  PurpleBlistNode *node;

  BuddyListNode(PurpleBlistNode *node);
  virtual ~BuddyListNode();

  /* Called by BuddyListBuddy and BuddyListContact to get presence status
   * char. Returned value should be used as a prefix of buddy/contact name.
   * */
  const gchar *GetBuddyStatus(PurpleBuddy *buddy) const;

  /* Returns weight of buddy status (available > away > offline...). Used
   * for sorting. */
  int GetBuddyStatusWeight(PurpleBuddy *buddy) const;

private:
  BuddyListNode(BuddyListNode&);
  BuddyListNode& operator=(BuddyListNode&);
};

class BuddyListBuddy
: public BuddyListNode
{
friend class BuddyListNode;
public:
  // BuddyListNode
  virtual bool LessThan(const BuddyListNode& other) const;
  virtual void Update();
  virtual void OnActivate(Button& activator);
  virtual const gchar *ToString() const;

protected:
  PurpleBuddy *buddy;

private:
  BuddyListBuddy(PurpleBlistNode *node);
  BuddyListBuddy(const BuddyListBuddy&);
  BuddyListBuddy& operator=(const BuddyListBuddy&);
  virtual ~BuddyListBuddy() {}
};

class BuddyListChat
: public BuddyListNode
{
friend class BuddyListNode;
public:
  // BuddyListNode
  virtual bool LessThan(const BuddyListNode& other) const;
  virtual void Update();
  virtual void OnActivate(Button& activator);
  virtual const gchar *ToString() const;

protected:
  PurpleChat *chat;

private:
  BuddyListChat(PurpleBlistNode *node);
  BuddyListChat(const BuddyListChat&);
  BuddyListChat& operator=(const BuddyListChat&);
  virtual ~BuddyListChat() {}
};

class BuddyListContact
: public BuddyListNode
{
friend class BuddyListNode;
public:
  // BuddyListNode
  virtual bool LessThan(const BuddyListNode& other) const;
  virtual void Update();
  virtual void OnActivate(Button& activator);
  virtual const gchar *ToString() const;

protected:
  PurpleContact *contact;

private:
  BuddyListContact(PurpleBlistNode *node);
  BuddyListContact(const BuddyListContact&);
  BuddyListContact& operator=(const BuddyListContact&);
  virtual ~BuddyListContact() {}
};

class BuddyListGroup
: public BuddyListNode
{
friend class BuddyListNode;
public:
  // BuddyListNode
  virtual bool LessThan(const BuddyListNode& other) const;
  virtual void Update();
  virtual void OnActivate(Button& activator);
  virtual const gchar *ToString() const;
  virtual void SortIn();

protected:
  PurpleGroup *group;

private:
  BuddyListGroup(PurpleBlistNode *node);
  BuddyListGroup(const BuddyListGroup&);
  BuddyListGroup& operator=(const BuddyListGroup&);
  virtual ~BuddyListGroup() {}
};

#endif // _BUDDYLISTNODE_H__
