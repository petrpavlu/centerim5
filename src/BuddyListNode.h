/*
 * Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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
#include <cppconsui/InputDialog.h>
#include <cppconsui/MenuWindow.h>
#include <cppconsui/MessageDialog.h>
#include <cppconsui/TreeView.h>
#include <libpurple/purple.h>

class BuddyListNode : public CppConsUI::Button {
public:
  static BuddyListNode *createNode(PurpleBlistNode *node);

  // Widget
  virtual void setParent(CppConsUI::Container &parent);

  virtual bool lessOrEqual(const BuddyListNode &other) const = 0;
  virtual void update();
  virtual void onActivate(CppConsUI::Button &activator) = 0;
  // debugging method
  virtual const char *toString() const = 0;

  virtual void setRefNode(CppConsUI::TreeView::NodeReference n);
  virtual CppConsUI::TreeView::NodeReference getRefNode() const { return ref; }

  PurpleBlistNode *getPurpleBlistNode() const { return blist_node; }

  /* Sorts in this node. */
  void sortIn();

  BuddyListNode *getParentNode() const;

protected:
  class ContextMenu : public CppConsUI::MenuWindow {
  public:
    ContextMenu(BuddyListNode &parent_node_);
    virtual ~ContextMenu() {}

  protected:
    BuddyListNode *parent_node;

    void onMenuAction(Button &activator, PurpleCallback callback, void *data);
    void appendMenuAction(MenuWindow &menu, PurpleMenuAction *act);
    void appendProtocolMenu(PurpleConnection *gc);
    void appendExtendedMenu();

  private:
    CONSUI_DISABLE_COPY(ContextMenu);
  };

  CppConsUI::TreeView *treeview;
  CppConsUI::TreeView::NodeReference ref;

  PurpleBlistNode *blist_node;

  // cached value of purple_blist_node_get_int(blist_node, "last_activity")
  int last_activity;

  BuddyListNode(PurpleBlistNode *node_);
  virtual ~BuddyListNode();

  virtual void openContextMenu() = 0;

  bool lessOrEqualByType(const BuddyListNode &other) const;
  bool lessOrEqualByBuddySort(PurpleBuddy *left, PurpleBuddy *right) const;

  /* Called by BuddyListBuddy and BuddyListContact to get presence status
   * char. Returned value should be used as a prefix of buddy/contact name. */
  const char *getBuddyStatus(PurpleBuddy *buddy) const;

  /* Returns weight of buddy status (available > away > offline...). Used
   * for sorting. */
  int getBuddyStatusWeight(PurpleBuddy *buddy) const;

  void updateFilterVisibility(const char *name);

  void retrieveUserInfoForName(PurpleConnection *gc, const char *name) const;

private:
  CONSUI_DISABLE_COPY(BuddyListNode);

  void actionOpenContextMenu();
  void declareBindables();
};

class BuddyListBuddy : public BuddyListNode {
  friend class BuddyListNode;

public:
  // BuddyListNode
  virtual bool lessOrEqual(const BuddyListNode &other) const;
  virtual void update();
  virtual void onActivate(Button &activator);
  virtual const char *toString() const;

  PurpleBuddy *getPurpleBuddy() const { return buddy; }
  void retrieveUserInfo();

protected:
  class BuddyContextMenu : public ContextMenu {
  public:
    BuddyContextMenu(BuddyListBuddy &parent_buddy_);
    virtual ~BuddyContextMenu() {}

  protected:
    BuddyListBuddy *parent_buddy;

    void onInformation(Button &activator);

    void changeAliasResponseHandler(CppConsUI::InputDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);
    void onChangeAlias(Button &activator);

    void removeResponseHandler(CppConsUI::MessageDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);
    void onRemove(Button &activator);

  private:
    CONSUI_DISABLE_COPY(BuddyContextMenu);
  };

  PurpleBuddy *buddy;

  // Widget
  virtual int getColorPair(const char *widget, const char *property) const;

  // BuddyListNode
  virtual void openContextMenu();

  void updateColorScheme();

private:
  BuddyListBuddy(PurpleBlistNode *node_);
  virtual ~BuddyListBuddy() {}
  CONSUI_DISABLE_COPY(BuddyListBuddy);
};

