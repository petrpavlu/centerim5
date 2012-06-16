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

/* Note: Parts of "add buddy/chat/group" code are based on Finch and Pidgin
 * code. */

#include "BuddyList.h"

#include "CenterIM.h"
#include "Footer.h"
#include "Log.h"

#include <cppconsui/Keys.h>
#include <cppconsui/Spacer.h>
#include "gettext.h"

BuddyList *BuddyList::instance = NULL;

BuddyList *BuddyList::Instance()
{
  return instance;
}

bool BuddyList::RestoreFocus()
{
  FOOTER->SetText(_("%s act conv, %s status menu, %s context menu"),
      "centerim|conversation-active", "centerim|accountstatusmenu",
      "buddylist|contextmenu");

  return CppConsUI::Window::RestoreFocus();
}

void BuddyList::UngrabFocus()
{
  FOOTER->SetText(NULL);
  CppConsUI::Window::UngrabFocus();
}

void BuddyList::Close()
{
  // BuddyList can't be closed
}

void BuddyList::OnScreenResized()
{
  MoveResizeRect(CENTERIM->GetScreenAreaSize(CenterIM::BUDDY_LIST_AREA));
}

BuddyList::BuddyList()
: Window(0, 0, 80, 24)
{
  SetColorScheme("buddylist");

  CppConsUI::HorizontalListBox *lbox
    = new CppConsUI::HorizontalListBox(AUTOSIZE, AUTOSIZE);
  lbox->AppendWidget(*(new CppConsUI::Spacer(1, AUTOSIZE)));
  treeview = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  lbox->AppendWidget(*treeview);
  lbox->AppendWidget(*(new CppConsUI::Spacer(1, AUTOSIZE)));
  AddWidget(*lbox, 0, 0);

  /* TODO Check if this has been moved to purple_blist_init(). Remove these
   * lines if it was as this will probably move to purple_init(), the
   * buddylist object should be available a lot more early and the uiops
   * should be set a lot more early. (All in all a lot of work.) */
  buddylist = purple_blist_new();
  buddylist->ui_data = this;
  purple_set_blist(buddylist);

  // load the pounces
  purple_pounces_load();

  // init prefs
  purple_prefs_add_none(CONF_PREFIX "/blist");
  purple_prefs_add_bool(CONF_PREFIX "/blist/show_empty_groups", false);
  purple_prefs_add_bool(CONF_PREFIX "/blist/show_offline_buddies", true);
  purple_prefs_add_string(CONF_PREFIX "/blist/colorization_mode", "none");
  purple_prefs_add_string(CONF_PREFIX "/blist/list_mode", "normal");

  UpdateCachedPreference(CONF_PREFIX "/blist/show_empty_groups");
  UpdateCachedPreference(CONF_PREFIX "/blist/show_offline_buddies");
  UpdateCachedPreference(CONF_PREFIX "/blist/colorization_mode");
  UpdateCachedPreference(CONF_PREFIX "/blist/list_mode");

  // connect callbacks
  purple_prefs_connect_callback(this, CONF_PREFIX "/blist",
      blist_pref_change_, this);

  // setup the callbacks for the buddylist
  memset(&centerim_blist_ui_ops, 0, sizeof(centerim_blist_ui_ops));
  centerim_blist_ui_ops.new_list = new_list_;
  centerim_blist_ui_ops.new_node = new_node_;
  //centerim_blist_ui_ops.show = show_;
  centerim_blist_ui_ops.update = update_;
  centerim_blist_ui_ops.remove = remove_;
  centerim_blist_ui_ops.destroy = destroy_;
  //centerim_blist_ui_ops.set_visible = set_visible_;
  centerim_blist_ui_ops.request_add_buddy = request_add_buddy_;
  centerim_blist_ui_ops.request_add_chat = request_add_chat_;
  centerim_blist_ui_ops.request_add_group = request_add_group_;
  // since libpurple 2.6.0
  //centerim_blist_ui_ops.save_node = save_node_;
  //centerim_blist_ui_ops.remove_node = remove_node_;
  //centerim_blist_ui_ops.save_account = save_account_;
  purple_blist_set_ui_ops(&centerim_blist_ui_ops);

  COREMANAGER->TimeoutOnceConnect(sigc::mem_fun(this, &BuddyList::Load), 0);
}

