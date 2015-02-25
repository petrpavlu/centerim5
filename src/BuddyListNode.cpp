/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 by CenterIM developers
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
#include "Log.h"
#include "Utils.h"

#include <cppconsui/ColorScheme.h>
#include "gettext.h"

BuddyListNode *BuddyListNode::createNode(PurpleBlistNode *node)
{
  PurpleBlistNodeType type = purple_blist_node_get_type(node);
  if (type == PURPLE_BLIST_BUDDY_NODE)
    return new BuddyListBuddy(node);
  if (type == PURPLE_BLIST_CHAT_NODE)
    return new BuddyListChat(node);
  if (type == PURPLE_BLIST_CONTACT_NODE)
    return new BuddyListContact(node);
  if (type == PURPLE_BLIST_GROUP_NODE)
    return new BuddyListGroup(node);

  LOG->error(_("Unhandled buddy list node '%d'."), type);
  return NULL;
}

void BuddyListNode::setParent(CppConsUI::Container &parent)
{
  Button::setParent(parent);

  treeview = dynamic_cast<CppConsUI::TreeView *>(&parent);
  g_assert(treeview);
}

void BuddyListNode::setRefNode(CppConsUI::TreeView::NodeReference n)
{
  ref = n;
  treeview->setCollapsed(ref, true);
}

void BuddyListNode::update()
{
  // cache the last_activity time
  last_activity = purple_blist_node_get_int(blist_node, "last_activity");

  BuddyListNode *parent_node = getParentNode();
  // the parent could have changed, so re-parent the node
  if (parent_node)
    treeview->setNodeParent(ref, parent_node->getRefNode());
}

void BuddyListNode::sortIn()
{
  CppConsUI::TreeView::NodeReference parent_ref;
  if (purple_blist_node_get_parent(blist_node)) {
    /* This blist node has got a logical (libpurple) parent, check if it is
     * possible to find also a cim node. */
    BuddyListNode *parent_node = getParentNode();
    if (parent_node)
      parent_ref = parent_node->getRefNode();
    else {
      // there shouldn't be a cim node only if the flat mode is active
      g_assert(BUDDYLIST->getListMode() == BuddyList::LIST_FLAT);

      parent_ref = treeview->getRootNode();
    }
  }
  else {
    if (PURPLE_BLIST_NODE_IS_GROUP(blist_node)) {
      // groups don't have parent nodes
      parent_ref = treeview->getRootNode();
    }
    else {
      /* When the new_node() callback is called for a contact/chat/buddy (and
       * this method is called as a part of that callback) then the node
       * doesn't have any parent set yet. In such a case, simply return. */
      return;
    }
  }

  /* Do the insertion sort. It should be fast enough here because nodes are
   * usually already sorted and only one node is in a wrong position, so it
   * kind of runs in O(n). */
  CppConsUI::TreeView::SiblingIterator i = parent_ref.end();
  i--;
  while (true) {
    // sref is a node that we want to sort in
    CppConsUI::TreeView::SiblingIterator sref = i;

    // calculate a stop condition
    bool stop_flag;
    if (i != parent_ref.begin()) {
      stop_flag = false;
      i--;
    }
    else
      stop_flag = true;

    BuddyListNode *swidget = dynamic_cast<BuddyListNode *>(sref->getWidget());
    g_assert(swidget);
    CppConsUI::TreeView::SiblingIterator j = sref;
    j++;
    while (j != parent_ref.end()) {
      BuddyListNode *n = dynamic_cast<BuddyListNode *>(j->getWidget());
      g_assert(n);

      if (swidget->lessOrEqual(*n)) {
        treeview->moveNodeBefore(sref, j);
        break;
      }
      j++;
    }
    // the node is last in the list
    if (j == parent_ref.end())
      treeview->moveNodeAfter(sref, --j);

    if (stop_flag)
      break;
  }
}

BuddyListNode *BuddyListNode::getParentNode() const
{
  PurpleBlistNode *parent = purple_blist_node_get_parent(blist_node);
  if (!parent)
    return NULL;

  return reinterpret_cast<BuddyListNode *>(
      purple_blist_node_get_ui_data(parent));
}

BuddyListNode::ContextMenu::ContextMenu(BuddyListNode &parent_node_)
: MenuWindow(parent_node_, AUTOSIZE, AUTOSIZE), parent_node(&parent_node_)
{
}

void BuddyListNode::ContextMenu::onMenuAction(Button & /*activator*/,
    PurpleCallback callback, void *data)
{
  g_assert(callback);

  typedef void (*TypedCallback)(void *, void *);
  TypedCallback real_callback = reinterpret_cast<TypedCallback>(callback);
  real_callback(parent_node->getPurpleBlistNode(), data);

  close();
}

