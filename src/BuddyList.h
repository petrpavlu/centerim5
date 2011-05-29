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

#ifndef __BUDDYLIST_H__
#define __BUDDYLIST_H__

#include "BuddyListNode.h"

#include <cppconsui/ComboBox.h>
#include <cppconsui/InputDialog.h>
#include <cppconsui/TreeView.h>
#include <cppconsui/Window.h>
#include <libpurple/purple.h>

#define BUDDYLIST (BuddyList::Instance())

class BuddyList
: public CppConsUI::Window
{
public:
  static BuddyList *Instance();

  // Widget
  virtual bool RestoreFocus();
  virtual void UngrabFocus();

  // FreeWindow
  virtual void Close();
  virtual void ScreenResized();

protected:

private:
  PurpleBlistUiOps centerim_blist_ui_ops;
  PurpleBuddyList *buddylist;
  CppConsUI::TreeView *treeview;

  static BuddyList *instance;

  BuddyList();
  BuddyList(const BuddyList&);
  BuddyList& operator=(const BuddyList&);
  virtual ~BuddyList();

  static void Init();
  static void Finalize();
  friend class CenterIM;

  void Load();

  bool CheckAnyAccountConnected();

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
    { BUDDYLIST->request_add_chat(account, group, alias, name); }
  static void request_add_group_()
    { /* TODO */ ; }

  void new_list(PurpleBuddyList *list);
  void new_node(PurpleBlistNode *node);
  void update(PurpleBuddyList *list, PurpleBlistNode *node);
  void remove(PurpleBuddyList *list, PurpleBlistNode *node);
  void destroy(PurpleBuddyList *list);
  void request_add_buddy(PurpleAccount *account, const char *username,
      const char *group, const char *alias);
  void request_add_chat(PurpleAccount *account, PurpleGroup *group,
      const char *alias, const char *name);

  static void add_buddy_ok_cb_(void *data, PurpleRequestFields *fields)
    { reinterpret_cast<BuddyList*>(data)->add_buddy_ok_cb(fields); }
  void add_buddy_ok_cb(PurpleRequestFields *fields);
  static void add_chat_ok_cb_(void *data, PurpleRequestFields *fields)
    { reinterpret_cast<BuddyList*>(data)->add_chat_ok_cb(fields); }
  void add_chat_ok_cb(PurpleRequestFields *fields);
};

#endif // __BUDDYLIST_H__
