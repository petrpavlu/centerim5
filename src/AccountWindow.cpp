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

#include "AccountWindow.h"

#include "Log.h"

#include <errno.h>
#include "gettext.h"

AccountWindow::AccountWindow() : SplitDialog(0, 0, 80, 24, _("Accounts"))
{
  setColorScheme("generalwindow");

  treeview = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  setContainer(*treeview);

  // populate all defined accounts
  for (GList *i = purple_accounts_get_all(); i; i = i->next)
    populateAccount(reinterpret_cast<PurpleAccount *>(i->data));

  buttons->appendItem(
    _("Add"), sigc::mem_fun(this, &AccountWindow::addAccount));
  buttons->appendSeparator();
  buttons->appendItem(
    _("Done"), sigc::hide(sigc::mem_fun(this, &AccountWindow::close)));

  onScreenResized();
}

void AccountWindow::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::CHAT_AREA));
}

AccountWindow::BoolOption::BoolOption(
  PurpleAccount *account_, PurpleAccountOption *option_)
  : account(account_), option(option_), type(TYPE_PURPLE)
{
  g_assert(account);
  g_assert(option);

  setText(purple_account_option_get_text(option));
  setChecked(
    purple_account_get_bool(account, purple_account_option_get_setting(option),
      purple_account_option_get_default_bool(option)));

  signal_toggle.connect(sigc::mem_fun(this, &BoolOption::onToggle));
}

AccountWindow::BoolOption::BoolOption(PurpleAccount *account_, Type type_)
  : account(account_), option(NULL), type(type_)
{
  g_assert(account);

  if (type == TYPE_REMEMBER_PASSWORD) {
    setText(_("Remember password"));
    setChecked(purple_account_get_remember_password(account));
  }
  else if (type == TYPE_ENABLE_ACCOUNT) {
    setText(_("Account enabled"));
    setChecked(purple_account_get_enabled(account, PACKAGE_NAME));
  }

  signal_toggle.connect(sigc::mem_fun(this, &BoolOption::onToggle));
}

void AccountWindow::BoolOption::onToggle(
  CheckBox & /*activator*/, bool new_state)
{
  if (type == TYPE_REMEMBER_PASSWORD)
    purple_account_set_remember_password(account, new_state);
  else if (type == TYPE_ENABLE_ACCOUNT)
    purple_account_set_enabled(account, PACKAGE_NAME, new_state);
  else
    purple_account_set_bool(
      account, purple_account_option_get_setting(option), new_state);
}

AccountWindow::StringOption::StringOption(
  PurpleAccount *account_, PurpleAccountOption *option_)
  : Button(FLAG_VALUE), account(account_), option(option_), type(TYPE_PURPLE)
{
  g_assert(account);
  g_assert(option);

  initialize();
}

AccountWindow::StringOption::StringOption(PurpleAccount *account_, Type type_)
  : Button(FLAG_VALUE), account(account_), option(NULL), type(type_)
{
  g_assert(account);

  initialize();
}

void AccountWindow::StringOption::initialize()
{
  if (type == TYPE_PASSWORD)
    setText(_("Password"));
  else if (type == TYPE_ALIAS)
    setText(_("Alias"));
  else
    setText(purple_account_option_get_text(option));

  updateValue();
  signal_activate.connect(sigc::mem_fun(this, &StringOption::onActivate));
}

void AccountWindow::StringOption::updateValue()
{
  if (type == TYPE_PASSWORD) {
    setMasked(true);
    setValue(purple_account_get_password(account));
  }
  else if (type == TYPE_ALIAS)
    setValue(purple_account_get_alias(account));
  else
    setValue(purple_account_get_string(account,
      purple_account_option_get_setting(option),
      purple_account_option_get_default_string(option)));
}

void AccountWindow::StringOption::onActivate(Button & /*activator*/)
{
  CppConsUI::InputDialog *dialog =
    new CppConsUI::InputDialog(getText(), getValue());
  dialog->setMasked(isMasked());
  dialog->signal_response.connect(
    sigc::mem_fun(this, &StringOption::responseHandler));
  dialog->show();
}

void AccountWindow::StringOption::responseHandler(
  CppConsUI::InputDialog &activator, AbstractDialog::ResponseType response)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  if (type == TYPE_PASSWORD)
    purple_account_set_password(account, activator.getText());
  else if (type == TYPE_ALIAS)
    purple_account_set_alias(account, activator.getText());
  else
    purple_account_set_string(
      account, purple_account_option_get_setting(option), activator.getText());

  updateValue();
}

