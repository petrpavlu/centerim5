// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
// Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
//
// This file is part of CenterIM.
//
// CenterIM is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// CenterIM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

#include "BuddyListNode.h"

#include "BuddyList.h"
#include "Log.h"
#include "Utils.h"

#include "gettext.h"
#include <cppconsui/ColorScheme.h>

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
  return nullptr;
}

void BuddyListNode::setParent(CppConsUI::Container &parent)
{
  Button::setParent(parent);

  treeview_ = dynamic_cast<CppConsUI::TreeView *>(&parent);
  g_assert(treeview_ != nullptr);
}

void BuddyListNode::setRefNode(CppConsUI::TreeView::NodeReference n)
{
  ref_ = n;
  treeview_->setCollapsed(ref_, true);
}

void BuddyListNode::update()
{
  // Cache the last_activity time.
  last_activity_ = purple_blist_node_get_int(blist_node_, "last_activity");

  BuddyListNode *parent_node = getParentNode();
  // The parent could have changed, so re-parent the node.
  if (parent_node != nullptr)
    treeview_->setNodeParent(ref_, parent_node->getRefNode());
}

void BuddyListNode::sortIn()
{
  CppConsUI::TreeView::NodeReference parent_ref;
  if (purple_blist_node_get_parent(blist_node_) != nullptr) {
    // This blist node has got a logical (libpurple) parent, check if it is
    // possible to find also a cim node.
    BuddyListNode *parent_node = getParentNode();
    if (parent_node != nullptr)
      parent_ref = parent_node->getRefNode();
    else {
      // there shouldn't be a cim node only if the flat mode is active
      g_assert(BUDDYLIST->getListMode() == BuddyList::LIST_FLAT);

      parent_ref = treeview_->getRootNode();
    }
  }
  else {
    if (PURPLE_BLIST_NODE_IS_GROUP(blist_node_)) {
      // Groups do not have parent nodes.
      parent_ref = treeview_->getRootNode();
    }
    else {
      // When the new_node() callback is called for a contact/chat/buddy (and
      // this method is called as a part of that callback) then the node does
      // not have any parent set yet. In such a case, simply return.
      return;
    }
  }

  // Do the insertion sort. It should be fast enough here because nodes are
  // usually already sorted and only one node is in a wrong position, so it kind
  // of runs in O(n).
  CppConsUI::TreeView::SiblingIterator i = parent_ref.end();
  --i;
  while (true) {
    // sref is a node that we want to sort in.
    CppConsUI::TreeView::SiblingIterator sref = i;

    // Calculate a stop condition.
    bool stop_flag;
    if (i != parent_ref.begin()) {
      stop_flag = false;
      --i;
    }
    else
      stop_flag = true;

    BuddyListNode *swidget = dynamic_cast<BuddyListNode *>(sref->getWidget());
    g_assert(swidget != nullptr);
    CppConsUI::TreeView::SiblingIterator j = sref;
    ++j;
    while (j != parent_ref.end()) {
      BuddyListNode *n = dynamic_cast<BuddyListNode *>(j->getWidget());
      g_assert(n != nullptr);

      if (swidget->lessOrEqual(*n)) {
        treeview_->moveNodeBefore(sref, j);
        break;
      }
      ++j;
    }
    // the node is last in the list
    if (j == parent_ref.end())
      treeview_->moveNodeAfter(sref, --j);

    if (stop_flag)
      break;
  }
}

BuddyListNode *BuddyListNode::getParentNode() const
{
  PurpleBlistNode *parent = purple_blist_node_get_parent(blist_node_);
  if (parent == nullptr)
    return nullptr;

  return reinterpret_cast<BuddyListNode *>(
    purple_blist_node_get_ui_data(parent));
}

BuddyListNode::ContextMenu::ContextMenu(BuddyListNode &parent_node)
  : MenuWindow(parent_node, AUTOSIZE, AUTOSIZE), parent_node_(&parent_node)
{
}

void BuddyListNode::ContextMenu::onMenuAction(
  Button & /*activator*/, PurpleCallback callback, void *data)
{
  g_assert(callback != nullptr);

  typedef void (*TypedCallback)(void *, void *);
  TypedCallback real_callback = reinterpret_cast<TypedCallback>(callback);
  real_callback(parent_node_->getPurpleBlistNode(), data);

  close();
}

void BuddyListNode::ContextMenu::appendMenuAction(
  MenuWindow &menu, PurpleMenuAction *act)
{
  if (act == nullptr) {
    menu.appendSeparator();
    return;
  }

  if (act->children == nullptr) {
    if (act->callback != nullptr)
      menu.appendItem(
        act->label, sigc::bind(sigc::mem_fun(this, &ContextMenu::onMenuAction),
                      act->callback, act->data));
    else {
      // TODO display non-focusable widget?
    }
  }
  else {
    auto submenu = new MenuWindow(0, 0, AUTOSIZE, AUTOSIZE);
    menu.appendSubMenu(act->label, *submenu);

    for (GList *l = act->children; l != nullptr; l = l->next) {
      PurpleMenuAction *act = reinterpret_cast<PurpleMenuAction *>(l->data);
      appendMenuAction(*submenu, act);
    }

    // Free memory associated with the children.
    g_list_free(act->children);
    act->children = nullptr;
  }

  // Free the menu action.
  purple_menu_action_free(act);
}

