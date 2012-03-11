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

#include "BuddyListNode.h"

#include "BuddyList.h"
#include "Conversations.h"
#include "Utils.h"

#include <cppconsui/ConsuiCurses.h>
#include <cppconsui/ColorScheme.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/Keys.h>
#include "gettext.h"

BuddyListNode *BuddyListNode::CreateNode(PurpleBlistNode *node)
{
  if (PURPLE_BLIST_NODE_IS_BUDDY(node))
    return new BuddyListBuddy(node);
  if (PURPLE_BLIST_NODE_IS_CHAT(node))
    return new BuddyListChat(node);
  if (PURPLE_BLIST_NODE_IS_CONTACT(node))
    return new BuddyListContact(node);
  if (PURPLE_BLIST_NODE_IS_GROUP(node))
    return new BuddyListGroup(node);

  LOG->Warning(_("Unrecognized BuddyList node."));
  return NULL;
}

void BuddyListNode::SetParent(CppConsUI::Container& parent)
{
  Button::SetParent(parent);

  treeview = dynamic_cast<CppConsUI::TreeView*>(&parent);
  g_assert(treeview);
}

void BuddyListNode::Update()
{
  BuddyListNode *parent_node = GetParentNode();
  // the parent could have changed, so re-parent the node
  if (parent_node)
    treeview->SetNodeParent(ref, parent_node->GetRefNode());
}

void BuddyListNode::SortIn()
{
  CppConsUI::TreeView::NodeReference parent_ref;
  BuddyListNode *parent_node = GetParentNode();
  if (parent_node)
    parent_ref = parent_node->GetRefNode();
  else
    parent_ref = treeview->GetRootNode();

  CppConsUI::TreeView::SiblingIterator i;
  for (i = parent_ref.begin(); i != parent_ref.end(); i++) {
    // skip this node
    if (i == ref)
      continue;

    BuddyListNode *n = dynamic_cast<BuddyListNode*>(i->GetWidget());
    g_assert(n);

    if (LessThan(*n)) {
      treeview->MoveNodeBefore(ref, i);
      break;
    }
  }
  // the ref is last in a list
  if (i == parent_ref.end())
    treeview->MoveNodeAfter(ref, --i);
}

BuddyListNode *BuddyListNode::GetParentNode() const
{
  PurpleBlistNode *parent = node->parent;

  if (!parent || !parent->ui_data)
    return NULL;

  return reinterpret_cast<BuddyListNode*>(parent->ui_data);
}

BuddyListNode::BuddyListNode(PurpleBlistNode *node)
: treeview(NULL), node(node)
{
  node->ui_data = this;
  signal_activate.connect(sigc::mem_fun(this, &BuddyListNode::OnActivate));
  DeclareBindables();
}

BuddyListNode::~BuddyListNode()
{
  node->ui_data = NULL;
}

const char *BuddyListNode::GetBuddyStatus(PurpleBuddy *buddy) const
{
  if (!purple_account_is_connected(purple_buddy_get_account(buddy)))
    return "";

  PurplePresence *presence = purple_buddy_get_presence(buddy);
  PurpleStatus *status = purple_presence_get_active_status(presence);
  return Utils::GetStatusIndicator(status);
}

int BuddyListNode::GetBuddyStatusWeight(PurpleBuddy *buddy) const
{
  if (!purple_account_is_connected(purple_buddy_get_account(buddy)))
    return 0;

  PurplePresence *presence = purple_buddy_get_presence(buddy);
  PurpleStatus *status = purple_presence_get_active_status(presence);
  PurpleStatusType *status_type = purple_status_get_type(status);
  PurpleStatusPrimitive prim = purple_status_type_get_primitive(status_type);

  switch (prim) {
    case PURPLE_STATUS_OFFLINE:
      return 0;
    default:
      return 1;
    case PURPLE_STATUS_UNSET:
      return 2;
    case PURPLE_STATUS_UNAVAILABLE:
      return 3;
    case PURPLE_STATUS_AWAY:
      return 4;
    case PURPLE_STATUS_EXTENDED_AWAY:
      return 5;
    case PURPLE_STATUS_MOBILE:
      return 6;
#if PURPLE_VERSION_CHECK(2, 7, 0)
    case PURPLE_STATUS_MOOD:
      return 7;
#endif
    case PURPLE_STATUS_TUNE:
      return 8;
    case PURPLE_STATUS_INVISIBLE:
      return 9;
    case PURPLE_STATUS_AVAILABLE:
      return 10;
  }
}