void BuddyListNode::ContextMenu::appendMenuAction(MenuWindow &menu,
    PurpleMenuAction *act)
{
  if (!act) {
    menu.appendSeparator();
    return;
  }

  if (!act->children) {
    if (act->callback)
      menu.appendItem(act->label, sigc::bind(sigc::mem_fun(this,
            &ContextMenu::onMenuAction), act->callback, act->data));
    else {
      // TODO display non-focusable widget?
    }
  }
  else {
    MenuWindow *submenu = new MenuWindow(0, 0, AUTOSIZE, AUTOSIZE);
    menu.appendSubMenu(act->label, *submenu);

    for (GList *l = act->children; l; l = l->next) {
      PurpleMenuAction *act = reinterpret_cast<PurpleMenuAction *>(l->data);
      appendMenuAction(*submenu, act);
    }

    // free memory associated with the children
    g_list_free(act->children);
    act->children = NULL;
  }

  // free the menu action
  purple_menu_action_free(act);
}

void BuddyListNode::ContextMenu::appendProtocolMenu(PurpleConnection *gc)
{
  PurplePluginProtocolInfo *prpl_info
    = PURPLE_PLUGIN_PROTOCOL_INFO(purple_connection_get_prpl(gc));
  if (!prpl_info || !prpl_info->blist_node_menu)
    return;

  GList *ll = prpl_info->blist_node_menu(parent_node->getPurpleBlistNode());
  for (GList *l = ll; l; l = l->next) {
    PurpleMenuAction *act = reinterpret_cast<PurpleMenuAction *>(l->data);
    appendMenuAction(*this, act);
  }

  if (ll) {
    // append a separator because there has been some items
    appendSeparator();
  }

  g_list_free(ll);
}

void BuddyListNode::ContextMenu::appendExtendedMenu()
{
  GList *ll = purple_blist_node_get_extended_menu(
      parent_node->getPurpleBlistNode());
  for (GList *l = ll; l; l = l->next) {
    PurpleMenuAction *act = reinterpret_cast<PurpleMenuAction *>(l->data);
    appendMenuAction(*this, act);
  }

  if (ll) {
    // append a separator because there has been some items
    appendSeparator();
  }

  g_list_free(ll);
}

BuddyListNode::BuddyListNode(PurpleBlistNode *node_)
: treeview(NULL), blist_node(node_), last_activity(0)
{
  purple_blist_node_set_ui_data(blist_node, this);
  signal_activate.connect(sigc::mem_fun(this, &BuddyListNode::onActivate));
  declareBindables();
}

BuddyListNode::~BuddyListNode()
{
  purple_blist_node_set_ui_data(blist_node, NULL);
}

bool BuddyListNode::lessOrEqualByType(const BuddyListNode &other) const
{
  // group < contact < buddy < chat < other
  PurpleBlistNodeType t1 = purple_blist_node_get_type(blist_node);
  PurpleBlistNodeType t2 = purple_blist_node_get_type(other.blist_node);
  return t1 <= t2;
}

bool BuddyListNode::lessOrEqualByBuddySort(PurpleBuddy *left,
    PurpleBuddy *right) const
{
  BuddyList::BuddySortMode mode = BUDDYLIST->getBuddySortMode();
  int a, b;

  switch (mode) {
    case BuddyList::BUDDY_SORT_BY_NAME:
      break;
    case BuddyList::BUDDY_SORT_BY_STATUS:
      a = getBuddyStatusWeight(left);
      b = getBuddyStatusWeight(right);
      if (a != b)
        return a > b;
      break;
    case BuddyList::BUDDY_SORT_BY_ACTIVITY:
      {
        BuddyListNode *bnode_left = reinterpret_cast<BuddyListNode *>(
            purple_blist_node_get_ui_data(PURPLE_BLIST_NODE(left)));
        BuddyListNode *bnode_right = reinterpret_cast<BuddyListNode *>(
            purple_blist_node_get_ui_data(PURPLE_BLIST_NODE(right)));
        a = bnode_left->last_activity;
        b = bnode_right->last_activity;
        if (a != b)
          return a > b;
      }
      break;
  }
  return g_utf8_collate(purple_buddy_get_alias(left),
      purple_buddy_get_alias(right)) <= 0;
}

const char *BuddyListNode::getBuddyStatus(PurpleBuddy *buddy) const
{
  if (!purple_account_is_connected(purple_buddy_get_account(buddy)))
    return "";

  PurplePresence *presence = purple_buddy_get_presence(buddy);
  PurpleStatus *status = purple_presence_get_active_status(presence);
  return Utils::getStatusIndicator(status);
}