AccountWindow::IntegerOption::IntegerOption(
  PurpleAccount *account_, PurpleAccountOption *option_)
  : Button(FLAG_VALUE), account(account_), option(option_)
{
  g_assert(account);
  g_assert(option);

  setText(purple_account_option_get_text(option));
  updateValue();
  signal_activate.connect(sigc::mem_fun(this, &IntegerOption::onActivate));
}

void AccountWindow::IntegerOption::updateValue()
{
  setValue(
    purple_account_get_int(account, purple_account_option_get_setting(option),
      purple_account_option_get_default_int(option)));
}

void AccountWindow::IntegerOption::onActivate(Button & /*activator*/)
{
  CppConsUI::InputDialog *dialog =
    new CppConsUI::InputDialog(getText(), getValue());
  dialog->setFlags(CppConsUI::TextEntry::FLAG_NUMERIC);
  dialog->signal_response.connect(
    sigc::mem_fun(this, &IntegerOption::responseHandler));
  dialog->show();
}

void AccountWindow::IntegerOption::responseHandler(
  CppConsUI::InputDialog &activator,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  const char *text = activator.getText();
  errno = 0;
  long i = strtol(text, NULL, 10);
  if (errno == ERANGE || i > INT_MAX || i < INT_MIN)
    LOG->warning(_("Value is out of range."));
  purple_account_set_int(account, purple_account_option_get_setting(option),
    CLAMP(i, INT_MIN, INT_MAX));

  updateValue();
}

AccountWindow::StringListOption::StringListOption(
  PurpleAccount *account_, PurpleAccountOption *option_)
  : account(account_), option(option_)
{
  g_assert(account);
  g_assert(option);

  setText(purple_account_option_get_text(option));

  const char *def = purple_account_get_string(account,
    purple_account_option_get_setting(option),
    purple_account_option_get_default_list_value(option));
  for (GList *l = purple_account_option_get_list(option); l; l = l->next)
    if (l->data) {
      PurpleKeyValuePair *kvp = reinterpret_cast<PurpleKeyValuePair *>(l->data);
      addOptionPtr(kvp->key, kvp->value);
      if (kvp->value && def &&
        !strcmp(def, reinterpret_cast<const char *>(kvp->value)))
        setSelectedByDataPtr(kvp->value);
    }

  signal_selection_changed.connect(
    sigc::mem_fun(this, &StringListOption::onSelectionChanged));
}

void AccountWindow::StringListOption::onSelectionChanged(
  ComboBox & /*activator*/, int /*new_entry*/, const char * /*title*/,
  intptr_t data)
{
  purple_account_set_string(account, purple_account_option_get_setting(option),
    reinterpret_cast<const char *>(data));
}

AccountWindow::SplitOption::SplitOption(PurpleAccount *account_,
  PurpleAccountUserSplit *split_, AccountEntry *account_entry_)
  : Button(FLAG_VALUE), account(account_), split(split_),
    account_entry(account_entry_)
{
  g_assert(account);

  if (split)
    setText(purple_account_user_split_get_text(split));
  else
    setText(_("Username"));

  signal_activate.connect(sigc::mem_fun(this, &SplitOption::onActivate));
}

void AccountWindow::SplitOption::updateSplits()
{
  SplitWidgets *split_widgets = &account_entry->split_widgets;
  SplitWidgets::iterator split_widget = split_widgets->begin();
  SplitOption *widget = *split_widget;
  const char *val = widget->getValue();
  split_widget++;

  GString *username = g_string_new(val);
  PurplePluginProtocolInfo *prplinfo = PURPLE_PLUGIN_PROTOCOL_INFO(
    purple_find_prpl(purple_account_get_protocol_id(account)));

  for (GList *iter = prplinfo->user_splits;
       iter && split_widget != split_widgets->end();
       iter = iter->next, split_widget++) {
    PurpleAccountUserSplit *user_split =
      reinterpret_cast<PurpleAccountUserSplit *>(iter->data);
    widget = *split_widget;

    val = widget->getValue();
    if (!val || !*val)
      val = purple_account_user_split_get_default_value(user_split);
    g_string_append_printf(username, "%c%s",
      purple_account_user_split_get_separator(user_split), val);
  }

  purple_account_set_username(account, username->str);
  g_string_free(username, TRUE);
}

void AccountWindow::SplitOption::onActivate(Button & /*activator*/)
{
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(text, value);
  dialog->signal_response.connect(
    sigc::mem_fun(this, &SplitOption::responseHandler));
  dialog->show();
}

void AccountWindow::SplitOption::responseHandler(
  CppConsUI::InputDialog &activator,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  setValue(activator.getText());
  updateSplits();
}