void BuddyListNode::ActionOpenContextMenu()
{
  OpenContextMenu();
}

void BuddyListNode::DeclareBindables()
{
  DeclareBindable("buddylist", "contextmenu", sigc::mem_fun(this,
        &BuddyListNode::ActionOpenContextMenu),
      InputProcessor::BINDABLE_NORMAL);
}

bool BuddyListBuddy::LessThan(const BuddyListNode& other) const
{
  const BuddyListBuddy *o = dynamic_cast<const BuddyListBuddy*>(&other);
  if (o) {
    int a = GetBuddyStatusWeight(buddy);
    int b = GetBuddyStatusWeight(o->buddy);
    if (a != b)
      return a > b;

    return g_utf8_collate(purple_buddy_get_alias(buddy),
        purple_buddy_get_alias(o->buddy)) < 0;
  }
  return false;
}

void BuddyListBuddy::Update()
{
  BuddyListNode::Update();

  const char *status = GetBuddyStatus(buddy);
  const char *alias = purple_buddy_get_alias(buddy);
  if (status[0]) {
    char *text = g_strdup_printf("%s %s", status, alias);
    SetText(text);
    g_free(text);
  }
  else
    SetText(alias);

  SortIn();

  UpdateColorScheme();

  if (!purple_account_is_connected(purple_buddy_get_account(buddy))) {
    // hide if account is offline
    SetVisibility(false);
  }
  else
    SetVisibility(BUDDYLIST->GetShowOfflineBuddiesPref() || status[0]);
}

void BuddyListBuddy::OnActivate(Button& activator)
{
  PurpleAccount *account = purple_buddy_get_account(buddy);
  const char *name = purple_buddy_get_name(buddy);
  PurpleConversation *conv = purple_find_conversation_with_account(
      PURPLE_CONV_TYPE_IM, name, account);

  if (!conv)
    conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, account, name);
  purple_conversation_present(conv);
}

const char *BuddyListBuddy::ToString() const
{
  return purple_buddy_get_alias(buddy);
}

BuddyListBuddy::ContextMenu::ContextMenu(BuddyListBuddy& parent)
: MenuWindow(parent, AUTOSIZE, AUTOSIZE)
, parent(&parent)
{
  AppendItem(_("Alias..."), sigc::mem_fun(this,
        &BuddyListBuddy::ContextMenu::OnChangeAlias));
  AppendItem(_("Delete..."), sigc::mem_fun(this,
        &BuddyListBuddy::ContextMenu::OnRemove));
}

void BuddyListBuddy::ContextMenu::ChangeAliasResponseHandler(
    CppConsUI::InputDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_OK:
      break;
    default:
      return;
  }

  PurpleBuddy *buddy = parent->GetPurpleBuddy();
  purple_blist_alias_buddy(buddy, activator.GetText());
  serv_alias_buddy(buddy);

  // close context menu
  Close();
}

void BuddyListBuddy::ContextMenu::OnChangeAlias(Button& activator)
{
  PurpleBuddy *buddy = parent->GetPurpleBuddy();
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(
      _("Alias"), purple_buddy_get_alias(buddy));
  dialog->signal_response.connect(sigc::mem_fun(this,
        &BuddyListBuddy::ContextMenu::ChangeAliasResponseHandler));
  dialog->Show();
}

