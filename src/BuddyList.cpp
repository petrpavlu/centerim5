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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "BuddyList.h"

#include "Footer.h"
#include "Log.h"
#include "Utils.h"

#include "gettext.h"
#include <cppconsui/Spacer.h>
#include <cstdlib>
#include <cstring>

BuddyList *BuddyList::my_instance_ = nullptr;

BuddyList *BuddyList::instance()
{
  return my_instance_;
}

bool BuddyList::processInputText(const TermKeyKey &key)
{
  if (!filter_->isVisible())
    return false;

  std::size_t input_len = strlen(key.utf8);
  if (filter_buffer_length_ + input_len + 1 > sizeof(filter_buffer_))
    return false;

  char *pos = filter_buffer_ + filter_buffer_length_;
  strcpy(pos, key.utf8);
  filter_buffer_onscreen_width_ += CppConsUI::Curses::onScreenWidth(pos);
  filter_buffer_length_ += input_len;

  updateList(UPDATE_OTHERS);
  redraw();

  return true;
}

bool BuddyList::restoreFocus()
{
  FOOTER->setText(_("%s act conv, %s main menu, %s context menu, %s filter"),
    "centerim|conversation-active", "centerim|generalmenu",
    "buddylist|contextmenu", "buddylist|filter");

  return CppConsUI::Window::restoreFocus();
}

void BuddyList::ungrabFocus()
{
  FOOTER->setText(nullptr);
  CppConsUI::Window::ungrabFocus();
}

void BuddyList::close()
{
  // BuddyList cannot be closed, instead close the filter input if it is active.
  if (!filter_->isVisible())
    return;

  filterHide();
  updateList(UPDATE_OTHERS);
}

void BuddyList::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::BUDDY_LIST_AREA));
}

void BuddyList::updateNode(PurpleBlistNode *node)
{
  update(buddylist_, node);
}

BuddyList::Filter::Filter(BuddyList *parent_blist)
  : Widget(AUTOSIZE, 1), parent_blist_(parent_blist)
{
}

int BuddyList::Filter::draw(
  CppConsUI::Curses::ViewPort area, CppConsUI::Error &error)
{
  int printed;
  DRAW(area.addString(0, 0, real_width_, _("Filter: "), error, &printed));
  if (static_cast<int>(parent_blist_->filter_buffer_onscreen_width_) <=
    real_width_ - printed) {
    // Optimized simple case.
    DRAW(area.addString(printed, 0, parent_blist_->filter_buffer_, error));
    return 0;
  }

  int w = 0;
  const char *cur =
    parent_blist_->filter_buffer_ + parent_blist_->filter_buffer_length_;
  while (true) {
    const char *prev =
      g_utf8_find_prev_char(parent_blist_->filter_buffer_, cur);
    if (!prev)
      break;
    gunichar uc = g_utf8_get_char(prev);
    int wc = CppConsUI::Curses::onScreenWidth(uc);
    if (w + wc > real_width_ - printed)
      break;
    w += wc;
    cur = prev;
  }
  DRAW(area.addString(real_width_ - w, 0, cur, error));

  return 0;
}

BuddyList::AddWindow::AddWindow(const char *title)
  : SplitDialog(0, 0, 80, 24, title)
{
  setColorScheme(CenterIM::SCHEME_GENERALWINDOW);

  treeview_ = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  setContainer(*treeview_);

  buttons_->appendItem(_("Add"), sigc::mem_fun(this, &AddWindow::onAddRequest));
  buttons_->appendSeparator();
  buttons_->appendItem(
    _("Cancel"), sigc::hide(sigc::mem_fun(this, &AddWindow::close)));

  onScreenResized();
}

void BuddyList::AddWindow::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::CHAT_AREA));
}

