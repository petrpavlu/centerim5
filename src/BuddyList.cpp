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

#include "BuddyList.h"

#include "Footer.h"
#include "Log.h"
#include "Utils.h"

#include <cppconsui/Spacer.h>
#include <errno.h>
#include "gettext.h"

BuddyList *BuddyList::my_instance = NULL;

BuddyList *BuddyList::instance()
{
  return my_instance;
}

bool BuddyList::processInputText(const TermKeyKey &key)
{
  if (!filter->isVisible())
    return false;

  size_t input_len = strlen(key.utf8);
  if (filter_buffer_length + input_len + 1 > sizeof(filter_buffer))
    return false;

  char *pos = filter_buffer + filter_buffer_length;
  strcpy(pos, key.utf8);
  filter_buffer_onscreen_width += CppConsUI::Curses::onScreenWidth(pos);
  filter_buffer_length += input_len;

  updateList(UPDATE_OTHERS);
  redraw();

  return true;
}

bool BuddyList::restoreFocus()
{
  FOOTER->setText(_("%s act conv, %s main menu, %s context menu, "
                    "%s filter"),
    "centerim|conversation-active", "centerim|generalmenu",
    "buddylist|contextmenu", "buddylist|filter");

  return CppConsUI::Window::restoreFocus();
}

void BuddyList::ungrabFocus()
{
  FOOTER->setText(NULL);
  CppConsUI::Window::ungrabFocus();
}

void BuddyList::close()
{
  // BuddyList can't be closed, instead close the filter input if it's active
  if (!filter->isVisible())
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
  update(buddylist, node);
}

BuddyList::Filter::Filter(BuddyList *parent_)
  : Widget(AUTOSIZE, 1), parent(parent_)
{
}

void BuddyList::Filter::draw(CppConsUI::Curses::ViewPort area)
{
  int x = 0;
  x += area.addString(x, 0, real_width - x, _("Filter: "));
  if (static_cast<int>(parent->filter_buffer_onscreen_width) <=
    real_width - x) {
    // optimized simple case
    area.addString(x, 0, parent->filter_buffer);
    return;
  }

  int w = 0;
  const char *cur = parent->filter_buffer + parent->filter_buffer_length;
  while (true) {
    const char *prev = g_utf8_find_prev_char(parent->filter_buffer, cur);
    if (!prev)
      break;
    gunichar uc = g_utf8_get_char(prev);
    int wc = CppConsUI::Curses::onScreenWidth(uc);
    if (w + wc > real_width - x)
      break;
    w += wc;
    cur = prev;
  }
  area.addString(real_width - w, 0, cur);
}