AccountWindow::ProtocolOption::ProtocolOption(
  PurpleAccount *account_, AccountWindow &account_window_)
  : account_window(&account_window_), account(account_)
{
  g_assert(account);

  setText(_("Protocol"));

  for (GList *i = purple_plugins_get_protocols(); i; i = i->next)
    addOptionPtr(
      purple_plugin_get_name(reinterpret_cast<PurplePlugin *>(i->data)),
      i->data);

  const char *proto_id = purple_account_get_protocol_id(account);
  PurplePlugin *plugin = purple_plugins_find_with_id(proto_id);
  setSelectedByDataPtr(plugin);

  signal_selection_changed.connect(
    sigc::mem_fun(this, &ProtocolOption::onProtocolChanged));
}

void AccountWindow::ProtocolOption::onProtocolChanged(ComboBox & /*activator*/,
  size_t /*new_entry*/, const char * /*title*/, intptr_t data)
{
  purple_account_set_protocol_id(
    account, purple_plugin_get_id(reinterpret_cast<PurplePlugin *>(data)));

  // this deletes us so don't touch any instance variable after
  account_window->populateAccount(account);
}

AccountWindow::ColorOption::ColorOption(PurpleAccount *account_)
  : ColorPicker(CppConsUI::Curses::Color::DEFAULT,
      CppConsUI::Curses::Color::DEFAULT, _("Buddylist color:"), true),
    account(account_)
{
  g_assert(account);

  int fg = purple_account_get_ui_int(account, "centerim5",
    "buddylist-foreground-color", CppConsUI::Curses::Color::DEFAULT);
  int bg = purple_account_get_ui_int(account, "centerim5",
    "buddylist-background-color", CppConsUI::Curses::Color::DEFAULT);
  setColorPair(fg, bg);
  signal_colorpair_selected.connect(
    sigc::mem_fun(this, &ColorOption::onColorChanged));
}

void AccountWindow::ColorOption::onColorChanged(
  CppConsUI::ColorPicker & /*activator*/, int new_fg, int new_bg)
{
  purple_account_set_ui_int(
    account, "centerim5", "buddylist-foreground-color", new_fg);
  purple_account_set_ui_int(
    account, "centerim5", "buddylist-background-color", new_bg);
}

void AccountWindow::clearAccount(PurpleAccount *account, bool full)
{
  AccountEntry *account_entry = &account_entries[account];

  // move focus
  account_entry->parent->grabFocus();

  // remove the nodes from the tree
  treeview->deleteNodeChildren(account_entry->parent_reference, false);
  if (full) {
    treeview->deleteNode(account_entry->parent_reference, false);
    account_entries.erase(account);
  }
}