BuddyList::AddWindow::AccountOption::AccountOption(
  PurpleAccount *default_account, bool chat_only)
  : ComboBox(_("Account"))
{
  for (GList *l = purple_accounts_get_all(); l != nullptr; l = l->next) {
    PurpleAccount *account = static_cast<PurpleAccount *>(l->data);
    if (!purple_account_is_connected(account))
      continue;

    if (chat_only) {
      PurpleConnection *gc = purple_account_get_connection(account);
      if (PURPLE_PLUGIN_PROTOCOL_INFO(gc->prpl)->join_chat == nullptr)
        continue;
    }

    char *label =
      g_strdup_printf("[%s] %s", purple_account_get_protocol_name(account),
        purple_account_get_username(account));
    addOptionPtr(label, account);
    g_free(label);
  }
  setSelectedByDataPtr(default_account);
}

BuddyList::AddWindow::GroupOption::GroupOption(const char *default_group)
  : ComboBox(_("Group"))
{
  bool add_default_group = true;
  for (PurpleBlistNode *node = purple_blist_get_root(); node != nullptr;
       node = purple_blist_node_get_sibling_next(node))
    if (PURPLE_BLIST_NODE_IS_GROUP(node)) {
      const char *name = purple_group_get_name(PURPLE_GROUP(node));
      int opt = addOption(name);
      if (default_group != nullptr && std::strcmp(default_group, name) == 0)
        setSelected(opt);
      add_default_group = false;
    }
  if (add_default_group)
    addOption(_("Default"));
}

BuddyList::AddWindow::StringOption::StringOption(
  const char *text, const char *value, bool masked)
  : Button(FLAG_VALUE, text, value, nullptr, nullptr, masked)
{
  signal_activate.connect(sigc::mem_fun(this, &StringOption::onActivate));
}

void BuddyList::AddWindow::StringOption::onActivate(
  CppConsUI::Button & /*activator*/)
{
  auto dialog = new CppConsUI::InputDialog(getText(), getValue());
  dialog->setMasked(isMasked());
  dialog->signal_response.connect(
    sigc::mem_fun(this, &StringOption::responseHandler));
  dialog->show();
}

void BuddyList::AddWindow::StringOption::responseHandler(
  CppConsUI::InputDialog &activator,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  setValue(activator.getText());
}

BuddyList::AddWindow::IntegerOption::IntegerOption(
  const char *text, const char *value, bool masked, int min, int max)
  : Button(FLAG_VALUE, text, nullptr, nullptr, nullptr, masked),
    min_value_(min), max_value_(max)
{
  // Make sure that the default value is in the range.
  long i = std::strtol(value, nullptr, 10);
  i = CLAMP(i, min_value_, max_value_);
  setValue(i);

  signal_activate.connect(sigc::mem_fun(this, &IntegerOption::onActivate));
}

void BuddyList::AddWindow::IntegerOption::onActivate(
  CppConsUI::Button & /*activator*/)
{
  auto dialog = new CppConsUI::InputDialog(getText(), getValue());
  dialog->setFlags(CppConsUI::TextEntry::FLAG_NUMERIC);
  dialog->setMasked(isMasked());
  dialog->signal_response.connect(
    sigc::mem_fun(this, &IntegerOption::responseHandler));
  dialog->show();
}

void BuddyList::AddWindow::IntegerOption::responseHandler(
  CppConsUI::InputDialog &activator,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  long num;
  if (!Utils::stringToNumber(activator.getText(), min_value_, max_value_, &num))
    return;

  setValue(num);
}

BuddyList::AddWindow::BooleanOption::BooleanOption(
  const char *text, bool checked)
  : CheckBox(text, checked)
{
}

BuddyList::AddBuddyWindow::AddBuddyWindow(PurpleAccount *account,
  const char *username, const char *group, const char *alias)
  : AddWindow(_("Add buddy"))
{
  CppConsUI::TreeView::NodeReference parent = treeview_->getRootNode();

  account_option_ = new AccountOption(account);
  treeview_->appendNode(parent, *account_option_);
  account_option_->grabFocus();

  name_option_ = new StringOption(_("Buddy name"), username);
  treeview_->appendNode(parent, *name_option_);

  alias_option_ = new StringOption(_("Alias"), alias);
  treeview_->appendNode(parent, *alias_option_);

  group_option_ = new GroupOption(group);
  treeview_->appendNode(parent, *group_option_);
}