void BuddyListNode::ContextMenu::appendProtocolMenu(PurpleConnection *gc)
{
  PurplePluginProtocolInfo *prpl_info =
    PURPLE_PLUGIN_PROTOCOL_INFO(purple_connection_get_prpl(gc));
  if (prpl_info == nullptr || prpl_info->blist_node_menu == nullptr)
    return;

  GList *ll = prpl_info->blist_node_menu(parent_node_->getPurpleBlistNode());
  for (GList *l = ll; l != nullptr; l = l->next) {
    PurpleMenuAction *act = reinterpret_cast<PurpleMenuAction *>(l->data);
    appendMenuAction(*this, act);
  }

  if (ll != nullptr) {
    // Append a separator because some items were added.
    appendSeparator();
  }

  g_list_free(ll);
}

void BuddyListNode::ContextMenu::appendExtendedMenu()
{
  GList *ll =
    purple_blist_node_get_extended_menu(parent_node_->getPurpleBlistNode());
  for (GList *l = ll; l != nullptr; l = l->next) {
    PurpleMenuAction *act = reinterpret_cast<PurpleMenuAction *>(l->data);
    appendMenuAction(*this, act);
  }

  if (ll != nullptr) {
    // Append a separator because some items were added.
    appendSeparator();
  }

  g_list_free(ll);
}

BuddyListNode::BuddyListNode(PurpleBlistNode *node)
  : treeview_(nullptr), blist_node_(node), last_activity_(0)
{
  purple_blist_node_set_ui_data(blist_node_, this);
  signal_activate.connect(sigc::mem_fun(this, &BuddyListNode::onActivate));
  declareBindables();
}

BuddyListNode::~BuddyListNode()
{
  purple_blist_node_set_ui_data(blist_node_, nullptr);
}

bool BuddyListNode::lessOrEqualByType(const BuddyListNode &other) const
{
  // group < contact < buddy < chat < other.
  PurpleBlistNodeType t1 = purple_blist_node_get_type(blist_node_);
  PurpleBlistNodeType t2 = purple_blist_node_get_type(other.blist_node_);
  return t1 <= t2;
}

bool BuddyListNode::lessOrEqualByBuddySort(
  PurpleBuddy *left, PurpleBuddy *right) const
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
  case BuddyList::BUDDY_SORT_BY_ACTIVITY: {
    // Compare buddies according to their last activity.
    //
    // It is possible that a blist node will not have the ui_data set. For
    // instance, this happens when libpurple informs the program that a blist
    // node is about to be removed. At that point, an associated BuddyListNode
    // is destroyed, a parent node is updated and the parent tries to update its
    // position according to its priority buddy. This buddy will not have the
    // ui_data set because the BuddyListNode has been already freed.
    //
    // In such a case, the cached value cannot be obtained and value 0 will be
    // used instead.
    BuddyListNode *bnode_left = reinterpret_cast<BuddyListNode *>(
      purple_blist_node_get_ui_data(PURPLE_BLIST_NODE(left)));
    BuddyListNode *bnode_right = reinterpret_cast<BuddyListNode *>(
      purple_blist_node_get_ui_data(PURPLE_BLIST_NODE(right)));
    a = bnode_left ? bnode_left->last_activity_ : 0;
    b = bnode_right ? bnode_right->last_activity_ : 0;
    if (a != b)
      return a > b;
  } break;
  }
  return g_utf8_collate(
           purple_buddy_get_alias(left), purple_buddy_get_alias(right)) <= 0;
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