int BuddyListNode::getBuddyStatusWeight(PurpleBuddy *buddy) const
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
    case PURPLE_STATUS_MOOD:
      return 7;
    case PURPLE_STATUS_TUNE:
      return 8;
    case PURPLE_STATUS_INVISIBLE:
      return 9;
    case PURPLE_STATUS_AVAILABLE:
      return 10;
  }
}

void BuddyListNode::updateFilterVisibility(const char *name)
{
  if (!isVisible())
    return;

  const char *filter = BUDDYLIST->getFilterString();
  if (!filter[0])
    return;

  // filtering is active
  setVisibility(purple_strcasestr(name, filter));
}

void BuddyListNode::retrieveUserInfoForName(PurpleConnection *gc,
    const char *name) const
{
  PurpleNotifyUserInfo *info = purple_notify_user_info_new();
  purple_notify_user_info_add_pair(info, _("Information"),
      _("Retrieving..."));
  purple_notify_userinfo(gc, name, info, NULL, NULL);
  purple_notify_user_info_destroy(info);
  serv_get_info(gc, name);
}

void BuddyListNode::actionOpenContextMenu()
{
  openContextMenu();
}

void BuddyListNode::declareBindables()
{
  declareBindable("buddylist", "contextmenu", sigc::mem_fun(this,
        &BuddyListNode::actionOpenContextMenu),
      InputProcessor::BINDABLE_NORMAL);
}

bool BuddyListBuddy::lessOrEqual(const BuddyListNode &other) const
{
  const BuddyListBuddy *o = dynamic_cast<const BuddyListBuddy *>(&other);
  if (o)
    return lessOrEqualByBuddySort(buddy, o->buddy);
  return lessOrEqualByType(other);
}

void BuddyListBuddy::update()
{
  BuddyListNode::update();

  const char *status = getBuddyStatus(buddy);
  const char *alias = purple_buddy_get_alias(buddy);
  if (status[0]) {
    char *text = g_strdup_printf("%s %s", status, alias);
    setText(text);
    g_free(text);
  }
  else
    setText(alias);

  sortIn();

  updateColorScheme();

  if (!purple_account_is_connected(purple_buddy_get_account(buddy))) {
    // hide if account is offline
    setVisibility(false);
  }
  else
    setVisibility(BUDDYLIST->getShowOfflineBuddiesPref() || status[0]);

  updateFilterVisibility(alias);
}

void BuddyListBuddy::onActivate(Button & /*activator*/)
{
  PurpleAccount *account = purple_buddy_get_account(buddy);
  const char *name = purple_buddy_get_name(buddy);
  PurpleConversation *conv = purple_find_conversation_with_account(
      PURPLE_CONV_TYPE_IM, name, account);

  if (!conv)
    conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, account, name);
  purple_conversation_present(conv);
}

const char *BuddyListBuddy::toString() const
{
  return purple_buddy_get_alias(buddy);
}

void BuddyListBuddy::retrieveUserInfo()
{
  PurpleConnection *gc = purple_account_get_connection(
      purple_buddy_get_account(buddy));
  retrieveUserInfoForName(gc, purple_buddy_get_name(buddy));
}

BuddyListBuddy::BuddyContextMenu::BuddyContextMenu(
    BuddyListBuddy &parent_buddy_)
: ContextMenu(parent_buddy_), parent_buddy(&parent_buddy_)
{
  appendProtocolMenu(purple_account_get_connection(
          purple_buddy_get_account(parent_buddy->getPurpleBuddy())));
  appendExtendedMenu();

  appendItem(_("Information..."), sigc::mem_fun(this,
        &BuddyContextMenu::onInformation));
  appendItem(_("Alias..."), sigc::mem_fun(this,
        &BuddyContextMenu::onChangeAlias));
  appendItem(_("Delete..."), sigc::mem_fun(this,
        &BuddyContextMenu::onRemove));
}

void BuddyListBuddy::BuddyContextMenu::onInformation(Button & /*activator*/)
{
  parent_buddy->retrieveUserInfo();
  close();
}

void BuddyListBuddy::BuddyContextMenu::changeAliasResponseHandler(
    CppConsUI::InputDialog &activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  PurpleBuddy *buddy = parent_buddy->getPurpleBuddy();
  purple_blist_alias_buddy(buddy, activator.getText());
  serv_alias_buddy(buddy);

  // close context menu
  close();
}