void BuddyList::AddBuddyWindow::onAddRequest(CppConsUI::Button & /*activator*/)
{
  PurpleAccount *account =
    static_cast<PurpleAccount *>(account_option_->getSelectedDataPtr());
  const char *name = name_option_->getValue();
  const char *alias = alias_option_->getValue();
  const char *group = group_option_->getSelectedTitle();

  if (!purple_account_is_connected(account)) {
    LOG->message(_("Selected account is not connected."));
    return;
  }
  if (name == nullptr || name[0] == '\0') {
    LOG->message(_("No buddy name specified."));
    return;
  }
  if (alias != nullptr && alias[0] == '\0')
    alias = nullptr;

  PurpleGroup *g = purple_find_group(group);
  if (g == nullptr) {
    g = purple_group_new(group);
    purple_blist_add_group(g, nullptr);
  }
  PurpleBuddy *b = purple_find_buddy_in_group(account, name, g);
  if (b != nullptr)
    LOG->message(_("Specified buddy is already in the list."));
  else {
    // Add the specified buddy.
    b = purple_buddy_new(account, name, alias);
    purple_blist_add_buddy(b, nullptr, g, nullptr);
    purple_account_add_buddy(account, b);
  }

  close();
}

BuddyList::AddChatWindow::AddChatWindow(PurpleAccount *account,
  const char * /*name*/, const char *alias, const char *group)
  : AddWindow(_("Add chat"))
{
  CppConsUI::TreeView::NodeReference parent = treeview_->getRootNode();

  account_option_ = new AccountOption(account, true);
  account_option_->signal_selection_changed.connect(
    sigc::mem_fun(this, &AddChatWindow::onAccountChanged));
  account_option_ref_ = treeview_->appendNode(parent, *account_option_);
  account_option_->grabFocus();

  alias_option_ = new StringOption(_("Alias"), alias);
  treeview_->appendNode(parent, *alias_option_);

  group_option_ = new GroupOption(group);
  treeview_->appendNode(parent, *group_option_);

  autojoin_option_ = new BooleanOption(_("Auto-join"), false);
  treeview_->appendNode(parent, *autojoin_option_);

  // Add protocol-specific fields.
  populateChatInfo(
    static_cast<PurpleAccount *>(account_option_->getSelectedDataPtr()));
}

void BuddyList::AddChatWindow::onAddRequest(CppConsUI::Button & /*activator*/)
{
  PurpleAccount *account =
    static_cast<PurpleAccount *>(account_option_->getSelectedDataPtr());
  const char *alias = alias_option_->getValue();
  const char *group = group_option_->getSelectedTitle();
  bool autojoin = autojoin_option_->isChecked();

  if (!purple_account_is_connected(account)) {
    LOG->message(_("Selected account is not connected."));
    return;
  }

  GHashTable *components =
    g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  for (ChatInfos::value_type &chat_info : chat_infos_) {
    CppConsUI::Button *button =
      dynamic_cast<CppConsUI::Button *>(chat_info.second->getWidget());
    g_assert(button != nullptr);
    const char *value = button->getValue();
    if (value[0] != '\0')
      g_hash_table_replace(components, g_strdup(chat_info.first.c_str()),
        g_strdup(button->getValue()));
  }

  PurpleChat *chat = purple_chat_new(account, alias, components);

  if (chat != nullptr) {
    PurpleGroup *g = purple_find_group(group);
    if (g == nullptr) {
      g = purple_group_new(group);
      purple_blist_add_group(g, nullptr);
    }
    purple_blist_add_chat(chat, g, nullptr);
    purple_blist_node_set_bool(
      PURPLE_BLIST_NODE(chat), PACKAGE_NAME "-autojoin", autojoin);
  }

  close();
}