int BuddyListNode::getColorSchemeByBuddy(int base_scheme, PurpleBuddy *buddy)
{
  g_assert(base_scheme == CenterIM::SCHEME_BUDDYLISTBUDDY ||
    base_scheme == CenterIM::SCHEME_BUDDYLISTCONTACT);

  bool scheme_buddy = (base_scheme == CenterIM::SCHEME_BUDDYLISTBUDDY);

  if (!purple_account_is_connected(purple_buddy_get_account(buddy)))
    return scheme_buddy ? CenterIM::SCHEME_BUDDYLISTBUDDY_OFFLINE
                        : CenterIM::SCHEME_BUDDYLISTCONTACT_OFFLINE;

  PurplePresence *presence = purple_buddy_get_presence(buddy);
  PurpleStatus *status = purple_presence_get_active_status(presence);
  PurpleStatusType *status_type = purple_status_get_type(status);
  PurpleStatusPrimitive prim = purple_status_type_get_primitive(status_type);

  switch (prim) {
  case PURPLE_STATUS_UNSET:
  case PURPLE_STATUS_OFFLINE:
  default:
    return scheme_buddy ? CenterIM::SCHEME_BUDDYLISTBUDDY_OFFLINE
                        : CenterIM::SCHEME_BUDDYLISTCONTACT_OFFLINE;
  case PURPLE_STATUS_AVAILABLE:
  case PURPLE_STATUS_MOBILE:
  case PURPLE_STATUS_TUNE:
  case PURPLE_STATUS_MOOD:
    return scheme_buddy ? CenterIM::SCHEME_BUDDYLISTBUDDY_ONLINE
                        : CenterIM::SCHEME_BUDDYLISTCONTACT_ONLINE;
  case PURPLE_STATUS_AWAY:
  case PURPLE_STATUS_EXTENDED_AWAY:
    return scheme_buddy ? CenterIM::SCHEME_BUDDYLISTBUDDY_AWAY
                        : CenterIM::SCHEME_BUDDYLISTCONTACT_AWAY;
  case PURPLE_STATUS_UNAVAILABLE:
  case PURPLE_STATUS_INVISIBLE:
    return scheme_buddy ? CenterIM::SCHEME_BUDDYLISTBUDDY_NA
                        : CenterIM::SCHEME_BUDDYLISTCONTACT_NA;
  }
}

void BuddyListNode::updateFilterVisibility(const char *name)
{
  if (!isVisible())
    return;

  const char *filter = BUDDYLIST->getFilterString();
  if (filter[0] == '\0')
    return;

  // Filtering is active.
  setVisibility(purple_strcasestr(name, filter));
}

void BuddyListNode::retrieveUserInfoForName(
  PurpleConnection *gc, const char *name) const
{
  PurpleNotifyUserInfo *info = purple_notify_user_info_new();
  purple_notify_user_info_add_pair(info, _("Information"), _("Retrieving..."));
  purple_notify_userinfo(gc, name, info, nullptr, nullptr);
  purple_notify_user_info_destroy(info);
  serv_get_info(gc, name);
}

void BuddyListNode::actionOpenContextMenu()
{
  openContextMenu();
}

void BuddyListNode::declareBindables()
{
  declareBindable("buddylist", "contextmenu",
    sigc::mem_fun(this, &BuddyListNode::actionOpenContextMenu),
    InputProcessor::BINDABLE_NORMAL);
}

bool BuddyListBuddy::lessOrEqual(const BuddyListNode &other) const
{
  const BuddyListBuddy *o = dynamic_cast<const BuddyListBuddy *>(&other);
  if (o != nullptr)
    return lessOrEqualByBuddySort(buddy_, o->buddy_);
  return lessOrEqualByType(other);
}

void BuddyListBuddy::update()
{
  BuddyListNode::update();

  const char *status = getBuddyStatus(buddy_);
  const char *alias = purple_buddy_get_alias(buddy_);
  if (status[0] != '\0') {
    char *text = g_strdup_printf("%s %s", status, alias);
    setText(text);
    g_free(text);
  }
  else
    setText(alias);

  sortIn();

  updateColorScheme();

  if (!purple_account_is_connected(purple_buddy_get_account(buddy_))) {
    // Hide if account is offline.
    setVisibility(false);
  }
  else
    setVisibility(status[0] != '\0' || BUDDYLIST->getShowOfflineBuddiesPref());

  updateFilterVisibility(alias);
}

void BuddyListBuddy::onActivate(Button & /*activator*/)
{
  PurpleAccount *account = purple_buddy_get_account(buddy_);
  const char *name = purple_buddy_get_name(buddy_);
  PurpleConversation *conv =
    purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, name, account);

  if (conv == nullptr)
    conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, account, name);
  purple_conversation_present(conv);
}

const char *BuddyListBuddy::toString() const
{
  return purple_buddy_get_alias(buddy_);
}

void BuddyListBuddy::retrieveUserInfo()
{
  PurpleConnection *gc =
    purple_account_get_connection(purple_buddy_get_account(buddy_));
  retrieveUserInfoForName(gc, purple_buddy_get_name(buddy_));
}

BuddyListBuddy::BuddyContextMenu::BuddyContextMenu(BuddyListBuddy &parent_buddy)
  : ContextMenu(parent_buddy), parent_buddy_(&parent_buddy)
{
  appendProtocolMenu(purple_account_get_connection(
    purple_buddy_get_account(parent_buddy_->getPurpleBuddy())));
  appendExtendedMenu();

  appendItem(
    _("Information..."), sigc::mem_fun(this, &BuddyContextMenu::onInformation));
  appendItem(
    _("Alias..."), sigc::mem_fun(this, &BuddyContextMenu::onChangeAlias));
  appendItem(_("Delete..."), sigc::mem_fun(this, &BuddyContextMenu::onRemove));
}