void BuddyListBuddy::BuddyContextMenu::onChangeAlias(Button & /*activator*/)
{
  PurpleBuddy *buddy = parent_buddy->getPurpleBuddy();
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(
      _("Alias"), purple_buddy_get_alias(buddy));
  dialog->signal_response.connect(sigc::mem_fun(this,
        &BuddyContextMenu::changeAliasResponseHandler));
  dialog->show();
}

void BuddyListBuddy::BuddyContextMenu::removeResponseHandler(
    CppConsUI::MessageDialog & /*activator*/,
    CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  PurpleBuddy *buddy = parent_buddy->getPurpleBuddy();
  purple_account_remove_buddy(purple_buddy_get_account(buddy), buddy,
      purple_buddy_get_group(buddy));

  /* Close the context menu before the buddy is deleted because its deletion
   * can lead to destruction of this object. */
  close();

  purple_blist_remove_buddy(buddy);
}

void BuddyListBuddy::BuddyContextMenu::onRemove(Button & /*activator*/)
{
  PurpleBuddy *buddy = parent_buddy->getPurpleBuddy();
  char *msg = g_strdup_printf(
      _("Are you sure you want to delete buddy %s from the list?"),
      purple_buddy_get_alias(buddy));
  CppConsUI::MessageDialog *dialog = new CppConsUI::MessageDialog(
      _("Buddy deletion"), msg);
  g_free(msg);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &BuddyContextMenu::removeResponseHandler));
  dialog->show();
}

int BuddyListBuddy::getColorPair(const char *widget, const char *property)
  const
{
  if (BUDDYLIST->getColorizationMode() != BuddyList::COLOR_BY_ACCOUNT ||
      strcmp(property, "normal"))
    return Button::getColorPair(widget, property);

  PurpleAccount *account = purple_buddy_get_account(buddy);
  int fg = purple_account_get_ui_int(account, "centerim5",
      "buddylist-foreground-color", CppConsUI::Curses::Color::DEFAULT);
  int bg = purple_account_get_ui_int(account, "centerim5",
      "buddylist-background-color", CppConsUI::Curses::Color::DEFAULT);

  CppConsUI::ColorScheme::Color c(fg, bg);
  return COLORSCHEME->getColorPair(c);
}

void BuddyListBuddy::openContextMenu()
{
  ContextMenu *w = new BuddyContextMenu(*this);
  w->show();
}

BuddyListBuddy::BuddyListBuddy(PurpleBlistNode *node_)
: BuddyListNode(node_)
{
  setColorScheme("buddylistbuddy");

  buddy = PURPLE_BUDDY(blist_node);
}

void BuddyListBuddy::updateColorScheme()
{
  char *new_scheme;

  switch (BUDDYLIST->getColorizationMode()) {
    case BuddyList::COLOR_BY_STATUS:
      new_scheme = Utils::getColorSchemeString("buddylistbuddy", buddy);
      setColorScheme(new_scheme);
      g_free(new_scheme);
      break;
    default:
      // note: COLOR_BY_ACCOUNT case is handled by BuddyListBuddy::draw()
      setColorScheme("buddylistbuddy");
      break;
  }
}

bool BuddyListChat::lessOrEqual(const BuddyListNode &other) const
{
  const BuddyListChat *o = dynamic_cast<const BuddyListChat *>(&other);
  if (o)
    return g_utf8_collate(purple_chat_get_name(chat),
        purple_chat_get_name(o->chat)) <= 0;
  return lessOrEqualByType(other);
}

void BuddyListChat::update()
{
  BuddyListNode::update();

  const char *name = purple_chat_get_name(chat);
  setText(name);

  sortIn();

  // hide if account is offline
  setVisibility(purple_account_is_connected(purple_chat_get_account(chat)));

  updateFilterVisibility(name);
}

void BuddyListChat::onActivate(Button & /*activator*/)
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

const char *BuddyListChat::toString() const
{
  return purple_chat_get_name(chat);
}

BuddyListChat::ChatContextMenu::ChatContextMenu(BuddyListChat &parent_chat_)
: ContextMenu(parent_chat_), parent_chat(&parent_chat_)
{
  appendProtocolMenu(purple_account_get_connection(
          purple_chat_get_account(parent_chat->getPurpleChat())));
  appendExtendedMenu();

  appendItem(_("Alias..."), sigc::mem_fun(this,
        &ChatContextMenu::onChangeAlias));
  appendItem(_("Delete..."), sigc::mem_fun(this,
        &ChatContextMenu::onRemove));
}

void BuddyListChat::ChatContextMenu::changeAliasResponseHandler(
    CppConsUI::InputDialog &activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  PurpleChat *chat = parent_chat->getPurpleChat();
  purple_blist_alias_chat(chat, activator.getText());

  // close context menu
  close();
}