void BuddyList::AddChatWindow::populateChatInfo(PurpleAccount *account)
{
  // Remove old entries.
  for (ChatInfos::value_type &chat_info : chat_infos_)
    treeview_->deleteNode(chat_info.second, false);
  chat_infos_.clear();

  PurpleConnection *gc = purple_account_get_connection(account);
  PurplePluginProtocolInfo *info =
    PURPLE_PLUGIN_PROTOCOL_INFO(purple_connection_get_prpl(gc));
  if (info->chat_info == nullptr)
    return;

  GHashTable *defaults = nullptr;
  if (info->chat_info_defaults != nullptr)
    defaults = info->chat_info_defaults(gc, "");

  CppConsUI::TreeView::NodeReference ref = account_option_ref_;
  for (GList *l = info->chat_info(gc); l != nullptr; l = l->next) {
    struct proto_chat_entry *entry =
      static_cast<struct proto_chat_entry *>(l->data);

    // Remove any accelerator from the label:
    char *label = Utils::stripAccelerator(entry->label);

    // And strip any trailing colon.
    std::size_t len = strlen(label);
    if (label[len - 1] == ':')
      label[len - 1] = '\0';

    // Get default value.
    const char *value = nullptr;
    if (defaults != nullptr)
      value = static_cast<const char *>(
        g_hash_table_lookup(defaults, entry->identifier));

    CppConsUI::Button *button;
    if (entry->is_int)
      button =
        new IntegerOption(label, value, entry->secret, entry->min, entry->max);
    else
      button = new StringOption(label, value, entry->secret);
    g_free(label);

    ref = treeview_->insertNodeAfter(ref, *button);
    chat_infos_[entry->identifier] = ref;
  }

  if (defaults != nullptr)
    g_hash_table_destroy(defaults);
}

void BuddyList::AddChatWindow::onAccountChanged(
  CppConsUI::Button & /*activator*/, std::size_t /*new_entry*/,
  const char * /*title*/, intptr_t data)
{
  populateChatInfo(reinterpret_cast<PurpleAccount *>(data));
}

BuddyList::AddGroupWindow::AddGroupWindow() : AddWindow(_("Add group"))
{
  name_option_ = new StringOption(_("Name"), _("New group"));
  treeview_->appendNode(treeview_->getRootNode(), *name_option_);
  name_option_->grabFocus();
}

void BuddyList::AddGroupWindow::onAddRequest(CppConsUI::Button & /*activator*/)
{
  const char *name = name_option_->getValue();
  if (name == nullptr || name[0] == '\0') {
    LOG->message(_("No group name specified."));
    return;
  }

  PurpleGroup *group = purple_group_new(name);
  purple_blist_add_group(group, nullptr);

  close();
}

