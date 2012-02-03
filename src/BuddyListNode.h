/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2012 by CenterIM developers
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
 */

#ifndef _BUDDYLISTNODE_H__
#define _BUDDYLISTNODE_H__

#include <cppconsui/Button.h>
#include <cppconsui/MenuWindow.h>
#include <cppconsui/MessageDialog.h>
#include <cppconsui/InputDialog.h>
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

  PurpleBlistNode *GetPurpleBlistNode() const { return node; }

  /* Sorts in this node. It supposes that all siblings are in the correct
   * order. */
  void SortIn();

  BuddyListNode *GetParentNode() const;

protected:
  CppConsUI::TreeView *treeview;
  CppConsUI::TreeView::NodeReference ref;

  PurpleBlistNode *node;

  BuddyListNode(PurpleBlistNode *node);
  virtual ~BuddyListNode();

  virtual void OpenContextMenu() = 0;

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

  void ActionOpenContextMenu();
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

  PurpleBuddy *GetPurpleBuddy() const { return buddy; }

protected:
  class ContextMenu
  : public CppConsUI::MenuWindow
  {
  public:
    ContextMenu(BuddyListBuddy& parent);
    virtual ~ContextMenu() {}

  protected:
    BuddyListBuddy *parent;

    void ChangeAliasResponseHandler(CppConsUI::InputDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response);
    void OnChangeAlias(Button& activator);

    void RemoveResponseHandler(CppConsUI::MessageDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response);
    void OnRemove(Button& activator);

  private:
    ContextMenu(const ContextMenu&);
    ContextMenu& operator=(const ContextMenu&);
  };

  PurpleBuddy *buddy;

  // BuddyListNode
  virtual void OpenContextMenu();

  void UpdateColorScheme();

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

  PurpleChat *GetPurpleChat() const { return chat; }

protected:
  class ContextMenu
  : public CppConsUI::MenuWindow
  {
  public:
    ContextMenu(BuddyListChat& parent);
    virtual ~ContextMenu() {}

  protected:
    BuddyListChat *parent;

    void ChangeAliasResponseHandler(CppConsUI::InputDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response);
    void OnChangeAlias(Button& activator);

    void RemoveResponseHandler(CppConsUI::MessageDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response);
    void OnRemove(Button& activator);

  private:
    ContextMenu(const ContextMenu&);
    ContextMenu& operator=(const ContextMenu&);
  };

  PurpleChat *chat;

  // BuddyListNode
  virtual void OpenContextMenu();

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

  PurpleContact *GetPurpleContact() const { return contact; }

protected:
  class ContextMenu
  : public CppConsUI::MenuWindow
  {
  public:
    ContextMenu(BuddyListContact& parent);
    virtual ~ContextMenu() {}

  protected:
    BuddyListContact *parent;

    void ChangeAliasResponseHandler(CppConsUI::InputDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response);
    void OnChangeAlias(Button& activator);

    void RemoveResponseHandler(CppConsUI::MessageDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response);
    void OnRemove(Button& activator);
    void OnMoveTo(Button& activator, PurpleGroup *group);

  private:
    ContextMenu(const ContextMenu&);
    ContextMenu& operator=(const ContextMenu&);
  };

  PurpleContact *contact;

  // BuddyListNode
  virtual void OpenContextMenu();

  void UpdateColorScheme();

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

  PurpleGroup *GetPurpleGroup() const { return group; }

protected:
  class ContextMenu
  : public CppConsUI::MenuWindow
  {
  public:
    ContextMenu(BuddyListGroup& parent);
    virtual ~ContextMenu() {}

  protected:
    BuddyListGroup *parent;

    void RenameResponseHandler(CppConsUI::InputDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response);
    void OnRename(Button& activator);

    void RemoveResponseHandler(CppConsUI::MessageDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response);
    void OnRemove(Button& activator);

  private:
    ContextMenu(const ContextMenu&);
    ContextMenu& operator=(const ContextMenu&);
  };

  PurpleGroup *group;

  // BuddyListNode
  virtual void OpenContextMenu();

  void DelayedInit();

private:
  BuddyListGroup(PurpleBlistNode *node);
  BuddyListGroup(const BuddyListGroup&);
  BuddyListGroup& operator=(const BuddyListGroup&);
  virtual ~BuddyListGroup() {}
};

#endif // _BUDDYLISTNODE_H__

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