void BuddyListChat::ChatContextMenu::onChangeAlias(Button & /*activator*/)
{
  PurpleChat *chat = parent_chat->getPurpleChat();
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(
      _("Alias"), purple_chat_get_name(chat));
  dialog->signal_response.connect(sigc::mem_fun(this,
        &ChatContextMenu::changeAliasResponseHandler));
  dialog->show();
}

void BuddyListChat::ChatContextMenu::removeResponseHandler(
    CppConsUI::MessageDialog & /*activator*/,
    CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  PurpleChat *chat = parent_chat->getPurpleChat();

  /* Close the context menu before the chat is deleted because its deletion
   * can lead to destruction of this object. */
  close();

  purple_blist_remove_chat(chat);
}

void BuddyListChat::ChatContextMenu::onRemove(Button & /*activator*/)
{
  PurpleChat *chat = parent_chat->getPurpleChat();
  char *msg = g_strdup_printf(
      _("Are you sure you want to delete chat %s from the list?"),
      purple_chat_get_name(chat));
  CppConsUI::MessageDialog *dialog = new CppConsUI::MessageDialog(
      _("Chat deletion"), msg);
  g_free(msg);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &ChatContextMenu::removeResponseHandler));
  dialog->show();
}

void BuddyListChat::openContextMenu()
{
  ContextMenu *w = new ChatContextMenu(*this);
  w->show();
}

BuddyListChat::BuddyListChat(PurpleBlistNode *node_)
: BuddyListNode(node_)
{
  setColorScheme("buddylistchat");

  chat = PURPLE_CHAT(blist_node);
}

bool BuddyListContact::lessOrEqual(const BuddyListNode &other) const
{
  const BuddyListContact *o = dynamic_cast<const BuddyListContact *>(&other);
  if (o) {
    PurpleBuddy *left = purple_contact_get_priority_buddy(contact);
    PurpleBuddy *right = purple_contact_get_priority_buddy(o->contact);
    return lessOrEqualByBuddySort(left, right);
  }
  return lessOrEqualByType(other);
}

void BuddyListContact::update()
{
  BuddyListNode::update();

  PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact);
  if (!buddy) {
    /* The contact does not have any associated buddy, ignore it until it gets
     * a buddy assigned. */
    setText("*Contact*");
    setVisibility(false);
    return;
  }

  // format contact size
  char *size;
  if (contact->currentsize > 1)
    size = g_strdup_printf(" (%d)", contact->currentsize);
  else
    size = NULL;

  // format contact label
  const char *alias = purple_contact_get_alias(contact);
  const char *status = getBuddyStatus(buddy);
  char *text;
  if (status[0])
    text = g_strdup_printf("%s %s%s", status, alias, size ? size : "");
  else
    text = g_strdup_printf("%s%s", alias, size ? size : "");
  setText(text);
  g_free(size);
  g_free(text);

  sortIn();

  updateColorScheme();

  if (!purple_account_is_connected(purple_buddy_get_account(buddy))) {
    // hide if account is offline
    setVisibility(false);
  }
  else
    setVisibility(BUDDYLIST->getShowOfflineBuddiesPref() || status[0]);

  updateFilterVisibility(alias);
}

void BuddyListContact::onActivate(Button &activator)
{
  PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact);
  BuddyListNode *bnode = reinterpret_cast<BuddyListNode *>(
      purple_blist_node_get_ui_data(PURPLE_BLIST_NODE(buddy)));
  if (bnode)
    bnode->onActivate(activator);
}

const char *BuddyListContact::toString() const
{
  return purple_contact_get_alias(contact);
}

void BuddyListContact::setRefNode(CppConsUI::TreeView::NodeReference n)
{
  BuddyListNode::setRefNode(n);
  treeview->setNodeStyle(n, CppConsUI::TreeView::STYLE_VOID);
}

void BuddyListContact::retrieveUserInfo()
{
  PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact);
  PurpleConnection *gc = purple_account_get_connection(
      purple_buddy_get_account(buddy));
  retrieveUserInfoForName(gc, purple_buddy_get_name(buddy));
}

BuddyListContact::ContactContextMenu::ContactContextMenu(
    BuddyListContact &parent_contact_)