BuddyList::BuddyList() : Window(0, 0, 80, 24)
{
  setColorScheme(CenterIM::SCHEME_BUDDYLIST);

  auto lbox = new CppConsUI::ListBox(AUTOSIZE, AUTOSIZE);
  addWidget(*lbox, 1, 1);

  auto hbox = new CppConsUI::HorizontalListBox(AUTOSIZE, AUTOSIZE);
  lbox->appendWidget(*hbox);
  hbox->appendWidget(*(new CppConsUI::Spacer(1, AUTOSIZE)));
  treeview_ = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  hbox->appendWidget(*treeview_);
  hbox->appendWidget(*(new CppConsUI::Spacer(1, AUTOSIZE)));

  filter_ = new Filter(this);
  filterHide();
  lbox->appendWidget(*filter_);

  // TODO Check if this has been moved to purple_blist_init(). Remove these
  // lines if it was as this will probably move to purple_init(), the buddylist
  // object should be available a lot more early and the uiops should be set a
  // lot more early. (All in all a lot of work.)
  buddylist_ = purple_blist_new();
  buddylist_->ui_data = this;
  purple_set_blist(buddylist_);

  // Load the pounces.
  purple_pounces_load();

  // Init prefs.
  purple_prefs_add_none(CONF_PREFIX "/blist");
  purple_prefs_add_bool(CONF_PREFIX "/blist/show_empty_groups", false);
  purple_prefs_add_bool(CONF_PREFIX "/blist/show_offline_buddies", true);
  purple_prefs_add_string(CONF_PREFIX "/blist/list_mode", "normal");
  purple_prefs_add_string(CONF_PREFIX "/blist/group_sort_mode", "name");
  purple_prefs_add_string(CONF_PREFIX "/blist/buddy_sort_mode", "status");
  purple_prefs_add_string(CONF_PREFIX "/blist/colorization_mode", "none");

  updateCachedPreference(CONF_PREFIX "/blist/show_empty_groups");
  updateCachedPreference(CONF_PREFIX "/blist/show_offline_buddies");
  updateCachedPreference(CONF_PREFIX "/blist/list_mode");
  updateCachedPreference(CONF_PREFIX "/blist/group_sort_mode");
  updateCachedPreference(CONF_PREFIX "/blist/buddy_sort_mode");
  updateCachedPreference(CONF_PREFIX "/blist/colorization_mode");

  // Connect callbacks.
  purple_prefs_connect_callback(
    this, CONF_PREFIX "/blist", blist_pref_change_, this);

  // Setup the callbacks for the buddylist.
  std::memset(&centerim_blist_ui_ops_, 0, sizeof(centerim_blist_ui_ops_));
  centerim_blist_ui_ops_.new_list = new_list_;
  centerim_blist_ui_ops_.new_node = new_node_;
  // centerim_blist_ui_ops_.show = show_;
  centerim_blist_ui_ops_.update = update_;
  centerim_blist_ui_ops_.remove = remove_;
  centerim_blist_ui_ops_.destroy = destroy_;
  // centerim_blist_ui_ops_.set_visible = set_visible_;
  centerim_blist_ui_ops_.request_add_buddy = request_add_buddy_;
  centerim_blist_ui_ops_.request_add_chat = request_add_chat_;
  centerim_blist_ui_ops_.request_add_group = request_add_group_;
  // Since libpurple 2.6.0.
  // centerim_blist_ui_ops_.save_node = save_node_;
  // centerim_blist_ui_ops_.remove_node = remove_node_;
  // centerim_blist_ui_ops_.save_account = save_account_;
  purple_blist_set_ui_ops(&centerim_blist_ui_ops_);

  CENTERIM->timeoutOnceConnect(sigc::mem_fun(this, &BuddyList::load), 0);

  onScreenResized();
  declareBindables();
}

BuddyList::~BuddyList()
{
  purple_blist_set_ui_ops(nullptr);
  purple_prefs_disconnect_by_handle(this);
}

void BuddyList::init()
{
  g_assert(my_instance_ == nullptr);

  my_instance_ = new BuddyList;
  my_instance_->show();
}

void BuddyList::finalize()
{
  g_assert(my_instance_ != nullptr);

  delete my_instance_;
  my_instance_ = nullptr;
}

void BuddyList::load()
{
  // Load the buddy list from ~/.centerim5/blist.xml.
  purple_blist_load();

  delayedGroupNodesInit();
}

void BuddyList::rebuildList()
{
  treeview_->clear();

  for (PurpleBlistNode *node = purple_blist_get_root(); node != nullptr;
       node = purple_blist_node_next(node, TRUE))
    new_node(node);
}

void BuddyList::updateList(int flags)
{
  for (PurpleBlistNode *node = purple_blist_get_root(); node != nullptr;
       node = purple_blist_node_next(node, TRUE))
    if (PURPLE_BLIST_NODE_IS_GROUP(node) ? (flags & UPDATE_GROUPS)
                                         : (flags & UPDATE_OTHERS)) {
      BuddyListNode *bnode =
        reinterpret_cast<BuddyListNode *>(purple_blist_node_get_ui_data(node));
      if (bnode != nullptr)
        bnode->update();
    }
}

void BuddyList::delayedGroupNodesInit()
{
  // Delayed group nodes init.
  for (PurpleBlistNode *node = purple_blist_get_root(); node != nullptr;
       node = purple_blist_node_get_sibling_next(node))
    if (PURPLE_BLIST_NODE_IS_GROUP(node)) {
      BuddyListGroup *gnode =
        reinterpret_cast<BuddyListGroup *>(purple_blist_node_get_ui_data(node));
      if (gnode != nullptr)
        gnode->initCollapsedState();
    }
}