void BuddyListBuddy::BuddyContextMenu::onInformation(Button & /*activator*/)
{
  parent_buddy_->retrieveUserInfo();
  close();
}

void BuddyListBuddy::BuddyContextMenu::changeAliasResponseHandler(
  CppConsUI::InputDialog &activator,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  PurpleBuddy *buddy = parent_buddy_->getPurpleBuddy();
  purple_blist_alias_buddy(buddy, activator.getText());
  serv_alias_buddy(buddy);

  // Close context menu.
  close();
}

void BuddyListBuddy::BuddyContextMenu::onChangeAlias(Button & /*activator*/)
{
  PurpleBuddy *buddy = parent_buddy_->getPurpleBuddy();
  auto dialog =
    new CppConsUI::InputDialog(_("Alias"), purple_buddy_get_alias(buddy));
  dialog->signal_response.connect(
    sigc::mem_fun(this, &BuddyContextMenu::changeAliasResponseHandler));
  dialog->show();
}

void BuddyListBuddy::BuddyContextMenu::removeResponseHandler(
  CppConsUI::MessageDialog & /*activator*/,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  PurpleBuddy *buddy = parent_buddy_->getPurpleBuddy();
  purple_account_remove_buddy(
    purple_buddy_get_account(buddy), buddy, purple_buddy_get_group(buddy));

  // Close the context menu before the buddy is deleted because its deletion can
  // lead to destruction of this object.
  close();

  purple_blist_remove_buddy(buddy);
}

void BuddyListBuddy::BuddyContextMenu::onRemove(Button & /*activator*/)
{
  PurpleBuddy *buddy = parent_buddy_->getPurpleBuddy();
  char *msg = g_strdup_printf(
    _("Are you sure you want to delete buddy %s from the list?"),
    purple_buddy_get_alias(buddy));
  auto dialog = new CppConsUI::MessageDialog(_("Buddy deletion"), msg);
  g_free(msg);
  dialog->signal_response.connect(
    sigc::mem_fun(this, &BuddyContextMenu::removeResponseHandler));
  dialog->show();
}

int BuddyListBuddy::getAttributes(
  int property, int subproperty, int *attrs, CppConsUI::Error &error) const
{
  if (BUDDYLIST->getColorizationMode() != BuddyList::COLOR_BY_ACCOUNT ||
    property != CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL)
    return Button::getAttributes(property, subproperty, attrs, error);

  // TODO Implement caching for these two properties.
  PurpleAccount *account = purple_buddy_get_account(buddy_);
  int fg = purple_account_get_ui_int(account, "centerim5",
    "buddylist-foreground-color", CppConsUI::Curses::Color::DEFAULT);
  int bg = purple_account_get_ui_int(account, "centerim5",
    "buddylist-background-color", CppConsUI::Curses::Color::DEFAULT);

  CppConsUI::ColorScheme::Color c(fg, bg);
  return COLORSCHEME->getColorPair(c, attrs, error);
}

void BuddyListBuddy::openContextMenu()
{
  ContextMenu *w = new BuddyContextMenu(*this);
  w->show();
}

BuddyListBuddy::BuddyListBuddy(PurpleBlistNode *node) : BuddyListNode(node)
{
  setColorScheme(CenterIM::SCHEME_BUDDYLISTBUDDY);

  buddy_ = PURPLE_BUDDY(blist_node_);
}

void BuddyListBuddy::updateColorScheme()
{
  switch (BUDDYLIST->getColorizationMode()) {
  case BuddyList::COLOR_BY_STATUS:
    setColorScheme(
      getColorSchemeByBuddy(CenterIM::SCHEME_BUDDYLISTBUDDY, buddy_));
    break;
  default:
    // Note: COLOR_BY_ACCOUNT case is handled by
    // BuddyListBuddy::getAttributes().
    setColorScheme(CenterIM::SCHEME_BUDDYLISTBUDDY);
    break;
  }
}

bool BuddyListChat::lessOrEqual(const BuddyListNode &other) const
{
  const BuddyListChat *o = dynamic_cast<const BuddyListChat *>(&other);
  if (o != nullptr)
    return g_utf8_collate(
             purple_chat_get_name(chat_), purple_chat_get_name(o->chat_)) <= 0;
  return lessOrEqualByType(other);
}

void BuddyListChat::update()
{
  BuddyListNode::update();

  const char *name = purple_chat_get_name(chat_);
  setText(name);

  sortIn();

  // Hide if account is offline.
  setVisibility(purple_account_is_connected(purple_chat_get_account(chat_)));

  updateFilterVisibility(name);
}