BuddyList::~BuddyList()
{
  purple_blist_set_ui_ops(NULL);
  purple_prefs_disconnect_by_handle(this);
}

void BuddyList::Init()
{
  g_assert(!instance);

  instance = new BuddyList;
  instance->Show();
}

void BuddyList::Finalize()
{
  g_assert(instance);

  delete instance;
  instance = NULL;
}

void BuddyList::Load()
{
  // load the buddy list from ~/.centerim5/blist.xml
  purple_blist_load();

  DelayedGroupNodesInit();
}

void BuddyList::RebuildList()
{
  treeview->Clear();

  PurpleBlistNode *node = purple_blist_get_root();
  while (node) {
    new_node(node);
    node = purple_blist_node_next(node, TRUE);
  }

  DelayedGroupNodesInit();
}

void BuddyList::DelayedGroupNodesInit()
{
  // delayed group nodes init
  PurpleBlistNode *node = purple_blist_get_root();
  while (node) {
    if (PURPLE_BLIST_NODE_IS_GROUP(node)) {
      BuddyListGroup *gnode = reinterpret_cast<BuddyListGroup*>(node->ui_data);
      if (gnode)
        gnode->DelayedInit();
    }

    node = purple_blist_node_get_sibling_next(node);
  }
}

void BuddyList::UpdateCachedPreference(const char *name)
{
  if (!strcmp(name, CONF_PREFIX "/blist/show_empty_groups"))
    show_empty_groups = purple_prefs_get_bool(name);
  else if (!strcmp(name, CONF_PREFIX "/blist/show_offline_buddies"))
    show_offline_buddies = purple_prefs_get_bool(name);
  else if (!strcmp(name, CONF_PREFIX "/blist/colorization_mode")) {
    const char *value = purple_prefs_get_string(name);
    if (!strcmp(value, "status"))
      colorization_mode = COLOR_BY_STATUS;
    else if (!strcmp(value, "account"))
      colorization_mode = COLOR_BY_ACCOUNT;
    else
      colorization_mode = COLOR_NONE;
  }
  else if (!strcmp(name, CONF_PREFIX "/blist/list_mode")) {
    const char *value = purple_prefs_get_string(name);
    if (!strcmp(value, "flat"))
      list_mode = LIST_FLAT;
    else
      list_mode = LIST_NORMAL;
  }
}

bool BuddyList::CheckAnyAccountConnected()
{
  bool connected = false;
  for (GList *list = purple_accounts_get_all(); list; list = list->next) {
    PurpleAccount *account = reinterpret_cast<PurpleAccount*>(list->data);
    if (purple_account_is_connected(account)) {
      connected = true;
      break;
    }
  }
  if (!connected)
    LOG->Message(_("There are no connected accounts."));
  return connected;
}

void BuddyList::new_list(PurpleBuddyList *list)
{
  if (buddylist != list)
    LOG->Error(_("Different Buddylist detected!"));
}

void BuddyList::new_node(PurpleBlistNode *node)
{
  g_return_if_fail(!node->ui_data);

  if (PURPLE_BLIST_NODE_IS_GROUP(node) && list_mode == BuddyList::LIST_FLAT) {
    // flat mode = no groups
    return;
  }

  BuddyListNode *bnode = BuddyListNode::CreateNode(node);

  if (bnode) {
    BuddyListNode *parent = bnode->GetParentNode();
    CppConsUI::TreeView::NodeReference nref = treeview->AppendNode(
        parent ? parent->GetRefNode() : treeview->GetRootNode(), *bnode);
    treeview->SetCollapsed(nref, true);
    bnode->SetRefNode(nref);
    bnode->Update();

    if (PURPLE_BLIST_NODE_IS_CONTACT(node))
      treeview->SetNodeStyle(nref, CppConsUI::TreeView::STYLE_VOID);
  }
}

