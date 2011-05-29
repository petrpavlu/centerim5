/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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
#include <cppconsui/MessageDialog.h>
#include <cppconsui/TreeView.h>
#include <libpurple/purple.h>

class BuddyListNode
: public CppConsUI::Button
{
public:
  static BuddyListNode *CreateNode(PurpleBlistNode *node);

  // Widget
  virtual void SetParent(CppConsUI::Container& parent);

  virtual bool LessThan(const BuddyListNode& other) const = 0;
  virtual void Update();
  virtual void OnActivate(CppConsUI::Button& activator) = 0;
  // debugging method
  virtual const char *ToString() const = 0;

  virtual void SetRefNode(CppConsUI::TreeView::NodeReference n) { ref = n; }
  virtual CppConsUI::TreeView::NodeReference GetRefNode() const
    { return ref; }

  /* Sorts in this node. It supposes that all siblings are in the correct
   * order. */
  void SortIn();

  BuddyListNode *GetParentNode() const;

  sigc::signal<void> signal_remove;

protected:
  CppConsUI::TreeView *treeview;
  CppConsUI::TreeView::NodeReference ref;

  PurpleBlistNode *node;

  BuddyListNode(PurpleBlistNode *node);
  virtual ~BuddyListNode();

  /* Called by BuddyListBuddy and BuddyListContact to get presence status
   * char. Returned value should be used as a prefix of buddy/contact name.
   * */
  const char *GetBuddyStatus(PurpleBuddy *buddy) const;

  /* Returns weight of buddy status (available > away > offline...). Used
   * for sorting. */
  int GetBuddyStatusWeight(PurpleBuddy *buddy) const;

private:
  BuddyListNode(BuddyListNode&);
  BuddyListNode& operator=(BuddyListNode&);

  void ActionRemove();
  void DeclareBindables();
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
  virtual const char *ToString() const;

protected:
  PurpleBuddy *buddy;

  void RemoveBuddyResponseHandler(CppConsUI::MessageDialog& activator,
      CppConsUI::AbstractDialog::ResponseType response);
  void OnBuddyRemove();

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
  virtual const char *ToString() const;

protected:
  PurpleChat *chat;

  void RemoveChatResponseHandler(CppConsUI::MessageDialog& activator,
      CppConsUI::AbstractDialog::ResponseType response);
  void OnChatRemove();

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
  virtual const char *ToString() const;

  virtual void SetRefNode(CppConsUI::TreeView::NodeReference n);

protected:
  PurpleContact *contact;

  void RemoveContactResponseHandler(CppConsUI::MessageDialog& activator,
      CppConsUI::AbstractDialog::ResponseType response);
  void OnContactRemove();

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
  virtual const char *ToString() const;

protected:
  PurpleGroup *group;

  void DelayedInit();

  void RemoveGroupResponseHandler(CppConsUI::MessageDialog& activator,
      CppConsUI::AbstractDialog::ResponseType response);
  void OnGroupRemove();

private:
  BuddyListGroup(PurpleBlistNode *node);
  BuddyListGroup(const BuddyListGroup&);
  BuddyListGroup& operator=(const BuddyListGroup&);
  virtual ~BuddyListGroup() {}
};

#endif // _BUDDYLISTNODE_H__