void BuddyListChat::onActivate(Button & /*activator*/)
{
  PurpleAccount *account = purple_chat_get_account(chat_);
  PurplePluginProtocolInfo *prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(
    purple_find_prpl(purple_account_get_protocol_id(account)));
  GHashTable *components = purple_chat_get_components(chat_);

  char *chat_name = nullptr;
  if (prpl_info != nullptr && prpl_info->get_chat_name != nullptr)
    chat_name = prpl_info->get_chat_name(components);

  const char *name;
  if (chat_name != nullptr)
    name = chat_name;
  else
    name = purple_chat_get_name(chat_);

  PurpleConversation *conv =
    purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, name, account);
  if (conv != nullptr)
    purple_conversation_present(conv);

  serv_join_chat(purple_account_get_connection(account), components);

  g_free(chat_name);
}

const char *BuddyListChat::toString() const
{
  return purple_chat_get_name(chat_);
}

BuddyListChat::ChatContextMenu::ChatContextMenu(BuddyListChat &parent_chat)
  : ContextMenu(parent_chat), parent_chat_(&parent_chat)
{
  appendProtocolMenu(purple_account_get_connection(
    purple_chat_get_account(parent_chat_->getPurpleChat())));
  appendExtendedMenu();

  appendItem(
    _("Alias..."), sigc::mem_fun(this, &ChatContextMenu::onChangeAlias));
  appendItem(_("Delete..."), sigc::mem_fun(this, &ChatContextMenu::onRemove));
}

void BuddyListChat::ChatContextMenu::changeAliasResponseHandler(
  CppConsUI::InputDialog &activator,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  PurpleChat *chat = parent_chat_->getPurpleChat();
  purple_blist_alias_chat(chat, activator.getText());

  // Close context menu.
  close();
}

void BuddyListChat::ChatContextMenu::onChangeAlias(Button & /*activator*/)
{
  PurpleChat *chat = parent_chat_->getPurpleChat();
  auto dialog =
    new CppConsUI::InputDialog(_("Alias"), purple_chat_get_name(chat));
  dialog->signal_response.connect(
    sigc::mem_fun(this, &ChatContextMenu::changeAliasResponseHandler));
  dialog->show();
}

void BuddyListChat::ChatContextMenu::removeResponseHandler(
  CppConsUI::MessageDialog & /*activator*/,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  PurpleChat *chat = parent_chat_->getPurpleChat();

  // Close the context menu before the chat is deleted because its deletion can
  // lead to destruction of this object.
  close();

  purple_blist_remove_chat(chat);
}

void BuddyListChat::ChatContextMenu::onRemove(Button & /*activator*/)
{
  PurpleChat *chat = parent_chat_->getPurpleChat();
  char *msg =
    g_strdup_printf(_("Are you sure you want to delete chat %s from the list?"),
      purple_chat_get_name(chat));
  auto dialog = new CppConsUI::MessageDialog(_("Chat deletion"), msg);
  g_free(msg);
  dialog->signal_response.connect(
    sigc::mem_fun(this, &ChatContextMenu::removeResponseHandler));
  dialog->show();
}

void BuddyListChat::openContextMenu()
{
  ContextMenu *w = new ChatContextMenu(*this);
  w->show();
}

BuddyListChat::BuddyListChat(PurpleBlistNode *node) : BuddyListNode(node)
{
  setColorScheme(CenterIM::SCHEME_BUDDYLISTCHAT);

  chat_ = PURPLE_CHAT(blist_node_);
}

bool BuddyListContact::lessOrEqual(const BuddyListNode &other) const
{
  const BuddyListContact *o = dynamic_cast<const BuddyListContact *>(&other);
  if (o != nullptr) {
    PurpleBuddy *left = purple_contact_get_priority_buddy(contact_);
    PurpleBuddy *right = purple_contact_get_priority_buddy(o->contact_);
    return lessOrEqualByBuddySort(left, right);
  }
  return lessOrEqualByType(other);
}

void BuddyListContact::update()
{
  BuddyListNode::update();

  PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact_);
  if (buddy == nullptr) {
    // The contact does not have any associated buddy, ignore it until it gets a
    // buddy assigned.
    setText("*Contact*");
    setVisibility(false);
    return;
  }

  // Format contact size.
  char *size;
  if (contact_->currentsize > 1)
    size = g_strdup_printf(" (%d)", contact_->currentsize);
  else
    size = nullptr;

  // Format contact label.
  const char *alias = purple_contact_get_alias(contact_);
  const char *status = getBuddyStatus(buddy);
  char *text;
  if (status[0] != '\0')
    text = g_strdup_printf("%s %s%s", status, alias, size ? size : "");
  else
    text = g_strdup_printf("%s%s", alias, size ? size : "");
  setText(text);
  g_free(size);
  g_free(text);

  sortIn();

  updateColorScheme();

  if (!purple_account_is_connected(purple_buddy_get_account(buddy))) {
    // Hide if account is offline.
    setVisibility(false);
  }
  else
    setVisibility(status[0] != '\0' || BUDDYLIST->getShowOfflineBuddiesPref());

  updateFilterVisibility(alias);
}