void BuddyList::update(PurpleBuddyList *list, PurpleBlistNode *node)
{
  // not cool, but necessary because libpurple doesn't always behave nice
  if (!node->ui_data)
    new_node(node);

  BuddyListNode *bnode = reinterpret_cast<BuddyListNode*>(node->ui_data);
  g_return_if_fail(bnode);

  // update the node data
  bnode->Update();

  if (node->parent)
    update(list, node->parent);
}

void BuddyList::remove(PurpleBuddyList *list, PurpleBlistNode *node)
{
  BuddyListNode *bnode = reinterpret_cast<BuddyListNode*>(node->ui_data);
  g_return_if_fail(bnode);

  treeview->DeleteNode(bnode->GetRefNode(), false);

  if (node->parent)
    update(list, node->parent);
}

void BuddyList::destroy(PurpleBuddyList */*list*/)
{
}

void BuddyList::request_add_buddy(PurpleAccount *account,
    const char *username, const char *group, const char *alias)
{
  if (!CheckAnyAccountConnected())
    return;

  PurpleRequestFields *fields = purple_request_fields_new();
  PurpleRequestFieldGroup *g = purple_request_field_group_new(NULL);

  purple_request_fields_add_group(fields, g);

  PurpleRequestField *f;
  f = purple_request_field_account_new("account", _("Account"), account);
  purple_request_field_group_add_field(g, f);
  f = purple_request_field_string_new("name", _("Buddy name"), username,
      FALSE);
  purple_request_field_group_add_field(g, f);
  f = purple_request_field_string_new("alias", _("Alias"), alias, FALSE);
  purple_request_field_group_add_field(g, f);

  f = purple_request_field_choice_new("group", _("Group"), 0);
  bool add_default_group = true;
  int dval = 0;
  bool dval_set = false;
  for (PurpleBlistNode *i = purple_blist_get_root(); i; i = i->next)
    if (PURPLE_BLIST_NODE_IS_GROUP(i)) {
      const char *cur_name = purple_group_get_name(
          reinterpret_cast<PurpleGroup*>(i));
      purple_request_field_choice_add(f, cur_name);
      if (!dval_set) {
        if (group && !strcmp(group, cur_name)) {
          purple_request_field_choice_set_default_value(f, dval);
          dval_set = true;
        }
        dval++;
      }
      add_default_group = false;
    }
  if (add_default_group)
      purple_request_field_choice_add(f, _("Buddies"));
  purple_request_field_group_add_field(g, f);

  purple_request_fields(NULL, _("Add buddy"), NULL, NULL, fields, _("Add"),
      G_CALLBACK(add_buddy_ok_cb_), CANCEL_BUTTON_TEXT, NULL, NULL, NULL,
      NULL, this);
}