class BuddyListChat : public BuddyListNode {
  friend class BuddyListNode;

public:
  // BuddyListNode
  virtual bool lessOrEqual(const BuddyListNode &other) const;
  virtual void update();
  virtual void onActivate(Button &activator);
  virtual const char *toString() const;

  PurpleChat *getPurpleChat() const { return chat; }

protected:
  class ChatContextMenu : public ContextMenu {
  public:
    ChatContextMenu(BuddyListChat &parent_chat_);
    virtual ~ChatContextMenu() {}

  protected:
    BuddyListChat *parent_chat;

    void changeAliasResponseHandler(CppConsUI::InputDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);
    void onChangeAlias(Button &activator);

    void removeResponseHandler(CppConsUI::MessageDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);
    void onRemove(Button &activator);

  private:
    CONSUI_DISABLE_COPY(ChatContextMenu);
  };

  PurpleChat *chat;

  // BuddyListNode
  virtual void openContextMenu();

private:
  BuddyListChat(PurpleBlistNode *node_);
  virtual ~BuddyListChat() {}
  CONSUI_DISABLE_COPY(BuddyListChat);
};

class BuddyListContact : public BuddyListNode {
  friend class BuddyListNode;

public:
  // BuddyListNode
  virtual bool lessOrEqual(const BuddyListNode &other) const;
  virtual void update();
  virtual void onActivate(Button &activator);
  virtual const char *toString() const;
  virtual void setRefNode(CppConsUI::TreeView::NodeReference n);

  PurpleContact *getPurpleContact() const { return contact; }
  void retrieveUserInfo();

  bool isCollapsed() const { return ref->isCollapsed(); }
  void setCollapsed(bool collapsed) { treeview->setCollapsed(ref, collapsed); }

protected:
  class ContactContextMenu : public ContextMenu {
  public:
    ContactContextMenu(BuddyListContact &parent_contact_);
    virtual ~ContactContextMenu() {}

  protected:
    BuddyListContact *parent_contact;

    void onExpandRequest(Button &activator, bool expand);
    void onInformation(Button &activator);

    void changeAliasResponseHandler(CppConsUI::InputDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);
    void onChangeAlias(Button &activator);

    void removeResponseHandler(CppConsUI::MessageDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);
    void onRemove(Button &activator);

    void onMoveTo(Button &activator, PurpleGroup *group);

  private:
    CONSUI_DISABLE_COPY(ContactContextMenu);
  };

  PurpleContact *contact;

  // Widget
  virtual int getColorPair(const char *widget, const char *property) const;

  // BuddyListNode
  virtual void openContextMenu();

  void updateColorScheme();

private:
  BuddyListContact(PurpleBlistNode *node_);
  virtual ~BuddyListContact() {}
  CONSUI_DISABLE_COPY(BuddyListContact);
};

class BuddyListGroup : public BuddyListNode {
  friend class BuddyListNode;

public:
  // BuddyListNode
  virtual bool lessOrEqual(const BuddyListNode &other) const;
  virtual void update();
  virtual void onActivate(Button &activator);
  virtual const char *toString() const;
  virtual void setRefNode(CppConsUI::TreeView::NodeReference n);

  PurpleGroup *getPurpleGroup() const { return group; }

  void initCollapsedState();

protected:
  class GroupContextMenu : public ContextMenu {
  public:
    GroupContextMenu(BuddyListGroup &parent_group_);
    virtual ~GroupContextMenu() {}

  protected:
    BuddyListGroup *parent_group;

    void renameResponseHandler(CppConsUI::InputDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);
    void onRename(Button &activator);

    void removeResponseHandler(CppConsUI::MessageDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);
    void onRemove(Button &activator);

    void onMoveAfter(Button &activator, PurpleGroup *group);

  private:
    CONSUI_DISABLE_COPY(GroupContextMenu);
  };

  PurpleGroup *group;

  // BuddyListNode
  virtual void openContextMenu();

private:
  BuddyListGroup(PurpleBlistNode *node_);
  virtual ~BuddyListGroup() {}
  CONSUI_DISABLE_COPY(BuddyListGroup);
};

#endif // _BUDDYLISTNODE_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
