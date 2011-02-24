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

#include "BuddyList.h"

#include "CenterIM.h"
#include "Log.h"

#include <cppconsui/Keys.h>
#include <cppconsui/Spacer.h>
#include "gettext.h"

BuddyList *BuddyList::instance = NULL;

BuddyList *BuddyList::Instance()
{
  return instance;
}

BuddyList::BuddyList()
: Window(0, 0, 80, 24)
{
  SetColorScheme("buddylist");

  HorizontalListBox *lbox = new HorizontalListBox(AUTOSIZE, AUTOSIZE);
  lbox->AppendWidget(*(new Spacer(1, AUTOSIZE)));
  treeview = new TreeView(AUTOSIZE, AUTOSIZE);
  lbox->AppendWidget(*treeview);
  lbox->AppendWidget(*(new Spacer(1, AUTOSIZE)));
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
  purple_prefs_add_bool(CONF_PREFIX "/blist/show_offline_buddies", true);
  purple_prefs_add_bool(CONF_PREFIX "/blist/show_empty_groups", true);

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
  // loads the buddy list from ~/.centerim5/blist.xml
  purple_blist_load();
}

void BuddyList::Close()
{
}

void BuddyList::ScreenResized()
{
  MoveResizeRect(CENTERIM->GetScreenAreaSize(CenterIM::BUDDY_LIST_AREA));
}

void BuddyList::new_list(PurpleBuddyList *list)
{
  if (buddylist != list)
    LOG->Error(_("Different Buddylist detected!\n"));
}

void BuddyList::new_node(PurpleBlistNode *node)
{
  g_assert(!node->ui_data);

  BuddyListNode *bnode = BuddyListNode::CreateNode(node);

  if (bnode) {
    BuddyListNode *parent = bnode->GetParentNode();
    TreeView::NodeReference nref = treeview->AppendNode(
        parent ? parent->GetRefNode() : treeview->GetRootNode(), *bnode);
    treeview->CollapseNode(nref);
    bnode->SetRefNode(nref);
    bnode->Update();

    if (PURPLE_BLIST_NODE_IS_CONTACT(node))
      treeview->SetNodeStyle(nref, TreeView::STYLE_VOID);
  }
}

void BuddyList::update(PurpleBuddyList *list, PurpleBlistNode *node)
{
  // not cool, but necessary because libpurple doesn't always behave nice
  if (!node->ui_data)
    new_node(node);

  BuddyListNode *bnode = reinterpret_cast<BuddyListNode *>(node->ui_data);

  // update the node data
  bnode->Update();

  if (node->parent)
    update(list, node->parent);
}

void BuddyList::remove(PurpleBuddyList *list, PurpleBlistNode *node)
{
  BuddyListNode *bnode = reinterpret_cast<BuddyListNode *>(node->ui_data);
  g_return_if_fail(bnode);

  // TODO check for subnodes (if this is a group for instance)
  treeview->DeleteNode(bnode->GetRefNode(), false);

  if (node->parent)
    update(list, node->parent);
}

void BuddyList::destroy(PurpleBuddyList *list)
{
}

void BuddyList::request_add_buddy(PurpleAccount *account,
    const char *username, const char *group, const char *alias)
{
  bool connected = false;
  for (GList *list = purple_accounts_get_all(); list; list = list->next) {
    PurpleAccount *account = reinterpret_cast<PurpleAccount*>(list->data);
    if (purple_account_is_connected(account)) {
      connected = true;
      break;
    }
  }
  if (!connected) {
    LOG->Message(_("No connected accounts"));
    return;
  }

  PurpleRequestFields *fields = purple_request_fields_new();
  PurpleRequestFieldGroup *g = purple_request_field_group_new(NULL);

  purple_request_fields_add_group(fields, g);

  PurpleRequestField *f;
  f = purple_request_field_account_new("account", _("Account"), NULL);
  purple_request_field_group_add_field(g, f);
  f = purple_request_field_string_new("name", _("Buddy name"), NULL, FALSE);
  purple_request_field_group_add_field(g, f);
  f = purple_request_field_string_new("alias", _("Alias"), NULL, FALSE);
  purple_request_field_group_add_field(g, f);

  f = purple_request_field_choice_new("group", _("Group"), 0);
  bool add_default_group = true;
  for (PurpleBlistNode *i = purple_blist_get_root(); i; i = i->next)
    if (PURPLE_BLIST_NODE_IS_GROUP(i)) {
      purple_request_field_choice_add(f,
          purple_group_get_name(reinterpret_cast<PurpleGroup*>(i)));
      add_default_group = false;
    }
  if (add_default_group)
      purple_request_field_choice_add(f, _("Buddies"));
  purple_request_field_group_add_field(g, f);

  purple_request_fields(NULL, _("Add buddy"), NULL, NULL, fields, _("Add"),
      G_CALLBACK(add_buddy_ok_cb_), _("Cancel"), NULL, NULL, NULL, NULL,
      this);
}

void BuddyList::add_buddy_ok_cb(PurpleRequestFields *fields)
{
  PurpleAccount *account = purple_request_fields_get_account(fields, "account");
  const char *who = purple_request_fields_get_string(fields, "name");
  const char *whoalias = purple_request_fields_get_string(fields, "alias");
  int selected = purple_request_fields_get_choice(fields, "group");
  GList *list = purple_request_field_choice_get_labels(
      purple_request_fields_get_field(fields, "group"));
  const char *grp
    = reinterpret_cast<const char *>(g_list_nth_data(list, selected));

  if (!who || !who[0])
    return;

  PurpleGroup *g = purple_find_group(grp);
  PurpleBuddy *b = purple_find_buddy_in_group(account, who, g);
  if (b)
    return;

  b = purple_buddy_new(account, who, whoalias);
  purple_blist_add_buddy(b, NULL, g, NULL);
  purple_account_add_buddy(account, b);
}