void BuddyList::request_add_chat(PurpleAccount *account, PurpleGroup *group,
    const char */*alias*/, const char */*name*/)
{
  if (!CheckAnyAccountConnected())
    return;

  if (account) {
    PurpleConnection *gc = purple_account_get_connection(account);

    if (!PURPLE_PLUGIN_PROTOCOL_INFO(gc->prpl)->join_chat) {
      LOG->Message(_("This protocol does not support chat rooms."));
      return;
    }
  }
  else {
    // find an account with chat capabilities
    for (GList *l = purple_connections_get_all(); l; l = l->next) {
      PurpleConnection *gc = reinterpret_cast<PurpleConnection*>(l->data);

      if (PURPLE_PLUGIN_PROTOCOL_INFO(gc->prpl)->join_chat) {
        account = purple_connection_get_account(gc);
        break;
      }
    }

    if (!account) {
      LOG->Message(_("You are not currently signed on with any "
            "protocols that have the ability to chat."));
      return;
    }
  }

  PurpleRequestFields *fields = purple_request_fields_new();
  PurpleRequestFieldGroup *g = purple_request_field_group_new(NULL);

  purple_request_fields_add_group(fields, g);

  PurpleRequestField *f;
  f = purple_request_field_account_new("account", _("Account"), account);
  purple_request_field_group_add_field(g, f);
  f = purple_request_field_string_new("name", _("Chat name"), NULL, FALSE);
  purple_request_field_group_add_field(g, f);
  f = purple_request_field_string_new("alias", _("Alias"), NULL, FALSE);
  purple_request_field_group_add_field(g, f);

  f = purple_request_field_choice_new("group", _("Group"), 0);
  bool add_default_group = true;
  int dval = 0;
  bool dval_set = false;
  for (PurpleBlistNode *i = purple_blist_get_root(); i; i = i->next)
    if (PURPLE_BLIST_NODE_IS_GROUP(i)) {
      PurpleGroup *cur_group = reinterpret_cast<PurpleGroup*>(i);
      const char *cur_name = purple_group_get_name(cur_group);
      purple_request_field_choice_add(f, cur_name);
      if (!dval_set) {
        if (group == cur_group) {
          purple_request_field_choice_set_default_value(f, dval);
          dval_set = true;
        }
        dval++;
      }
      add_default_group = false;
    }
  if (add_default_group)
      purple_request_field_choice_add(f, _("Chats"));
  purple_request_field_group_add_field(g, f);

  f = purple_request_field_bool_new("autojoin", _("Auto-join"), FALSE);
  purple_request_field_group_add_field(g, f);

  purple_request_fields(NULL, _("Add chat"), NULL, NULL, fields, _("Add"),
      G_CALLBACK(add_chat_ok_cb_), CANCEL_BUTTON_TEXT, NULL, NULL, NULL, NULL,
      this);
}

void BuddyList::request_add_group()
{
  if (!CheckAnyAccountConnected())
    return;

  purple_request_input(NULL, _("Add Group"),
      _("Please enter the name of the group to be added."), NULL, NULL, FALSE,
      FALSE, NULL, _("Add"), G_CALLBACK(add_group_ok_cb_), _("Cancel"), NULL,
      NULL, NULL, NULL, this);
}

void BuddyList::add_buddy_ok_cb(PurpleRequestFields *fields)
{
  PurpleAccount *account =
    purple_request_fields_get_account(fields, "account");
  const char *name = purple_request_fields_get_string(fields, "name");
  const char *alias = purple_request_fields_get_string(fields, "alias");
  int selected = purple_request_fields_get_choice(fields, "group");
  GList *list = purple_request_field_choice_get_labels(
      purple_request_fields_get_field(fields, "group"));
  const char *group
    = reinterpret_cast<const char*>(g_list_nth_data(list, selected));

  bool err = false;
  if (!account) {
    LOG->Message(_("No account specified."));
    err = true;
  }
  else if (!purple_account_is_connected(account)) {
    LOG->Message(_("Selected account is not connected."));
    err = true;
  }
  if (!name || !name[0]) {
    LOG->Message(_("No buddy name specified."));
    err = true;
  }
  if (!group || !group[0]) {
    LOG->Message(_("No group name specified."));
    err = true;
  }
  if (err) {
    purple_blist_request_add_buddy(account, name, group, alias);
    return;
  }

  PurpleGroup *g = purple_find_group(group);
  if (!g) {
    g = purple_group_new(group);
    purple_blist_add_group(g, NULL);
  }
  PurpleBuddy *b = purple_find_buddy_in_group(account, name, g);
  if (b) {
    LOG->Message(_("Specified buddy is already in the list."));
    return;
  }

  if (alias && !alias[0])
    alias = NULL;
  b = purple_buddy_new(account, name, alias);
  purple_blist_add_buddy(b, NULL, g, NULL);
  purple_account_add_buddy(account, b);
}