: ContextMenu(parent_contact_), parent_contact(&parent_contact_)
{
  appendExtendedMenu();

  if (parent_contact->isCollapsed())
    appendItem(_("Expand"), sigc::bind(sigc::mem_fun(this,
            &ContactContextMenu::onExpandRequest), true));
  else
    appendItem(_("Collapse"), sigc::bind(sigc::mem_fun(this,
            &ContactContextMenu::onExpandRequest), false));

  appendItem(_("Information..."), sigc::mem_fun(this,
        &ContactContextMenu::onInformation));
  appendItem(_("Alias..."), sigc::mem_fun(this,
        &ContactContextMenu::onChangeAlias));
  appendItem(_("Delete..."), sigc::mem_fun(this,
        &ContactContextMenu::onRemove));

  CppConsUI::MenuWindow *groups = new CppConsUI::MenuWindow(*this, AUTOSIZE,
      AUTOSIZE);

  for (PurpleBlistNode *node = purple_blist_get_root(); node;
      node = purple_blist_node_get_sibling_next(node)) {
    if (!PURPLE_BLIST_NODE_IS_GROUP(node))
      continue;

    PurpleGroup *group = PURPLE_GROUP(node);
    CppConsUI::Button *button = groups->appendItem(
        purple_group_get_name(group), sigc::bind(sigc::mem_fun(this,
            &ContactContextMenu::onMoveTo), group));
    if (purple_contact_get_group(parent_contact->getPurpleContact())
        == group)
      button->grabFocus();
  }

  appendSubMenu(_("Move to..."), *groups);
}

void BuddyListContact::ContactContextMenu::onExpandRequest(
    Button & /*activator*/, bool expand)
{
  parent_contact->setCollapsed(!expand);
  close();
}

void BuddyListContact::ContactContextMenu::onInformation(
    Button & /*activator*/)
{
  parent_contact->retrieveUserInfo();
  close();
}

void BuddyListContact::ContactContextMenu::changeAliasResponseHandler(
    CppConsUI::InputDialog &activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  PurpleContact *contact = parent_contact->getPurpleContact();
  if (contact->alias)
    purple_blist_alias_contact(contact, activator.getText());
  else {
    PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact);
    purple_blist_alias_buddy(buddy, activator.getText());
    serv_alias_buddy(buddy);
  }

  // close context menu
  close();
}

void BuddyListContact::ContactContextMenu::onChangeAlias(
    Button & /*activator*/)
{
  PurpleContact *contact = parent_contact->getPurpleContact();
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(
      _("Alias"), purple_contact_get_alias(contact));
  dialog->signal_response.connect(sigc::mem_fun(this,
        &ContactContextMenu::changeAliasResponseHandler));
  dialog->show();
}

void BuddyListContact::ContactContextMenu::removeResponseHandler(
    CppConsUI::MessageDialog & /*activator*/,
    CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  // based on gtkdialogs.c:pidgin_dialogs_remove_contact_cb()
  PurpleContact *contact = parent_contact->getPurpleContact();
  PurpleBlistNode *cnode = PURPLE_BLIST_NODE(contact);
  PurpleGroup *group = purple_contact_get_group(contact);

  for (PurpleBlistNode *bnode = purple_blist_node_get_first_child(cnode);
      bnode; bnode = purple_blist_node_get_sibling_next(bnode)) {
    PurpleBuddy *buddy = PURPLE_BUDDY(bnode);
    PurpleAccount *account = purple_buddy_get_account(buddy);
    if (purple_account_is_connected(account))
      purple_account_remove_buddy(account, buddy, group);
  }

  /* Close the context menu before the contact is deleted because its deletion
   * can lead to destruction of this object. */
  close();

  purple_blist_remove_contact(contact);
}

void BuddyListContact::ContactContextMenu::onRemove(Button & /*activator*/)
{
  PurpleContact *contact = parent_contact->getPurpleContact();
  char *msg = g_strdup_printf(
      _("Are you sure you want to delete contact %s from the list?"),
      purple_contact_get_alias(contact));
  CppConsUI::MessageDialog *dialog = new CppConsUI::MessageDialog(
      _("Contact deletion"), msg);
  g_free(msg);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &ContactContextMenu::removeResponseHandler));
  dialog->show();
}

void BuddyListContact::ContactContextMenu::onMoveTo(Button & /*activator*/,
    PurpleGroup *group)
{
  PurpleContact *contact = parent_contact->getPurpleContact();
  close();

  purple_blist_add_contact(contact, group, NULL);
}

int BuddyListContact::getColorPair(const char *widget, const char *property)
  const
{
  if (BUDDYLIST->getColorizationMode() != BuddyList::COLOR_BY_ACCOUNT ||
      strcmp(property, "normal"))
    return Button::getColorPair(widget, property);

  PurpleAccount *account =
    purple_buddy_get_account(purple_contact_get_priority_buddy(contact));
  int fg = purple_account_get_ui_int(account, "centerim5",
      "buddylist-foreground-color", CppConsUI::Curses::Color::DEFAULT);
  int bg = purple_account_get_ui_int(account, "centerim5",
      "buddylist-background-color", CppConsUI::Curses::Color::DEFAULT);

  CppConsUI::ColorScheme::Color c(fg,bg);
  return COLORSCHEME->getColorPair(c);
}