BuddyList::AddWindow::AddWindow(const char *title)
  : SplitDialog(0, 0, 80, 24, title)
{
  setColorScheme("generalwindow");

  treeview = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  setContainer(*treeview);

  buttons->appendItem(_("Add"), sigc::mem_fun(this, &AddWindow::onAddRequest));
  buttons->appendSeparator();
  buttons->appendItem(
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
  for (GList *l = purple_accounts_get_all(); l; l = l->next) {
    PurpleAccount *account = static_cast<PurpleAccount *>(l->data);
    if (!purple_account_is_connected(account))
      continue;

    if (chat_only) {
      PurpleConnection *gc = purple_account_get_connection(account);
      if (!PURPLE_PLUGIN_PROTOCOL_INFO(gc->prpl)->join_chat)
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
  for (PurpleBlistNode *node = purple_blist_get_root(); node;
       node = purple_blist_node_get_sibling_next(node))
    if (PURPLE_BLIST_NODE_IS_GROUP(node)) {
      const char *name = purple_group_get_name(PURPLE_GROUP(node));
      int opt = addOption(name);
      if (default_group && !strcmp(default_group, name))
        setSelected(opt);
      add_default_group = false;
    }
  if (add_default_group)
    addOption(_("Default"));
}

BuddyList::AddWindow::StringOption::StringOption(
  const char *text, const char *value, bool masked)
  : Button(FLAG_VALUE, text, value, NULL, NULL, masked)
{
  signal_activate.connect(sigc::mem_fun(this, &StringOption::onActivate));
}

void BuddyList::AddWindow::StringOption::onActivate(
  CppConsUI::Button & /*activator*/)
{
  CppConsUI::InputDialog *dialog =
    new CppConsUI::InputDialog(getText(), getValue());
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
  : Button(FLAG_VALUE, text, NULL, NULL, NULL, masked), min_value(min),
    max_value(max)
{
  // make sure that the default value is in the range
  long i = strtol(value, NULL, 10);
  i = CLAMP(i, min, max);
  setValue(i);

  signal_activate.connect(sigc::mem_fun(this, &IntegerOption::onActivate));
}

void BuddyList::AddWindow::IntegerOption::onActivate(
  CppConsUI::Button & /*activator*/)
{
  CppConsUI::InputDialog *dialog =
    new CppConsUI::InputDialog(getText(), getValue());
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

  const char *text = activator.getText();
  errno = 0;
  long i = strtol(text, NULL, 10);
  if (errno == ERANGE || i > max_value || i < min_value)
    LOG->warning(_("Value is out of range."));
  i = CLAMP(i, min_value, max_value);
  setValue(i);
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
  CppConsUI::TreeView::NodeReference parent = treeview->getRootNode();

  account_option = new AccountOption(account);
  treeview->appendNode(parent, *account_option);
  account_option->grabFocus();

  name_option = new StringOption(_("Buddy name"), username);
  treeview->appendNode(parent, *name_option);

  alias_option = new StringOption(_("Alias"), alias);
  treeview->appendNode(parent, *alias_option);

  group_option = new GroupOption(group);
  treeview->appendNode(parent, *group_option);
}

void BuddyList::AddBuddyWindow::onAddRequest(CppConsUI::Button & /*activator*/)
{
  PurpleAccount *account =
    static_cast<PurpleAccount *>(account_option->getSelectedDataPtr());
  const char *name = name_option->getValue();
  const char *alias = alias_option->getValue();
  const char *group = group_option->getSelectedTitle();

  if (!purple_account_is_connected(account)) {
    LOG->message(_("Selected account is not connected."));
    return;
  }
  if (!name || !name[0]) {
    LOG->message(_("No buddy name specified."));
    return;
  }
  if (alias && !alias[0])
    alias = NULL;

  PurpleGroup *g = purple_find_group(group);
  if (!g) {
    g = purple_group_new(group);
    purple_blist_add_group(g, NULL);
  }
  PurpleBuddy *b = purple_find_buddy_in_group(account, name, g);
  if (b)
    LOG->message(_("Specified buddy is already in the list."));
  else {
    // add the specified buddy
    b = purple_buddy_new(account, name, alias);
    purple_blist_add_buddy(b, NULL, g, NULL);
    purple_account_add_buddy(account, b);
  }

  close();
}

BuddyList::AddChatWindow::AddChatWindow(PurpleAccount *account,
  const char * /*name*/, const char *alias, const char *group)
  : AddWindow(_("Add chat"))
{
  CppConsUI::TreeView::NodeReference parent = treeview->getRootNode();

  account_option = new AccountOption(account, true);
  account_option->signal_selection_changed.connect(
    sigc::mem_fun(this, &AddChatWindow::onAccountChanged));
  account_option_ref = treeview->appendNode(parent, *account_option);
  account_option->grabFocus();

  alias_option = new StringOption(_("Alias"), alias);
  treeview->appendNode(parent, *alias_option);

  group_option = new GroupOption(group);
  treeview->appendNode(parent, *group_option);

  autojoin_option = new BooleanOption(_("Auto-join"), false);
  treeview->appendNode(parent, *autojoin_option);

  // add protocol-specific fields
  populateChatInfo(
    static_cast<PurpleAccount *>(account_option->getSelectedDataPtr()));
}

void BuddyList::AddChatWindow::onAddRequest(CppConsUI::Button & /*activator*/)
{
  PurpleAccount *account =
    static_cast<PurpleAccount *>(account_option->getSelectedDataPtr());
  const char *alias = alias_option->getValue();
  const char *group = group_option->getSelectedTitle();
  bool autojoin = autojoin_option->isChecked();

  if (!purple_account_is_connected(account)) {
    LOG->message(_("Selected account is not connected."));
    return;
  }

  GHashTable *components =
    g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  for (ChatInfoMap::iterator i = chat_info_map.begin();
       i != chat_info_map.end(); i++) {
    CppConsUI::Button *button =
      dynamic_cast<CppConsUI::Button *>(i->second->getWidget());
    g_assert(button);
    const char *value = button->getValue();
    if (value[0])
      g_hash_table_replace(
        components, g_strdup(i->first.c_str()), g_strdup(button->getValue()));
  }

  PurpleChat *chat = purple_chat_new(account, alias, components);

  if (chat) {
    PurpleGroup *g = purple_find_group(group);
    if (!g) {
      g = purple_group_new(group);
      purple_blist_add_group(g, NULL);
    }
    purple_blist_add_chat(chat, g, NULL);
    purple_blist_node_set_bool(
      PURPLE_BLIST_NODE(chat), PACKAGE_NAME "-autojoin", autojoin);
  }

  close();
}

void BuddyList::AddChatWindow::populateChatInfo(PurpleAccount *account)
{
  // remove old entries
  for (ChatInfoMap::iterator i = chat_info_map.begin();
       i != chat_info_map.end(); i++)
    treeview->deleteNode(i->second, false);
  chat_info_map.clear();

  PurpleConnection *gc = purple_account_get_connection(account);
  PurplePluginProtocolInfo *info =
    PURPLE_PLUGIN_PROTOCOL_INFO(purple_connection_get_prpl(gc));
  if (!info->chat_info)
    return;

  GHashTable *defaults = NULL;
  if (info->chat_info_defaults)
    defaults = info->chat_info_defaults(gc, "");

  CppConsUI::TreeView::NodeReference ref = account_option_ref;
  for (GList *l = info->chat_info(gc); l; l = l->next) {
    struct proto_chat_entry *entry =
      static_cast<struct proto_chat_entry *>(l->data);

    // remove any accelerator from the label
    char *label = Utils::stripAccelerator(entry->label);

    // and strip any trailing colon
    size_t len = strlen(label);
    if (label[len - 1] == ':')
      label[len - 1] = '\0';

    // get default value
    const char *value = NULL;
    if (defaults)
      value = static_cast<const char *>(
        g_hash_table_lookup(defaults, entry->identifier));

    CppConsUI::Button *button;
    if (entry->is_int)
      button =
        new IntegerOption(label, value, entry->secret, entry->min, entry->max);
    else
      button = new StringOption(label, value, entry->secret);
    g_free(label);

    ref = treeview->insertNodeAfter(ref, *button);
    chat_info_map[entry->identifier] = ref;
  }

  if (defaults)
    g_hash_table_destroy(defaults);
}

void BuddyList::AddChatWindow::onAccountChanged(
  CppConsUI::Button & /*activator*/, size_t /*new_entry*/,
  const char * /*title*/, intptr_t data)
{
  populateChatInfo(reinterpret_cast<PurpleAccount *>(data));
}

BuddyList::AddGroupWindow::AddGroupWindow() : AddWindow(_("Add group"))
{
  name_option = new StringOption(_("Name"), _("New group"));
  treeview->appendNode(treeview->getRootNode(), *name_option);
  name_option->grabFocus();
}

void BuddyList::AddGroupWindow::onAddRequest(CppConsUI::Button & /*activator*/)
{
  const char *name = name_option->getValue();
  if (!name || !name[0]) {
    LOG->message(_("No group name specified."));
    return;
  }

  PurpleGroup *group = purple_group_new(name);
  purple_blist_add_group(group, NULL);

  close();
}

BuddyList::BuddyList() : Window(0, 0, 80, 24)
{
  setColorScheme("buddylist");

  CppConsUI::ListBox *lbox = new CppConsUI::ListBox(AUTOSIZE, AUTOSIZE);
  addWidget(*lbox, 1, 1);

  CppConsUI::HorizontalListBox *hbox =
    new CppConsUI::HorizontalListBox(AUTOSIZE, AUTOSIZE);
  lbox->appendWidget(*hbox);
  hbox->appendWidget(*(new CppConsUI::Spacer(1, AUTOSIZE)));
  treeview = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  hbox->appendWidget(*treeview);
  hbox->appendWidget(*(new CppConsUI::Spacer(1, AUTOSIZE)));

  filter = new Filter(this);
  filterHide();
  lbox->appendWidget(*filter);

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

  // connect callbacks
  purple_prefs_connect_callback(
    this, CONF_PREFIX "/blist", blist_pref_change_, this);

  // setup the callbacks for the buddylist
  memset(&centerim_blist_ui_ops, 0, sizeof(centerim_blist_ui_ops));
  centerim_blist_ui_ops.new_list = new_list_;
  centerim_blist_ui_ops.new_node = new_node_;
  // centerim_blist_ui_ops.show = show_;
  centerim_blist_ui_ops.update = update_;
  centerim_blist_ui_ops.remove = remove_;
  centerim_blist_ui_ops.destroy = destroy_;
  // centerim_blist_ui_ops.set_visible = set_visible_;
  centerim_blist_ui_ops.request_add_buddy = request_add_buddy_;
  centerim_blist_ui_ops.request_add_chat = request_add_chat_;
  centerim_blist_ui_ops.request_add_group = request_add_group_;
  // since libpurple 2.6.0
  // centerim_blist_ui_ops.save_node = save_node_;
  // centerim_blist_ui_ops.remove_node = remove_node_;
  // centerim_blist_ui_ops.save_account = save_account_;
  purple_blist_set_ui_ops(&centerim_blist_ui_ops);

  CENTERIM->timeoutOnceConnect(sigc::mem_fun(this, &BuddyList::load), 0);

  onScreenResized();
  declareBindables();
}

BuddyList::~BuddyList()
{
  purple_blist_set_ui_ops(NULL);
  purple_prefs_disconnect_by_handle(this);
}

void BuddyList::init()
{
  g_assert(!my_instance);

  my_instance = new BuddyList;
  my_instance->show();
}

void BuddyList::finalize()
{
  g_assert(my_instance);

  delete my_instance;
  my_instance = NULL;
}

void BuddyList::load()
{
  // load the buddy list from ~/.centerim5/blist.xml
  purple_blist_load();

  delayedGroupNodesInit();
}

void BuddyList::rebuildList()
{
  treeview->clear();

  PurpleBlistNode *node = purple_blist_get_root();
  while (node) {
    new_node(node);
    node = purple_blist_node_next(node, TRUE);
  }
}

void BuddyList::updateList(int flags)
{
  for (PurpleBlistNode *node = purple_blist_get_root(); node;
       node = purple_blist_node_next(node, TRUE))
    if (PURPLE_BLIST_NODE_IS_GROUP(node) ? (flags & UPDATE_GROUPS)
                                         : (flags & UPDATE_OTHERS)) {
      BuddyListNode *bnode =
        reinterpret_cast<BuddyListNode *>(purple_blist_node_get_ui_data(node));
      if (bnode)
        bnode->update();
    }
}

void BuddyList::delayedGroupNodesInit()
{
  // delayed group nodes init
  for (PurpleBlistNode *node = purple_blist_get_root(); node;
       node = purple_blist_node_get_sibling_next(node))
    if (PURPLE_BLIST_NODE_IS_GROUP(node)) {
      BuddyListGroup *gnode =
        reinterpret_cast<BuddyListGroup *>(purple_blist_node_get_ui_data(node));
      if (gnode)
        gnode->initCollapsedState();
    }
}

void BuddyList::updateCachedPreference(const char *name)
{
  if (!strcmp(name, CONF_PREFIX "/blist/show_empty_groups"))
    show_empty_groups = purple_prefs_get_bool(name);
  else if (!strcmp(name, CONF_PREFIX "/blist/show_offline_buddies"))
    show_offline_buddies = purple_prefs_get_bool(name);
  else if (!strcmp(name, CONF_PREFIX "/blist/list_mode")) {
    const char *value = purple_prefs_get_string(name);
    if (!strcmp(value, "flat"))
      list_mode = LIST_FLAT;
    else
      list_mode = LIST_NORMAL;
  }
  else if (!strcmp(name, CONF_PREFIX "/blist/group_sort_mode")) {
    const char *value = purple_prefs_get_string(name);
    if (!strcmp(value, "user"))
      group_sort_mode = GROUP_SORT_BY_USER;
    else
      group_sort_mode = GROUP_SORT_BY_NAME;
  }
  else if (!strcmp(name, CONF_PREFIX "/blist/buddy_sort_mode")) {
    const char *value = purple_prefs_get_string(name);
    if (!strcmp(value, "status"))
      buddy_sort_mode = BUDDY_SORT_BY_STATUS;
    else if (!strcmp(value, "activity"))
      buddy_sort_mode = BUDDY_SORT_BY_ACTIVITY;
    else
      buddy_sort_mode = BUDDY_SORT_BY_NAME;
  }
  else if (!strcmp(name, CONF_PREFIX "/blist/colorization_mode")) {
    const char *value = purple_prefs_get_string(name);
    if (!strcmp(value, "status"))
      colorization_mode = COLOR_BY_STATUS;
    else if (!strcmp(value, "account"))
      colorization_mode = COLOR_BY_ACCOUNT;
    else
      colorization_mode = COLOR_NONE;
  }
}

bool BuddyList::isAnyAccountConnected()
{
  for (GList *list = purple_accounts_get_all(); list; list = list->next) {
    PurpleAccount *account = reinterpret_cast<PurpleAccount *>(list->data);
    if (purple_account_is_connected(account))
      return true;
  }

  LOG->message(_("There are no connected accounts."));
  return false;
}

void BuddyList::filterHide()
{
  filter->setVisibility(false);
  filter_buffer[0] = '\0';
  filter_buffer_length = 0;
  filter_buffer_onscreen_width = 0;
}

void BuddyList::actionOpenFilter()
{
  if (filter->isVisible())
    return;

  filter->setVisibility(true);

  // stay sane
  g_assert(!filter_buffer[0]);
  g_assert(!filter_buffer_length);
  g_assert(!filter_buffer_onscreen_width);
}

void BuddyList::actionDeleteChar()
{
  if (!filter->isVisible())
    return;

  const char *end = filter_buffer + filter_buffer_length;
  g_assert(*end == '\0');
  char *prev = g_utf8_find_prev_char(filter_buffer, end);
  if (prev) {
    filter_buffer_length -= end - prev;
    filter_buffer_onscreen_width -= CppConsUI::Curses::onScreenWidth(prev);
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
  if (buddylist != list)
    LOG->error(_("Different buddylist detected!"));
}

void BuddyList::new_node(PurpleBlistNode *node)
{
  g_return_if_fail(!purple_blist_node_get_ui_data(node));

  if (PURPLE_BLIST_NODE_IS_GROUP(node) && list_mode == BuddyList::LIST_FLAT) {
    // flat mode = no groups
    return;
  }

  BuddyListNode *bnode = BuddyListNode::createNode(node);
  if (!bnode)
    return;

  BuddyListNode *parent = bnode->getParentNode();
  CppConsUI::TreeView::NodeReference nref = treeview->appendNode(
    parent ? parent->getRefNode() : treeview->getRootNode(), *bnode);
  bnode->setRefNode(nref);
  bnode->update();
}

void BuddyList::update(PurpleBuddyList *list, PurpleBlistNode *node)
{
  /* Not cool, but necessary because libpurple doesn't always behave nice.
   * Note that calling new_node() can modify node's ui_data. */
  if (!purple_blist_node_get_ui_data(node))
    new_node(node);

  BuddyListNode *bnode =
    reinterpret_cast<BuddyListNode *>(purple_blist_node_get_ui_data(node));
  if (!bnode)
    return;

  // update the node data
  bnode->update();

  if (node->parent)
    update(list, node->parent);
}

void BuddyList::remove(PurpleBuddyList *list, PurpleBlistNode *node)
{
  BuddyListNode *bnode =
    reinterpret_cast<BuddyListNode *>(purple_blist_node_get_ui_data(node));
  if (!bnode)
    return;

  treeview->deleteNode(bnode->getRefNode(), false);

  if (node->parent)
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

  AddBuddyWindow *window = new AddBuddyWindow(account, username, group, alias);
  window->show();
}

void BuddyList::request_add_chat(PurpleAccount *account, PurpleGroup *group,
  const char *alias, const char *name)
{
  if (!isAnyAccountConnected())
    return;

  // find an account with chat capabilities
  bool chat_account_found = false;
  for (GList *l = purple_connections_get_all(); l; l = l->next) {
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

  const char *group_name = NULL;
  if (group)
    group_name = purple_group_get_name(group);
  AddChatWindow *window = new AddChatWindow(account, name, alias, group_name);
  window->show();
}

void BuddyList::request_add_group()
{
  if (!isAnyAccountConnected())
    return;

  AddGroupWindow *window = new AddGroupWindow;
  window->show();
}

void BuddyList::blist_pref_change(
  const char *name, PurplePrefType /*type*/, gconstpointer /*val*/)
{
  // blist/* preference changed
  updateCachedPreference(name);

  if (!strcmp(name, CONF_PREFIX "/blist/list_mode")) {
    rebuildList();
    return;
  }

  bool groups_only = false;
  if (!strcmp(name, CONF_PREFIX "/blist/show_empty_groups") ||
    !strcmp(name, CONF_PREFIX "/blist/group_sort_mode"))
    groups_only = true;

  updateList(UPDATE_GROUPS | (!groups_only ? UPDATE_OTHERS : 0));
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