void BuddyListBuddy::ContextMenu::RemoveResponseHandler(
    CppConsUI::MessageDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_OK:
      break;
    default:
      return;
  }

  PurpleBuddy *buddy = parent->GetPurpleBuddy();
  purple_account_remove_buddy(purple_buddy_get_account(buddy), buddy,
      purple_buddy_get_group(buddy));

  /* Close the context menu before the buddy is deleted because its deletion
   * can lead to destruction of this object. */
  Close();

  purple_blist_remove_buddy(buddy);
}

void BuddyListBuddy::ContextMenu::OnRemove(Button& activator)
{
  PurpleBuddy *buddy = parent->GetPurpleBuddy();
  char *msg = g_strdup_printf(
      _("Are you sure you want to delete buddy %s from the list?"),
      purple_buddy_get_alias(buddy));
  CppConsUI::MessageDialog *dialog = new CppConsUI::MessageDialog(
      _("Buddy deletion"), msg);
  g_free(msg);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &BuddyListBuddy::ContextMenu::RemoveResponseHandler));
  dialog->Show();
}

int BuddyListBuddy::GetColorPair(const char *widget, const char *property)
  const
{
  if (BUDDYLIST->GetColorizationMode() != BuddyList::COLOR_BY_ACCOUNT
      || strcmp(property, "normal"))
    return Button::GetColorPair(widget, property);

  PurpleAccount *account = purple_buddy_get_account(buddy);
  int fg = purple_account_get_ui_int(account, "centerim5",
      "buddylist-foreground-color", CppConsUI::Curses::Color::DEFAULT);
  int bg = purple_account_get_ui_int(account, "centerim5",
      "buddylist-background-color", CppConsUI::Curses::Color::DEFAULT);

  CppConsUI::ColorScheme::Color c(fg, bg);
  return COLORSCHEME->GetColorPair(c);
}

void BuddyListBuddy::OpenContextMenu()
{
  ContextMenu *w = new ContextMenu(*this);
  w->Show();
}

BuddyListBuddy::BuddyListBuddy(PurpleBlistNode *node)
: BuddyListNode(node)
{
  SetColorScheme("buddylistbuddy");

  buddy = reinterpret_cast<PurpleBuddy*>(node);
}

void BuddyListBuddy::UpdateColorScheme()
{
  char *new_scheme;

  switch (BUDDYLIST->GetColorizationMode()) {
    case BuddyList::COLOR_BY_STATUS:
      new_scheme = Utils::GetColorSchemeString("buddylistbuddy", buddy);
      SetColorScheme(new_scheme);
      g_free(new_scheme);
      break;
    default:
      // note: COLOR_BY_ACCOUNT case is handled by BuddyListBuddy::Draw()
      SetColorScheme("buddylistbuddy");
      break;
  }
}

bool BuddyListChat::LessThan(const BuddyListNode& other) const
{
  const BuddyListChat *o = dynamic_cast<const BuddyListChat*>(&other);
  if (o)
    return g_utf8_collate(purple_chat_get_name(chat),
        purple_chat_get_name(o->chat)) < 0;
  return false;
}

void BuddyListChat::Update()
{
  BuddyListNode::Update();

  SetText(purple_chat_get_name(chat));

  SortIn();

  // hide if account is offline
  SetVisibility(purple_account_is_connected(purple_chat_get_account(chat)));
}

void BuddyListChat::OnActivate(Button& activator)
{
  PurpleAccount *account = purple_chat_get_account(chat);
  PurplePluginProtocolInfo *prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(
      purple_find_prpl(purple_account_get_protocol_id(account)));
  GHashTable *components = purple_chat_get_components(chat);

  char *chat_name = NULL;
  if (prpl_info && prpl_info->get_chat_name)
    chat_name = prpl_info->get_chat_name(components);

  const char *name;
  if (chat_name)
    name = chat_name;
  else
    name = purple_chat_get_name(chat);

  PurpleConversation *conv = purple_find_conversation_with_account(
      PURPLE_CONV_TYPE_CHAT, name, account);
  if (conv)
    purple_conversation_present(conv);

  serv_join_chat(purple_account_get_connection(account), components);

  g_free(chat_name);
}