void BuddyList::updateCachedPreference(const char *name)
{
  if (std::strcmp(name, CONF_PREFIX "/blist/show_empty_groups") == 0)
    show_empty_groups_ = purple_prefs_get_bool(name);
  else if (std::strcmp(name, CONF_PREFIX "/blist/show_offline_buddies") == 0)
    show_offline_buddies_ = purple_prefs_get_bool(name);
  else if (std::strcmp(name, CONF_PREFIX "/blist/list_mode") == 0) {
    const char *value = purple_prefs_get_string(name);
    if (std::strcmp(value, "flat") == 0)
      list_mode_ = LIST_FLAT;
    else
      list_mode_ = LIST_NORMAL;
  }
  else if (std::strcmp(name, CONF_PREFIX "/blist/group_sort_mode") == 0) {
    const char *value = purple_prefs_get_string(name);
    if (std::strcmp(value, "user") == 0)
      group_sort_mode_ = GROUP_SORT_BY_USER;
    else
      group_sort_mode_ = GROUP_SORT_BY_NAME;
  }
  else if (std::strcmp(name, CONF_PREFIX "/blist/buddy_sort_mode") == 0) {
    const char *value = purple_prefs_get_string(name);
    if (std::strcmp(value, "status") == 0)
      buddy_sort_mode_ = BUDDY_SORT_BY_STATUS;
    else if (std::strcmp(value, "activity") == 0)
      buddy_sort_mode_ = BUDDY_SORT_BY_ACTIVITY;
    else
      buddy_sort_mode_ = BUDDY_SORT_BY_NAME;
  }
  else if (std::strcmp(name, CONF_PREFIX "/blist/colorization_mode") == 0) {
    const char *value = purple_prefs_get_string(name);
    if (std::strcmp(value, "status") == 0)
      colorization_mode_ = COLOR_BY_STATUS;
    else if (std::strcmp(value, "account") != 0)
      colorization_mode_ = COLOR_BY_ACCOUNT;
    else
      colorization_mode_ = COLOR_NONE;
  }
}

bool BuddyList::isAnyAccountConnected()
{
  for (GList *list = purple_accounts_get_all(); list != nullptr;
       list = list->next) {
    PurpleAccount *account = reinterpret_cast<PurpleAccount *>(list->data);
    if (purple_account_is_connected(account))
      return true;
  }

  LOG->message(_("There are no connected accounts."));
  return false;
}

void BuddyList::filterHide()
{
  filter_->setVisibility(false);
  filter_buffer_[0] = '\0';
  filter_buffer_length_ = 0;
  filter_buffer_onscreen_width_ = 0;
}

void BuddyList::actionOpenFilter()
{
  if (filter_->isVisible())
    return;

  filter_->setVisibility(true);

  // Stay sane.
  g_assert(filter_buffer_[0] == '\0');
  g_assert(filter_buffer_length_ == 0);
  g_assert(filter_buffer_onscreen_width_ == 0);
}

void BuddyList::actionDeleteChar()
{
  if (!filter_->isVisible())
    return;

  const char *end = filter_buffer_ + filter_buffer_length_;
  g_assert(*end == '\0');
  char *prev = g_utf8_find_prev_char(filter_buffer_, end);
  if (prev != nullptr) {
    filter_buffer_length_ -= end - prev;
    filter_buffer_onscreen_width_ -= CppConsUI::Curses::onScreenWidth(prev);
    *prev = '\0';
  }
  else
    filterHide();

  updateList(UPDATE_OTHERS);
  redraw();
}

void BuddyList::declareBindables()
{
  declareBindable("buddylist", "filter",
    sigc::mem_fun(this, &BuddyList::actionOpenFilter),
    InputProcessor::BINDABLE_NORMAL);
  declareBindable("textentry", "backspace",
    sigc::mem_fun(this, &BuddyList::actionDeleteChar),
    InputProcessor::BINDABLE_NORMAL);
}