void BuddyListContact::openContextMenu()
{
  ContextMenu *w = new ContactContextMenu(*this);
  w->show();
}

BuddyListContact::BuddyListContact(PurpleBlistNode *node_)
: BuddyListNode(node_)
{
  setColorScheme("buddylistcontact");

  contact = PURPLE_CONTACT(blist_node);
}

void BuddyListContact::updateColorScheme()
{
  char *new_scheme;
  PurpleBuddy *buddy;

  switch (BUDDYLIST->getColorizationMode()) {
    case BuddyList::COLOR_BY_STATUS:
      buddy = purple_contact_get_priority_buddy(contact);
      new_scheme = Utils::getColorSchemeString("buddylistcontact", buddy);
      setColorScheme(new_scheme);
      g_free(new_scheme);
      break;
    default:
      // note: COLOR_BY_ACCOUNT case is handled by BuddyListContact::draw()
      setColorScheme("buddylistcontact");
      break;
  }
}

bool BuddyListGroup::lessOrEqual(const BuddyListNode &other) const
{
  /* If the groups aren't sorted but ordered manually then this method isn't
   * used. */

  const BuddyListGroup *o = dynamic_cast<const BuddyListGroup *>(&other);
  if (o)
    return g_utf8_collate(purple_group_get_name(group),
        purple_group_get_name(o->group)) <= 0;
  return lessOrEqualByType(other);
}

void BuddyListGroup::update()
{
  BuddyListNode::update();

  setText(purple_group_get_name(group));

  // sort in the group
  BuddyList::GroupSortMode mode = BUDDYLIST->getGroupSortMode();
  switch (mode) {
    case BuddyList::GROUP_SORT_BY_USER:
      {
        /* Note that the sorting below works even if there was
         * a contact/chat/buddy node that is attached at the root level of the
         * blist treeview. This happens when such a node was just created (the
         * new_node() callback was called) but the node doesn't have any
         * parent yet. */

        PurpleBlistNode *prev = purple_blist_node_get_sibling_prev(
            blist_node);

        if (prev) {
          // it better be a group node
          g_assert(PURPLE_BLIST_NODE_IS_GROUP(prev));

          BuddyListNode *bnode = reinterpret_cast<BuddyListNode *>(
              purple_blist_node_get_ui_data(prev));
          // there has to be ui_data set for all group nodes!
          g_assert(bnode);

          treeview->moveNodeAfter(ref, bnode->getRefNode());
        }
        else {
          // the group is the first one in the list
          CppConsUI::TreeView::NodeReference parent_ref
            = treeview->getRootNode();
          treeview->moveNodeBefore(ref, parent_ref.begin());
        }
      }
      break;
    case BuddyList::GROUP_SORT_BY_NAME:
      sortIn();
      break;
  }

  bool vis = true;
  if (!BUDDYLIST->getShowEmptyGroupsPref())
    vis = purple_blist_get_group_size(group, FALSE);
  setVisibility(vis);
}

void BuddyListGroup::onActivate(Button & /*activator*/)
{
  treeview->toggleCollapsed(ref);
  purple_blist_node_set_bool(blist_node, "collapsed", ref->isCollapsed());
}

const char *BuddyListGroup::toString() const
{
  return purple_group_get_name(group);
}

void BuddyListGroup::setRefNode(CppConsUI::TreeView::NodeReference n)
{
  BuddyListNode::setRefNode(n);
  initCollapsedState();
}

void BuddyListGroup::initCollapsedState()
{
  /* This can't be done when the purple_blist_load() function was called
   * because node settings are unavailable at that time. */
  treeview->setCollapsed(ref, purple_blist_node_get_bool(blist_node,
        "collapsed"));
}

BuddyListGroup::GroupContextMenu::GroupContextMenu(
    BuddyListGroup &parent_group_)
