/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2013 by CenterIM developers
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

#ifndef __BUDDYLIST_H__
#define __BUDDYLIST_H__

#include "BuddyListNode.h"

#include <cppconsui/Button.h>
#include <cppconsui/CheckBox.h>
#include <cppconsui/ComboBox.h>
#include <cppconsui/TreeView.h>
#include <cppconsui/SplitDialog.h>
#include <cppconsui/Window.h>

#define BUDDYLIST (BuddyList::instance())

class BuddyList
: public CppConsUI::Window
{
public:
  enum ListMode {
    LIST_NORMAL,
    LIST_FLAT
  };

  enum GroupSortMode {
    GROUP_SORT_BY_USER,
    GROUP_SORT_BY_NAME
  };

  enum BuddySortMode {
    BUDDY_SORT_BY_NAME,
    BUDDY_SORT_BY_STATUS,
    BUDDY_SORT_BY_ACTIVITY
  };

  enum ColorizationMode {
    COLOR_NONE,
    COLOR_BY_STATUS,
    COLOR_BY_ACCOUNT
  };

  static BuddyList *instance();

  // InputProcessor
  virtual bool processInputText(const TermKeyKey &key);

  // Widget
  virtual bool restoreFocus();
  virtual void ungrabFocus();

  // FreeWindow
  virtual void close();
  virtual void onScreenResized();

  // these functions are faster version of getting blist/* prefs
  bool getShowEmptyGroupsPref() const { return show_empty_groups; }
  bool getShowOfflineBuddiesPref() const { return show_offline_buddies; }
  ListMode getListMode() const { return list_mode; }
  GroupSortMode getGroupSortMode() const { return group_sort_mode; }
  BuddySortMode getBuddySortMode() const { return buddy_sort_mode; }
  ColorizationMode getColorizationMode() const { return colorization_mode; }

  const char *getFilterString() const { return filter_buffer; }

  void updateNode(PurpleBlistNode *node);

protected:

private:
  enum UpdateFlags {
    UPDATE_GROUPS = 1 << 0,
    UPDATE_OTHERS = 1 << 1
  };

  class Filter
  : public CppConsUI::Widget
  {
  public:
    Filter(BuddyList *parent_);
    virtual ~Filter() {}

    // Widget
    virtual void draw(CppConsUI::Curses::ViewPort area);

  protected:
    BuddyList *parent;

  private:
    CONSUI_DISABLE_COPY(Filter);
  };

  class AddWindow
  : public CppConsUI::SplitDialog
  {
  public:
    AddWindow(const char *title);
    virtual ~AddWindow() {}

    // FreeWindow
    virtual void onScreenResized();

  protected:
    class AccountOption
    : public CppConsUI::ComboBox
    {
    public:
      explicit AccountOption(PurpleAccount *default_account = NULL,
          bool chat_only = false);
      virtual ~AccountOption() {}

    protected:

    private:
      CONSUI_DISABLE_COPY(AccountOption);
    };

    class GroupOption
    : public CppConsUI::ComboBox
    {
    public:
      explicit GroupOption(const char *default_group);
      explicit GroupOption(PurpleGroup *default_group);
      virtual ~GroupOption() {}

    protected:

    private:
      CONSUI_DISABLE_COPY(GroupOption);
    };

    class StringOption
    : public CppConsUI::Button
    {
    public:
      explicit StringOption(const char *text, const char *value = NULL,
          bool masked = false);
      virtual ~StringOption() {}

    protected:
      void onActivate(CppConsUI::Button& activator);
      void responseHandler(CppConsUI::InputDialog& activator,
          CppConsUI::AbstractDialog::ResponseType response);

    private:
      CONSUI_DISABLE_COPY(StringOption);
    };

    class IntegerOption
    : public CppConsUI::Button
    {
    public:
      explicit IntegerOption(const char *text, const char *value = NULL,
          bool masked = false, int min = INT_MIN, int max = INT_MAX);
      virtual ~IntegerOption() {}

    protected:
      int min_value;
      int max_value;

      void onActivate(CppConsUI::Button& activator);
      void responseHandler(CppConsUI::InputDialog& activator,
          CppConsUI::AbstractDialog::ResponseType response);

    private:
      CONSUI_DISABLE_COPY(IntegerOption);
    };

    class BooleanOption
    : public CppConsUI::CheckBox
    {
    public:
      explicit BooleanOption(const char *text, bool checked = true);
      virtual ~BooleanOption() {}

    protected:

    private:
      CONSUI_DISABLE_COPY(BooleanOption);
    };

    CppConsUI::TreeView *treeview;

    virtual void onAddRequest(CppConsUI::Button& activator) = 0;

  private:
    CONSUI_DISABLE_COPY(AddWindow);
  };

  class AddBuddyWindow
  : public AddWindow
  {
  public:
    AddBuddyWindow(PurpleAccount *account, const char *username,
        const char *group, const char *alias);
    virtual ~AddBuddyWindow() {}

  protected:
    AccountOption *account_option;
    StringOption *name_option;
    StringOption *alias_option;
    GroupOption *group_option;

    // AddWindow
    virtual void onAddRequest(CppConsUI::Button& activator);

  private:
    CONSUI_DISABLE_COPY(AddBuddyWindow);
  };

  class AddChatWindow
  : public AddWindow
  {
  public:
    AddChatWindow(PurpleAccount *account, const char *name, const char *alias,
        const char *group);
    virtual ~AddChatWindow() {}

  protected:
    typedef std::map<std::string, CppConsUI::TreeView::NodeReference>
      ChatInfoMap;

    AccountOption *account_option;
    CppConsUI::TreeView::NodeReference account_option_ref;
    StringOption *alias_option;
    GroupOption *group_option;
    BooleanOption *autojoin_option;
    ChatInfoMap chat_info_map;

    // AddWindow
    virtual void onAddRequest(CppConsUI::Button& activator);

    void populateChatInfo(PurpleAccount *account);
    void onAccountChanged(CppConsUI::Button& activator, size_t new_entry,
        const char *title, intptr_t data);

  private:
    CONSUI_DISABLE_COPY(AddChatWindow);
  };

  class AddGroupWindow
  : public AddWindow
  {
  public:
    AddGroupWindow();
    virtual ~AddGroupWindow() {}

  protected:
    StringOption *name_option;

    // AddWindow
    virtual void onAddRequest(CppConsUI::Button& activator);

  private:
    CONSUI_DISABLE_COPY(AddGroupWindow);
  };

  PurpleBlistUiOps centerim_blist_ui_ops;
  PurpleBuddyList *buddylist;
  CppConsUI::TreeView *treeview;

  bool show_empty_groups;
  bool show_offline_buddies;
  ListMode list_mode;
  BuddySortMode buddy_sort_mode;
  GroupSortMode group_sort_mode;
  ColorizationMode colorization_mode;

  Filter *filter;
  char filter_buffer[256];
  // length in bytes
  size_t filter_buffer_length;
  // onscreen width
  size_t filter_buffer_onscreen_width;

  static BuddyList *my_instance;

  BuddyList();
  virtual ~BuddyList();
  CONSUI_DISABLE_COPY(BuddyList);

  static void init();
  static void finalize();
  friend class CenterIM;

  void load();
  void rebuildList();
  void updateList(int flags);
  void delayedGroupNodesInit();
  void updateCachedPreference(const char *name);
  bool isAnyAccountConnected();
  void filterHide();
  void actionOpenFilter();
  void actionDeleteChar();
  void declareBindables();

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
    { BUDDYLIST->request_add_group(); }

  void new_list(PurpleBuddyList *list);
  void new_node(PurpleBlistNode *node);
  void update(PurpleBuddyList *list, PurpleBlistNode *node);
  void remove(PurpleBuddyList *list, PurpleBlistNode *node);
  void destroy(PurpleBuddyList *list);
  void request_add_buddy(PurpleAccount *account, const char *username,
      const char *group, const char *alias);
  void request_add_chat(PurpleAccount *account, PurpleGroup *group,
      const char *alias, const char *name);
  void request_add_group();

  // called when any blist/* pref is changed
  static void blist_pref_change_(const char *name, PurplePrefType type,
      gconstpointer val, gpointer data)
    { reinterpret_cast<BuddyList*>(data)->blist_pref_change(name, type,
        val); }
  void blist_pref_change(const char *name, PurplePrefType type,
      gconstpointer val);
};

#endif // __BUDDYLIST_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