void BuddyList::add_chat_ok_cb(PurpleRequestFields *fields)
{
  PurpleAccount *account =
    purple_request_fields_get_account(fields, "account");
  const char *name = purple_request_fields_get_string(fields, "name");
  const char *alias = purple_request_fields_get_string(fields, "alias");
  int selected = purple_request_fields_get_choice(fields, "group");
  GList *list = purple_request_field_choice_get_labels(
      purple_request_fields_get_field(fields, "group"));
  const char *group
    = reinterpret_cast<const char*>(g_list_nth_data(list, selected));
  bool autojoin = purple_request_fields_get_bool(fields, "autojoin");

  bool err = false;
  if (!account) {
    LOG->Message(_("No account specified."));
    err = true;
  }
  else if (!purple_account_is_connected(account)) {
    LOG->Message(_("Selected account is not connected."));
    err = true;
  }
  else {
    PurpleConnection *gc = purple_account_get_connection(account);
    PurplePluginProtocolInfo *info = PURPLE_PLUGIN_PROTOCOL_INFO(
        purple_connection_get_prpl(gc));
    if (!info->join_chat) {
      LOG->Message(_("This protocol does not support chat rooms."));
      account = NULL;
      err = true;
    }
  }
  if (!name || !name[0]) {
    LOG->Message(_("No buddy name specified."));
    err = true;
  }
  if (!group || !group[0]) {
    LOG->Message(_("No group name specified."));
    err = true;
  }
  if (err) {
    purple_blist_request_add_chat(account, purple_find_group(group),
        alias, name);
    return;
  }

  PurpleConnection *gc = purple_account_get_connection(account);
  PurplePluginProtocolInfo *info = PURPLE_PLUGIN_PROTOCOL_INFO(
      purple_connection_get_prpl(gc));
  GHashTable *hash = NULL;
  if (info->chat_info_defaults)
    hash = info->chat_info_defaults(gc, name);

  PurpleChat *chat = purple_chat_new(account, name, hash);

  if (chat) {
    PurpleGroup* g = purple_find_group(group);
    if (!g) {
      g = purple_group_new(group);
      purple_blist_add_group(g, NULL);
    }
    purple_blist_add_chat(chat, g, NULL);
    if (alias && alias[0])
      purple_blist_alias_chat(chat, alias);
    purple_blist_node_set_bool(reinterpret_cast<PurpleBlistNode*>(chat),
        PACKAGE_NAME "-autojoin", autojoin);
  }
}

void BuddyList::add_group_ok_cb(const char *name)
{
  if (!name || !name[0]) {
    LOG->Message(_("No group name specified."));
    purple_blist_request_add_group();
    return;
  }

  PurpleGroup *group = purple_group_new(name);
  purple_blist_add_group(group, NULL);
}

void BuddyList::blist_pref_change(const char *name, PurplePrefType /*type*/,
    gconstpointer /*val*/)
{
  // blist/* preference changed
  UpdateCachedPreference(name);

  if (!strcmp(name, CONF_PREFIX "/blist/list_mode")) {
    RebuildList();
    return;
  }

  bool groups_only = false;
  if (!strcmp(name, CONF_PREFIX "/blist/show_empty_groups"))
    groups_only = true;

  PurpleBlistNode *node = purple_blist_get_root();
  while (node) {
    if (!groups_only || PURPLE_BLIST_NODE_IS_GROUP(node)) {
      BuddyListNode *bnode = reinterpret_cast<BuddyListNode*>(node->ui_data);
      if (bnode)
        bnode->Update();
    }

    node = purple_blist_node_next(node, TRUE);
  }
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