void BuddyListContact::onActivate(Button &activator)
{
  PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact_);
  BuddyListNode *bnode = reinterpret_cast<BuddyListNode *>(
    purple_blist_node_get_ui_data(PURPLE_BLIST_NODE(buddy)));
  if (bnode != nullptr)
    bnode->onActivate(activator);
}

const char *BuddyListContact::toString() const
{
  return purple_contact_get_alias(contact_);
}

void BuddyListContact::setRefNode(CppConsUI::TreeView::NodeReference n)
{
  BuddyListNode::setRefNode(n);
  treeview_->setNodeStyle(n, CppConsUI::TreeView::STYLE_VOID);
}

void BuddyListContact::retrieveUserInfo()
{
  PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact_);
  PurpleConnection *gc =
    purple_account_get_connection(purple_buddy_get_account(buddy));
  retrieveUserInfoForName(gc, purple_buddy_get_name(buddy));
}

BuddyListContact::ContactContextMenu::ContactContextMenu(
  BuddyListContact &parent_contact)
  : ContextMenu(parent_contact), parent_contact_(&parent_contact)
{
  appendExtendedMenu();

  if (parent_contact_->isCollapsed())
    appendItem(_("Expand"),
      sigc::bind(sigc::mem_fun(this, &ContactContextMenu::onExpandRequest),
                 true));
  else
    appendItem(_("Collapse"),
      sigc::bind(sigc::mem_fun(this, &ContactContextMenu::onExpandRequest),
                 false));

  appendItem(_("Information..."),
    sigc::mem_fun(this, &ContactContextMenu::onInformation));
  appendItem(
    _("Alias..."), sigc::mem_fun(this, &ContactContextMenu::onChangeAlias));
  appendItem(
    _("Delete..."), sigc::mem_fun(this, &ContactContextMenu::onRemove));

  auto groups = new CppConsUI::MenuWindow(*this, AUTOSIZE, AUTOSIZE);

  for (PurpleBlistNode *node = purple_blist_get_root(); node != nullptr;
       node = purple_blist_node_get_sibling_next(node)) {
    if (!PURPLE_BLIST_NODE_IS_GROUP(node))
      continue;

    PurpleGroup *group = PURPLE_GROUP(node);
    CppConsUI::Button *button = groups->appendItem(purple_group_get_name(group),
      sigc::bind(sigc::mem_fun(this, &ContactContextMenu::onMoveTo), group));
    if (purple_contact_get_group(parent_contact_->getPurpleContact()) == group)
      button->grabFocus();
  }

  appendSubMenu(_("Move to..."), *groups);
}

void BuddyListContact::ContactContextMenu::onExpandRequest(
  Button & /*activator*/, bool expand)
{
  parent_contact_->setCollapsed(!expand);
  close();
}

void BuddyListContact::ContactContextMenu::onInformation(Button & /*activator*/)
{
  parent_contact_->retrieveUserInfo();
  close();
}

void BuddyListContact::ContactContextMenu::changeAliasResponseHandler(
  CppConsUI::InputDialog &activator,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  PurpleContact *contact = parent_contact_->getPurpleContact();
  if (contact->alias != nullptr)
    purple_blist_alias_contact(contact, activator.getText());
  else {
    PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact);
    purple_blist_alias_buddy(buddy, activator.getText());
    serv_alias_buddy(buddy);
  }

  // Close context menu.
  close();
}

void BuddyListContact::ContactContextMenu::onChangeAlias(Button & /*activator*/)
{
  PurpleContact *contact = parent_contact_->getPurpleContact();
  auto dialog =
    new CppConsUI::InputDialog(_("Alias"), purple_contact_get_alias(contact));
  dialog->signal_response.connect(
    sigc::mem_fun(this, &ContactContextMenu::changeAliasResponseHandler));
  dialog->show();
}

void BuddyListContact::ContactContextMenu::removeResponseHandler(
  CppConsUI::MessageDialog & /*activator*/,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  // Based on gtkdialogs.c:pidgin_dialogs_remove_contact_cb().
  PurpleContact *contact = parent_contact_->getPurpleContact();
  PurpleBlistNode *cnode = PURPLE_BLIST_NODE(contact);
  PurpleGroup *group = purple_contact_get_group(contact);

  for (PurpleBlistNode *bnode = purple_blist_node_get_first_child(cnode);
       bnode != nullptr; bnode = purple_blist_node_get_sibling_next(bnode)) {
    PurpleBuddy *buddy = PURPLE_BUDDY(bnode);
    PurpleAccount *account = purple_buddy_get_account(buddy);
    if (purple_account_is_connected(account))
      purple_account_remove_buddy(account, buddy, group);
  }

  // Close the context menu before the contact is deleted because its deletion
  // can lead to destruction of this object.
  close();

  purple_blist_remove_contact(contact);
}