void BuddyList::new_list(PurpleBuddyList *list)
{
  if (buddylist_ != list)
    LOG->error(_("Different buddylist detected!"));
}

void BuddyList::new_node(PurpleBlistNode *node)
{
  g_return_if_fail(!purple_blist_node_get_ui_data(node));

  if (PURPLE_BLIST_NODE_IS_GROUP(node) && list_mode_ == BuddyList::LIST_FLAT) {
    // Flat mode = no groups.
    return;
  }

  BuddyListNode *bnode = BuddyListNode::createNode(node);
  if (bnode == nullptr)
    return;

  BuddyListNode *parent = bnode->getParentNode();
  CppConsUI::TreeView::NodeReference nref = treeview_->appendNode(
    parent ? parent->getRefNode() : treeview_->getRootNode(), *bnode);
  bnode->setRefNode(nref);
  bnode->update();
}

void BuddyList::update(PurpleBuddyList *list, PurpleBlistNode *node)
{
  // Not cool, but necessary because libpurple does not always behave nice. Note
  // that calling new_node() can modify node's ui_data.
  if (purple_blist_node_get_ui_data(node) == nullptr)
    new_node(node);

  BuddyListNode *bnode =
    reinterpret_cast<BuddyListNode *>(purple_blist_node_get_ui_data(node));
  if (bnode == nullptr)
    return;

  // Update the node data.
  bnode->update();

  if (node->parent != nullptr)
    update(list, node->parent);
}

void BuddyList::remove(PurpleBuddyList *list, PurpleBlistNode *node)
{
  BuddyListNode *bnode =
    reinterpret_cast<BuddyListNode *>(purple_blist_node_get_ui_data(node));
  if (bnode == nullptr)
    return;

  treeview_->deleteNode(bnode->getRefNode(), false);

  if (node->parent != nullptr)
    update(list, node->parent);
}

void BuddyList::destroy(PurpleBuddyList * /*list*/)
{
}

void BuddyList::request_add_buddy(PurpleAccount *account, const char *username,
  const char *group, const char *alias)
{
  if (!isAnyAccountConnected())
    return;

  auto window = new AddBuddyWindow(account, username, group, alias);
  window->show();
}

void BuddyList::request_add_chat(PurpleAccount *account, PurpleGroup *group,
  const char *alias, const char *name)
{
  if (!isAnyAccountConnected())
    return;

  // Find an account with chat capabilities.
  bool chat_account_found = false;
  for (GList *l = purple_connections_get_all(); l != nullptr; l = l->next) {
    PurpleConnection *gc = reinterpret_cast<PurpleConnection *>(l->data);

    if (PURPLE_PLUGIN_PROTOCOL_INFO(gc->prpl)->join_chat) {
      chat_account_found = true;
      break;
    }
  }

  if (!chat_account_found) {
    LOG->message(_("You are not currently signed on with any "
                   "protocols that have the ability to chat."));
    return;
  }

  const char *group_name = nullptr;
  if (group != nullptr)
    group_name = purple_group_get_name(group);
  auto window = new AddChatWindow(account, name, alias, group_name);
  window->show();
}

void BuddyList::request_add_group()
{
  if (!isAnyAccountConnected())
    return;

  auto window = new AddGroupWindow;
  window->show();
}

void BuddyList::blist_pref_change(
  const char *name, PurplePrefType /*type*/, gconstpointer /*val*/)
{
  // Some blist/* preference changed.
  updateCachedPreference(name);

  if (std::strcmp(name, CONF_PREFIX "/blist/list_mode") == 0) {
    rebuildList();
    return;
  }

  bool groups_only = false;
  if (std::strcmp(name, CONF_PREFIX "/blist/show_empty_groups") == 0 ||
    std::strcmp(name, CONF_PREFIX "/blist/group_sort_mode") == 0)
    groups_only = true;

  updateList(UPDATE_GROUPS | (!groups_only ? UPDATE_OTHERS : 0));
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