const char *BuddyListChat::ToString() const
{
  return purple_chat_get_name(chat);
}

BuddyListChat::ContextMenu::ContextMenu(BuddyListChat& parent)
: MenuWindow(parent, AUTOSIZE, AUTOSIZE)
, parent(&parent)
{
  AppendItem(_("Alias..."), sigc::mem_fun(this,
        &BuddyListChat::ContextMenu::OnChangeAlias));
  AppendItem(_("Delete..."), sigc::mem_fun(this,
        &BuddyListChat::ContextMenu::OnRemove));
}

void BuddyListChat::ContextMenu::ChangeAliasResponseHandler(
    CppConsUI::InputDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_OK:
      break;
    default:
      return;
  }

  PurpleChat *chat = parent->GetPurpleChat();
  purple_blist_alias_chat(chat, activator.GetText());

  // close context menu
  Close();
}

void BuddyListChat::ContextMenu::OnChangeAlias(Button& activator)
{
  PurpleChat *chat = parent->GetPurpleChat();
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(
      _("Alias"), purple_chat_get_name(chat));
  dialog->signal_response.connect(sigc::mem_fun(this,
        &BuddyListChat::ContextMenu::ChangeAliasResponseHandler));
  dialog->Show();
}

void BuddyListChat::ContextMenu::RemoveResponseHandler(
    CppConsUI::MessageDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_OK:
      break;
    default:
      return;
  }

  PurpleChat *chat = parent->GetPurpleChat();

  /* Close the context menu before the chat is deleted because its deletion
   * can lead to destruction of this object. */
  Close();

  purple_blist_remove_chat(chat);
}

void BuddyListChat::ContextMenu::OnRemove(Button& activator)
{
  PurpleChat *chat = parent->GetPurpleChat();
  char *msg = g_strdup_printf(
      _("Are you sure you want to delete chat %s from the list?"),
      purple_chat_get_name(chat));
  CppConsUI::MessageDialog *dialog = new CppConsUI::MessageDialog(
      _("Chat deletion"), msg);
  g_free(msg);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &BuddyListChat::ContextMenu::RemoveResponseHandler));
  dialog->Show();
}

void BuddyListChat::OpenContextMenu()
{
  ContextMenu *w = new ContextMenu(*this);
  w->Show();
}

BuddyListChat::BuddyListChat(PurpleBlistNode *node)
: BuddyListNode(node)
{
  SetColorScheme("buddylistchat");

  chat = reinterpret_cast<PurpleChat*>(node);
}

bool BuddyListContact::LessThan(const BuddyListNode& other) const
{
  const BuddyListContact *o = dynamic_cast<const BuddyListContact*>(&other);
  if (o) {
    PurpleBuddy *buddy_a = purple_contact_get_priority_buddy(contact);
    PurpleBuddy *buddy_b = purple_contact_get_priority_buddy(o->contact);
    int a = GetBuddyStatusWeight(buddy_a);
    int b = GetBuddyStatusWeight(buddy_b);
    if (a != b)
      return a > b;

    return g_utf8_collate(purple_contact_get_alias(contact),
        purple_contact_get_alias(o->contact)) < 0;
  }
  return false;
}

void BuddyListContact::Update()
{
  BuddyListNode::Update();

  PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact);
  const char *status = GetBuddyStatus(buddy);
  const char *alias = purple_contact_get_alias(contact);
  if (status[0]) {
    char *text = g_strdup_printf("%s %s", status, alias);
    SetText(text);
    g_free(text);
  }
  else
    SetText(alias);

  SortIn();

  UpdateColorScheme();

  if (!purple_account_is_connected(purple_buddy_get_account(buddy))) {
    // hide if account is offline
    SetVisibility(false);
  }
  else
    SetVisibility(BUDDYLIST->GetShowOfflineBuddiesPref() || status[0]);
}