: ContextMenu(parent_group_), parent_group(&parent_group_)
{
  appendExtendedMenu();

  appendItem(_("Rename..."), sigc::mem_fun(this,
        &GroupContextMenu::onRename));
  appendItem(_("Delete..."), sigc::mem_fun(this,
        &GroupContextMenu::onRemove));

  if (BUDDYLIST->getGroupSortMode() == BuddyList::GROUP_SORT_BY_USER) {
    /* If the manual sorting is enabled then show a menu item and a submenu
     * for group moving. */
    CppConsUI::MenuWindow *groups = new CppConsUI::MenuWindow(*this, AUTOSIZE,
        AUTOSIZE);

    groups->appendItem(_("-Top-"), sigc::bind(
          sigc::mem_fun(this, &GroupContextMenu::onMoveAfter),
          static_cast<PurpleGroup *>(NULL)));
    for (PurpleBlistNode *node = purple_blist_get_root(); node;
        node = purple_blist_node_get_sibling_next(node)) {
      if (!PURPLE_BLIST_NODE_IS_GROUP(node))
        continue;

      PurpleGroup *group = PURPLE_GROUP(node);
      groups->appendItem(purple_group_get_name(group), sigc::bind(
            sigc::mem_fun(this, &GroupContextMenu::onMoveAfter), group));
    }

    appendSubMenu(_("Move after..."), *groups);
  }
}

void BuddyListGroup::GroupContextMenu::renameResponseHandler(
    CppConsUI::InputDialog &activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  const char *name = activator.getText();
  PurpleGroup *group = parent_group->getPurpleGroup();
  PurpleGroup *other = purple_find_group(name);
  if (other && !purple_utf8_strcasecmp(name, purple_group_get_name(group))) {
    LOG->message(_("Specified group is already in the list."));
    /* TODO Add group merging. Note that purple_blist_rename_group() can do
     * the merging. */
  }
  else
    purple_blist_rename_group(group, name);

  // close context menu
  close();
}

void BuddyListGroup::GroupContextMenu::onRename(Button & /*activator*/)
{
  PurpleGroup *group = parent_group->getPurpleGroup();
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(
      _("Rename"), purple_group_get_name(group));
  dialog->signal_response.connect(sigc::mem_fun(this,
        &GroupContextMenu::renameResponseHandler));
  dialog->show();
}

void BuddyListGroup::GroupContextMenu::removeResponseHandler(
    CppConsUI::MessageDialog & /*activator*/,
    CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  // based on gtkdialogs.c:pidgin_dialogs_remove_group_cb()
  PurpleGroup *group = parent_group->getPurpleGroup();
  PurpleBlistNode *cnode = purple_blist_node_get_first_child(
      PURPLE_BLIST_NODE(group));
  while (cnode) {
    if (PURPLE_BLIST_NODE_IS_CONTACT(cnode)) {
      PurpleBlistNode *bnode = purple_blist_node_get_first_child(cnode);
      cnode = purple_blist_node_get_sibling_next(cnode);
      while (bnode)
        if (PURPLE_BLIST_NODE_IS_BUDDY(bnode)) {
          PurpleBuddy *buddy = PURPLE_BUDDY(bnode);
          PurpleAccount *account = purple_buddy_get_account(buddy);
          bnode = purple_blist_node_get_sibling_next(bnode);
          if (purple_account_is_connected(account)) {
            purple_account_remove_buddy(account, buddy, group);
            purple_blist_remove_buddy(buddy);
          }
        }
        else
          bnode = purple_blist_node_get_sibling_next(bnode);
    }
    else if (PURPLE_BLIST_NODE_IS_CHAT(cnode)) {
      PurpleChat *chat = PURPLE_CHAT(cnode);
      cnode = purple_blist_node_get_sibling_next(cnode);
      purple_blist_remove_chat(chat);
    }
    else
      cnode = purple_blist_node_get_sibling_next(cnode);
  }

  /* Close the context menu before the group is deleted because its deletion
   * can lead to destruction of this object. */
  close();

  purple_blist_remove_group(group);
}

void BuddyListGroup::GroupContextMenu::onRemove(Button & /*activator*/)
{
  PurpleGroup *group = parent_group->getPurpleGroup();
  char *msg = g_strdup_printf(
      _("Are you sure you want to delete group %s from the list?"),
      purple_group_get_name(group));
  CppConsUI::MessageDialog *dialog
    = new CppConsUI::MessageDialog(_("Group deletion"), msg);
  g_free(msg);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &GroupContextMenu::removeResponseHandler));
  dialog->show();
}

void BuddyListGroup::GroupContextMenu::onMoveAfter(Button & /*activator*/,
    PurpleGroup *group)
{
  PurpleGroup *moved_group = parent_group->getPurpleGroup();
  close();

  purple_blist_add_group(moved_group, PURPLE_BLIST_NODE(group));
}

void BuddyListGroup::openContextMenu()
{
  ContextMenu *w = new GroupContextMenu(*this);
  w->show();
}

BuddyListGroup::BuddyListGroup(PurpleBlistNode *node_)
: BuddyListNode(node_)
{
  setColorScheme("buddylistgroup");

  group = PURPLE_GROUP(blist_node);
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