void BuddyListContact::ContactContextMenu::onRemove(Button & /*activator*/)
{
  PurpleContact *contact = parent_contact_->getPurpleContact();
  char *msg = g_strdup_printf(
    _("Are you sure you want to delete contact %s from the list?"),
    purple_contact_get_alias(contact));
  auto dialog = new CppConsUI::MessageDialog(_("Contact deletion"), msg);
  g_free(msg);
  dialog->signal_response.connect(
    sigc::mem_fun(this, &ContactContextMenu::removeResponseHandler));
  dialog->show();
}

void BuddyListContact::ContactContextMenu::onMoveTo(
  Button & /*activator*/, PurpleGroup *group)
{
  PurpleContact *contact = parent_contact_->getPurpleContact();
  close();

  purple_blist_add_contact(contact, group, nullptr);
}

int BuddyListContact::getAttributes(
  int property, int subproperty, int *attrs, CppConsUI::Error &error) const
{
  if (BUDDYLIST->getColorizationMode() != BuddyList::COLOR_BY_ACCOUNT ||
    property != CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL)
    return Button::getAttributes(property, subproperty, attrs, error);

  // TODO Implement caching for these two properties.
  PurpleAccount *account =
    purple_buddy_get_account(purple_contact_get_priority_buddy(contact_));
  int fg = purple_account_get_ui_int(account, "centerim5",
    "buddylist-foreground-color", CppConsUI::Curses::Color::DEFAULT);
  int bg = purple_account_get_ui_int(account, "centerim5",
    "buddylist-background-color", CppConsUI::Curses::Color::DEFAULT);

  CppConsUI::ColorScheme::Color c(fg, bg);
  return COLORSCHEME->getColorPair(c, attrs, error);
}

void BuddyListContact::openContextMenu()
{
  ContextMenu *w = new ContactContextMenu(*this);
  w->show();
}

BuddyListContact::BuddyListContact(PurpleBlistNode *node) : BuddyListNode(node)
{
  setColorScheme(CenterIM::SCHEME_BUDDYLISTCONTACT);

  contact_ = PURPLE_CONTACT(blist_node_);
}

void BuddyListContact::updateColorScheme()
{
  switch (BUDDYLIST->getColorizationMode()) {
  case BuddyList::COLOR_BY_STATUS: {
    PurpleBuddy *buddy = purple_contact_get_priority_buddy(contact_);
    setColorScheme(
      getColorSchemeByBuddy(CenterIM::SCHEME_BUDDYLISTCONTACT, buddy));
    break;
  }
  default:
    // Note: COLOR_BY_ACCOUNT case is handled by
    // BuddyListContact::getAttributes().
    setColorScheme(CenterIM::SCHEME_BUDDYLISTCONTACT);
    break;
  }
}

bool BuddyListGroup::lessOrEqual(const BuddyListNode &other) const
{
  // If the groups are not sorted but ordered manually then this method is not
  // used.

  const BuddyListGroup *o = dynamic_cast<const BuddyListGroup *>(&other);
  if (o != nullptr)
    return g_utf8_collate(purple_group_get_name(group_),
             purple_group_get_name(o->group_)) <= 0;
  return lessOrEqualByType(other);
}

void BuddyListGroup::update()
{
  BuddyListNode::update();

  setText(purple_group_get_name(group_));

  // Sort in the group.
  BuddyList::GroupSortMode mode = BUDDYLIST->getGroupSortMode();
  switch (mode) {
  case BuddyList::GROUP_SORT_BY_USER: {
    // Note that the sorting below works even if there was a contact/chat/buddy
    // node that is attached at the root level of the blist treeview. This
    // happens when such a node was just created (the new_node() callback was
    // called) but the node does not have any parent yet.

    PurpleBlistNode *prev = purple_blist_node_get_sibling_prev(blist_node_);

    if (prev != nullptr) {
      // It better be a group node.
      g_assert(PURPLE_BLIST_NODE_IS_GROUP(prev));

      BuddyListNode *bnode =
        reinterpret_cast<BuddyListNode *>(purple_blist_node_get_ui_data(prev));
      // There has to be ui_data set for all group nodes!
      g_assert(bnode != nullptr);

      treeview_->moveNodeAfter(ref_, bnode->getRefNode());
    }
    else {
      // The group is the first one in the list.
      CppConsUI::TreeView::NodeReference parent_ref = treeview_->getRootNode();
      treeview_->moveNodeBefore(ref_, parent_ref.begin());
    }
  } break;
  case BuddyList::GROUP_SORT_BY_NAME:
    sortIn();
    break;
  }

  bool vis = true;
  if (!BUDDYLIST->getShowEmptyGroupsPref())
    vis = purple_blist_get_group_size(group_, FALSE);
  setVisibility(vis);
}

void BuddyListGroup::onActivate(Button & /*activator*/)
{
  treeview_->toggleCollapsed(ref_);
  purple_blist_node_set_bool(blist_node_, "collapsed", ref_->isCollapsed());
}