void AccountWindow::populateAccount(PurpleAccount *account)
{
  if (account_entries.find(account) == account_entries.end()) {
    // no entry for this account, so add one
    AccountEntry entry;
    entry.parent = NULL;
    account_entries[account] = entry;
  }
  else {
    // the account exists, so clear all data
    clearAccount(account, false);
  }

  AccountEntry *account_entry = &account_entries[account];

  if (!account_entry->parent) {
    CppConsUI::TreeView::ToggleCollapseButton *button =
      new CppConsUI::TreeView::ToggleCollapseButton;
    CppConsUI::TreeView::NodeReference parent_reference =
      treeview->appendNode(treeview->getRootNode(), *button);
    treeview->setCollapsed(parent_reference, true);
    account_entry->parent = button;
    account_entry->parent_reference = parent_reference;
  }

  char *label =
    g_strdup_printf("[%s] %s", purple_account_get_protocol_name(account),
      purple_account_get_username(account));
  account_entry->parent->setText(label);
  g_free(label);

  const char *protocol_id = purple_account_get_protocol_id(account);
  PurplePlugin *prpl = purple_find_prpl(protocol_id);

  if (!prpl) {
    // we cannot change the settings of an unknown account
    CppConsUI::Label *label =
      new CppConsUI::Label(_("Invalid account or protocol plugin not loaded."));
    treeview->appendNode(account_entry->parent_reference, *label);
  }
  else {
    PurplePluginProtocolInfo *prplinfo = PURPLE_PLUGIN_PROTOCOL_INFO(prpl);

    // protocols combobox
    ProtocolOption *combobox = new ProtocolOption(account, *this);
    CppConsUI::TreeView::NodeReference protocol_node =
      treeview->appendNode(account_entry->parent_reference, *combobox);
    combobox->grabFocus();

    /* The username must be treated in a special way because it can contain
     * multiple values (e.g. user@server:port/resource). */
    char *username = g_strdup(purple_account_get_username(account));

    for (GList *iter = g_list_last(prplinfo->user_splits); iter;
         iter = iter->prev) {
      PurpleAccountUserSplit *split =
        reinterpret_cast<PurpleAccountUserSplit *>(iter->data);

      char *s;
      if (purple_account_user_split_get_reverse(split))
        s = strrchr(username, purple_account_user_split_get_separator(split));
      else
        s = strchr(username, purple_account_user_split_get_separator(split));

      const char *value;
      if (s) {
        *s = '\0';
        value = s + 1;
      }
      else
        value = purple_account_user_split_get_default_value(split);

      // create widget for the username split and remember
      SplitOption *widget_split =
        new SplitOption(account, split, account_entry);
      widget_split->setValue(value);
      account_entry->split_widgets.push_front(widget_split);

      treeview->appendNode(account_entry->parent_reference, *widget_split);
    }

    SplitOption *widget_split = new SplitOption(account, NULL, account_entry);
    widget_split->setValue(username);
    account_entry->split_widgets.push_front(widget_split);
    treeview->insertNodeAfter(protocol_node, *widget_split);
    g_free(username);

    // password
    Widget *widget = new StringOption(account, StringOption::TYPE_PASSWORD);
    treeview->appendNode(account_entry->parent_reference, *widget);

    // remember password
    widget = new BoolOption(account, BoolOption::TYPE_REMEMBER_PASSWORD);
    treeview->appendNode(account_entry->parent_reference, *widget);

    // alias
    widget = new StringOption(account, StringOption::TYPE_ALIAS);
    treeview->appendNode(account_entry->parent_reference, *widget);

    for (GList *pref = prplinfo->protocol_options; pref; pref = pref->next) {
      PurpleAccountOption *option =
        reinterpret_cast<PurpleAccountOption *>(pref->data);
      PurplePrefType type = purple_account_option_get_type(option);

      switch (type) {
      case PURPLE_PREF_STRING:
        widget = new StringOption(account, option);
        treeview->appendNode(account_entry->parent_reference, *widget);
        break;
      case PURPLE_PREF_INT:
        widget = new IntegerOption(account, option);
        treeview->appendNode(account_entry->parent_reference, *widget);
        break;
      case PURPLE_PREF_BOOLEAN:
        widget = new BoolOption(account, option);
        treeview->appendNode(account_entry->parent_reference, *widget);
        break;
      case PURPLE_PREF_STRING_LIST:
        widget = new StringListOption(account, option);
        treeview->appendNode(account_entry->parent_reference, *widget);
        break;
      default:
        LOG->error(_("Unhandled account option type '%d'."), type);
        break;
      }
    }

    // enable/disable account
    widget = new BoolOption(account, BoolOption::TYPE_ENABLE_ACCOUNT);
    treeview->appendNode(account_entry->parent_reference, *widget);

    // coloring
    widget = new ColorOption(account);
    treeview->appendNode(account_entry->parent_reference, *widget);
  }

  // drop account
  CppConsUI::Button *drop_button = new CppConsUI::Button(_("Drop account"));
  drop_button->signal_activate.connect(
    sigc::bind(sigc::mem_fun(this, &AccountWindow::dropAccount), account));
  treeview->appendNode(account_entry->parent_reference, *drop_button);
}

void AccountWindow::addAccount(CppConsUI::Button & /*activator*/)
{
  GList *i = purple_plugins_get_protocols();
  if (!i) {
    // libpurple returns NULL if there are no protocols available
    LOG->warning(_("No protocol plugins available."));
    return;
  }

  PurpleAccount *account = purple_account_new(_("Username"),
    purple_plugin_get_id(reinterpret_cast<PurplePlugin *>(i->data)));

  /* Stop here if libpurple returned an already created account. This happens
   * when user pressed Add button twice in a row. In that case there is
   * already one "empty" account that user can edit. */
  if (account_entries.find(account) == account_entries.end()) {
    purple_accounts_add(account);

    populateAccount(account);
  }
  account_entries[account].parent->grabFocus();
}

void AccountWindow::dropAccount(
  CppConsUI::Button & /*activator*/, PurpleAccount *account)
{
  CppConsUI::MessageDialog *dialog = new CppConsUI::MessageDialog(
    _("Account deletion"), _("Are you sure you want to delete this account?"));
  dialog->signal_response.connect(sigc::bind(
    sigc::mem_fun(this, &AccountWindow::dropAccountResponseHandler), account));
  dialog->show();
}

void AccountWindow::dropAccountResponseHandler(
  CppConsUI::MessageDialog & /*activator*/,
  CppConsUI::AbstractDialog::ResponseType response, PurpleAccount *account)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  purple_accounts_remove(account);
  clearAccount(account, true);
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