void BuddyListContact::OnActivate(Button& activator)
{
  PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact);
  void *ui_data = buddy->node.ui_data;
  if (ui_data) {
    BuddyListNode *bnode = reinterpret_cast<BuddyListNode*>(ui_data);
    bnode->OnActivate(activator);
  }
}

const char *BuddyListContact::ToString() const
{
  return purple_contact_get_alias(contact);
}

void BuddyListContact::SetRefNode(CppConsUI::TreeView::NodeReference n)
{
  BuddyListNode::SetRefNode(n);
  treeview->SetNodeStyle(n, CppConsUI::TreeView::STYLE_VOID);
}

BuddyListContact::ContextMenu::ContextMenu(BuddyListContact& parent)
: MenuWindow(parent, AUTOSIZE, AUTOSIZE)
, parent(&parent)
{
  CppConsUI::Button *button;
  PurpleGroup *group;
  PurpleBlistNode *node;

  AppendItem(_("Alias..."), sigc::mem_fun(this,
        &BuddyListContact::ContextMenu::OnChangeAlias));
  AppendItem(_("Delete..."), sigc::mem_fun(this,
        &BuddyListContact::ContextMenu::OnRemove));

  CppConsUI::MenuWindow *groups = new CppConsUI::MenuWindow(*this,
      AUTOSIZE, AUTOSIZE);

  for (node = purple_blist_get_root(); node;
      node = purple_blist_node_get_sibling_next(node))
  {
    if (!PURPLE_BLIST_NODE_IS_GROUP(node))
      continue;

    group = reinterpret_cast<PurpleGroup*>(node);

    button = groups->AppendItem(purple_group_get_name(
        group), sigc::bind(sigc::mem_fun(this,
            &BuddyListContact::ContextMenu::OnMoveTo), group));
    if (purple_contact_get_group(parent.GetPurpleContact())
        == group)
      button->GrabFocus();
  }

  AppendSubMenu(_("Move to..."), *groups);
}

void BuddyListContact::ContextMenu::ChangeAliasResponseHandler(
    CppConsUI::InputDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_OK:
      break;
    default:
      return;
  }

  PurpleContact *contact = parent->GetPurpleContact();
  if (contact->alias)
    purple_blist_alias_contact(contact, activator.GetText());
  else {
    PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact);
    purple_blist_alias_buddy(buddy, activator.GetText());
    serv_alias_buddy(buddy);
  }

  // close context menu
  Close();
}

void BuddyListContact::ContextMenu::OnChangeAlias(Button& activator)
{
  PurpleContact *contact = parent->GetPurpleContact();
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(
      _("Alias"), purple_contact_get_alias(contact));
  dialog->signal_response.connect(sigc::mem_fun(this,
        &BuddyListContact::ContextMenu::ChangeAliasResponseHandler));
  dialog->Show();
}

void BuddyListContact::ContextMenu::RemoveResponseHandler(
    CppConsUI::MessageDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_OK:
      break;
    default:
      return;
  }

  // based on gtkdialogs.c:pidgin_dialogs_remove_contact_cb()
  PurpleContact *contact = parent->GetPurpleContact();
  PurpleBlistNode *cnode = reinterpret_cast<PurpleBlistNode*>(contact);
  PurpleGroup *group = reinterpret_cast<PurpleGroup*>(
      purple_blist_node_get_parent(cnode));

  for (PurpleBlistNode *bnode = purple_blist_node_get_first_child(cnode);
      bnode; bnode = purple_blist_node_get_sibling_next(bnode)) {
    PurpleBuddy *buddy = reinterpret_cast<PurpleBuddy*>(bnode);
    PurpleAccount *account = purple_buddy_get_account(buddy);
    if (purple_account_is_connected(account))
      purple_account_remove_buddy(account, buddy, group);
  }

  /* Close the context menu before the contact is deleted because its deletion
   * can lead to destruction of this object. */
  Close();

  purple_blist_remove_contact(contact);
}