const char *BuddyListGroup::toString() const
{
  return purple_group_get_name(group_);
}

void BuddyListGroup::setRefNode(CppConsUI::TreeView::NodeReference n)
{
  BuddyListNode::setRefNode(n);
  initCollapsedState();
}

void BuddyListGroup::initCollapsedState()
{
  // This cannot be done when the purple_blist_load() function was called
  // because node settings are unavailable at that time.
  treeview_->setCollapsed(
    ref_, purple_blist_node_get_bool(blist_node_, "collapsed"));
}

BuddyListGroup::GroupContextMenu::GroupContextMenu(BuddyListGroup &parent_group)
  : ContextMenu(parent_group), parent_group_(&parent_group)
{
  appendExtendedMenu();

  appendItem(_("Rename..."), sigc::mem_fun(this, &GroupContextMenu::onRename));
  appendItem(_("Delete..."), sigc::mem_fun(this, &GroupContextMenu::onRemove));

  if (BUDDYLIST->getGroupSortMode() == BuddyList::GROUP_SORT_BY_USER) {
    // If the manual sorting is enabled then show a menu item and a submenu for
    // group moving.
    auto groups = new CppConsUI::MenuWindow(*this, AUTOSIZE, AUTOSIZE);

    groups->appendItem(_("-Top-"),
      sigc::bind(sigc::mem_fun(this, &GroupContextMenu::onMoveAfter),
                         static_cast<PurpleGroup *>(nullptr)));
    for (PurpleBlistNode *node = purple_blist_get_root(); node != nullptr;
         node = purple_blist_node_get_sibling_next(node)) {
      if (!PURPLE_BLIST_NODE_IS_GROUP(node))
        continue;

      PurpleGroup *group = PURPLE_GROUP(node);
      groups->appendItem(purple_group_get_name(group),
        sigc::bind(sigc::mem_fun(this, &GroupContextMenu::onMoveAfter), group));
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
  PurpleGroup *group = parent_group_->getPurpleGroup();
  PurpleGroup *other = purple_find_group(name);
  if (other != nullptr &&
    !purple_utf8_strcasecmp(name, purple_group_get_name(group))) {
    LOG->message(_("Specified group is already in the list."));
    // TODO Add group merging. Note that purple_blist_rename_group() can do the
    // merging.
  }
  else
    purple_blist_rename_group(group, name);

  // Close context menu.
  close();
}

void BuddyListGroup::GroupContextMenu::onRename(Button & /*activator*/)
{
  PurpleGroup *group = parent_group_->getPurpleGroup();
  auto dialog =
    new CppConsUI::InputDialog(_("Rename"), purple_group_get_name(group));
  dialog->signal_response.connect(
    sigc::mem_fun(this, &GroupContextMenu::renameResponseHandler));
  dialog->show();
}

void BuddyListGroup::GroupContextMenu::removeResponseHandler(
  CppConsUI::MessageDialog & /*activator*/,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  // Based on gtkdialogs.c:pidgin_dialogs_remove_group_cb().
  PurpleGroup *group = parent_group_->getPurpleGroup();
  PurpleBlistNode *cnode =
    purple_blist_node_get_first_child(PURPLE_BLIST_NODE(group));
  while (cnode != nullptr) {
    if (PURPLE_BLIST_NODE_IS_CONTACT(cnode)) {
      PurpleBlistNode *bnode = purple_blist_node_get_first_child(cnode);
      cnode = purple_blist_node_get_sibling_next(cnode);
      while (bnode != nullptr)
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

  // Close the context menu before the group is deleted because its deletion can
  // lead to destruction of this object.
  close();

  purple_blist_remove_group(group);
}

void BuddyListGroup::GroupContextMenu::onRemove(Button & /*activator*/)
{
  PurpleGroup *group = parent_group_->getPurpleGroup();
  char *msg = g_strdup_printf(
    _("Are you sure you want to delete group %s from the list?"),
    purple_group_get_name(group));
  auto dialog = new CppConsUI::MessageDialog(_("Group deletion"), msg);
  g_free(msg);
  dialog->signal_response.connect(
    sigc::mem_fun(this, &GroupContextMenu::removeResponseHandler));
  dialog->show();
}

void BuddyListGroup::GroupContextMenu::onMoveAfter(
  Button & /*activator*/, PurpleGroup *group)
{
  PurpleGroup *moved_group = parent_group_->getPurpleGroup();
  close();

  purple_blist_add_group(moved_group, PURPLE_BLIST_NODE(group));
}

void BuddyListGroup::openContextMenu()
{
  ContextMenu *w = new GroupContextMenu(*this);
  w->show();
}

BuddyListGroup::BuddyListGroup(PurpleBlistNode *node) : BuddyListNode(node)
{
  setColorScheme(CenterIM::SCHEME_BUDDYLISTGROUP);

  group_ = PURPLE_GROUP(blist_node_);
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