void BuddyListContact::ContextMenu::OnRemove(Button& activator)
{
  PurpleContact *contact = parent->GetPurpleContact();
  char *msg = g_strdup_printf(
      _("Are you sure you want to delete contact %s from the list?"),
      purple_buddy_get_alias(purple_contact_get_priority_buddy(contact)));
  CppConsUI::MessageDialog *dialog = new CppConsUI::MessageDialog(
      _("Contact deletion"), msg);
  g_free(msg);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &BuddyListContact::ContextMenu::RemoveResponseHandler));
  dialog->Show();
}

void BuddyListContact::ContextMenu::OnMoveTo(Button& activator,
    PurpleGroup *group)
{
  purple_blist_add_contact(parent->GetPurpleContact(), group, NULL);
}

int BuddyListContact::GetColorPair(const char *widget, const char *property)
  const
{
  if (BUDDYLIST->GetColorizationMode() != BuddyList::COLOR_BY_ACCOUNT
      || strcmp(property, "normal"))
    return Button::GetColorPair(widget, property);

  PurpleAccount *account =
    purple_buddy_get_account(purple_contact_get_priority_buddy(contact));
  int fg = purple_account_get_ui_int(account, "centerim5",
      "buddylist-foreground-color", CppConsUI::Curses::Color::DEFAULT);
  int bg = purple_account_get_ui_int(account, "centerim5",
      "buddylist-background-color", CppConsUI::Curses::Color::DEFAULT);

  CppConsUI::ColorScheme::Color c(fg,bg);
  return COLORSCHEME->GetColorPair(c);
}

void BuddyListContact::OpenContextMenu()
{
  ContextMenu *w = new ContextMenu(*this);
  w->Show();
}

BuddyListContact::BuddyListContact(PurpleBlistNode *node)
: BuddyListNode(node)
{
  SetColorScheme("buddylistcontact");

  contact = reinterpret_cast<PurpleContact*>(node);
}

void BuddyListContact::UpdateColorScheme()
{
  char *new_scheme;
  PurpleBuddy *buddy;

  switch (BUDDYLIST->GetColorizationMode()) {
    case BuddyList::COLOR_BY_STATUS:
      buddy = purple_contact_get_priority_buddy(contact);
      new_scheme = Utils::GetColorSchemeString("buddylistcontact", buddy);
      SetColorScheme(new_scheme);
      g_free(new_scheme);
      break;
    default:
      // note: COLOR_BY_ACCOUNT case is handled by BuddyListContact::Draw()
      SetColorScheme("buddylistcontact");
      break;
  }
}

bool BuddyListGroup::LessThan(const BuddyListNode& other) const
{
  const BuddyListGroup *o = dynamic_cast<const BuddyListGroup*>(&other);
  if (o) {
    return g_utf8_collate(purple_group_get_name(group),
        purple_group_get_name(o->group)) < 0;
  }
  return false;
}

void BuddyListGroup::Update()
{
  BuddyListNode::Update();

  SetText(purple_group_get_name(group));

  SortIn();

  bool vis = true;
  if (!BUDDYLIST->GetShowEmptyGroupsPref())
    vis = purple_blist_get_group_size(group, FALSE);

  SetVisibility(vis);
}

void BuddyListGroup::OnActivate(Button& activator)
{
  treeview->ToggleCollapsed(ref);
  purple_blist_node_set_bool(node, "collapsed", ref->GetCollapsed());
}

const char *BuddyListGroup::ToString() const
{
  return purple_group_get_name(group);
}

BuddyListGroup::ContextMenu::ContextMenu(BuddyListGroup& parent)
: MenuWindow(parent, AUTOSIZE, AUTOSIZE)
, parent(&parent)
{
  AppendItem(_("Rename..."), sigc::mem_fun(this,
        &BuddyListGroup::ContextMenu::OnRename));
  AppendItem(_("Delete..."), sigc::mem_fun(this,
        &BuddyListGroup::ContextMenu::OnRemove));
}

void BuddyListGroup::ContextMenu::RenameResponseHandler(
    CppConsUI::InputDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_OK:
      break;
    default:
      return;
  }

  const char *name = activator.GetText();
  PurpleGroup *group = parent->GetPurpleGroup();
  PurpleGroup *other = purple_find_group(name);
  if (other && g_strcasecmp(name, purple_group_get_name(group))) {
    LOG->Message(_("Specified group is already in the list."));
    // TODO add group merging
  }
  else
    purple_blist_rename_group(group, name);

  // close context menu
  Close();
}

void BuddyListGroup::ContextMenu::OnRename(Button& activator)
{
  PurpleGroup *group = parent->GetPurpleGroup();
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(
      _("Rename"), purple_group_get_name(group));
  dialog->signal_response.connect(sigc::mem_fun(this,
        &BuddyListGroup::ContextMenu::RenameResponseHandler));
  dialog->Show();
}

void BuddyListGroup::ContextMenu::RemoveResponseHandler(
    CppConsUI::MessageDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_OK:
      break;
    default:
      return;
  }

  // based on gtkdialogs.c:pidgin_dialogs_remove_group_cb()
  PurpleGroup *group = parent->GetPurpleGroup();
  PurpleBlistNode *cnode = reinterpret_cast<PurpleBlistNode*>(group)->child;
  while (cnode) {
    if (PURPLE_BLIST_NODE_IS_CONTACT(cnode)) {
      PurpleBlistNode *bnode = cnode->child;
      cnode = cnode->next;
      while (bnode)
        if (PURPLE_BLIST_NODE_IS_BUDDY(bnode)) {
          PurpleBuddy *buddy = reinterpret_cast<PurpleBuddy*>(bnode);
          bnode = bnode->next;
          if (purple_account_is_connected(buddy->account))
            purple_account_remove_buddy(buddy->account, buddy, group);
          purple_blist_remove_buddy(buddy);
        }
        else
          bnode = bnode->next;
    }
    else if (PURPLE_BLIST_NODE_IS_CHAT(cnode)) {
      PurpleChat *chat = reinterpret_cast<PurpleChat*>(cnode);
      cnode = cnode->next;
      purple_blist_remove_chat(chat);
    }
    else
      cnode = cnode->next;
  }

  /* Close the context menu before the group is deleted because its deletion
   * can lead to destruction of this object. */
  Close();

  purple_blist_remove_group(group);
}

void BuddyListGroup::ContextMenu::OnRemove(Button& activator)
{
  PurpleGroup *group = parent->GetPurpleGroup();
  char *msg = g_strdup_printf(
      _("Are you sure you want to delete group %s from the list?"),
      purple_group_get_name(group));
  CppConsUI::MessageDialog *dialog
    = new CppConsUI::MessageDialog(_("Group deletion"), msg);
  g_free(msg);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &BuddyListGroup::ContextMenu::RemoveResponseHandler));
  dialog->Show();
}

void BuddyListGroup::OpenContextMenu()
{
  ContextMenu *w = new ContextMenu(*this);
  w->Show();
}

void BuddyListGroup::DelayedInit()
{
  /* This can't be done when the node is created because node settings are
   * unavailable at that time. */
  if (!purple_blist_node_get_bool(node, "collapsed"))
    treeview->SetCollapsed(ref, false);
}

BuddyListGroup::BuddyListGroup(PurpleBlistNode *node)
: BuddyListNode(node)
{
  SetColorScheme("buddylistgroup");

  group = reinterpret_cast<PurpleGroup*>(node);

  COREMANAGER->TimeoutOnceConnect(sigc::mem_fun(this,
        &BuddyListGroup::DelayedInit), 0);
}

/* vim: set tabstop=2 shiftwidth=2 tw=78 expandtab : */
