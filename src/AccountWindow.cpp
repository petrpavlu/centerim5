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

#include "AccountWindow.h"

#include "Log.h"

#include <cstring>
#include <errno.h>
#include "gettext.h"

AccountWindow::AccountWindow() : SplitDialog(0, 0, 80, 24, _("Accounts"))
{
  setColorScheme(CenterIM::SCHEME_GENERALWINDOW);

  treeview_ = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  setContainer(*treeview_);

  // Populate all defined accounts.
  for (GList *i = purple_accounts_get_all(); i != nullptr; i = i->next)
    populateAccount(reinterpret_cast<PurpleAccount *>(i->data));

  buttons_->appendItem(
    _("Add"), sigc::mem_fun(this, &AccountWindow::addAccount));
  buttons_->appendSeparator();
  buttons_->appendItem(
    _("Done"), sigc::hide(sigc::mem_fun(this, &AccountWindow::close)));

  onScreenResized();
}

void AccountWindow::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::CHAT_AREA));
}

AccountWindow::BoolOption::BoolOption(
  PurpleAccount *account, PurpleAccountOption *option)
  : account_(account), option_(option), type_(TYPE_PURPLE)
{
  g_assert(account_ != nullptr);
  g_assert(option_ != nullptr);

  setText(purple_account_option_get_text(option_));
  setChecked(purple_account_get_bool(account_,
    purple_account_option_get_setting(option_),
    purple_account_option_get_default_bool(option_)));

  signal_toggle.connect(sigc::mem_fun(this, &BoolOption::onToggle));
}

AccountWindow::BoolOption::BoolOption(PurpleAccount *account, Type type)
  : account_(account), option_(nullptr), type_(type)
{
  g_assert(account_ != nullptr);

  if (type == TYPE_REMEMBER_PASSWORD) {
    setText(_("Remember password"));
    setChecked(purple_account_get_remember_password(account_));
  }
  else if (type == TYPE_ENABLE_ACCOUNT) {
    setText(_("Account enabled"));
    setChecked(purple_account_get_enabled(account_, PACKAGE_NAME));
  }

  signal_toggle.connect(sigc::mem_fun(this, &BoolOption::onToggle));
}

void AccountWindow::BoolOption::onToggle(
  CheckBox & /*activator*/, bool new_state)
{
  if (type_ == TYPE_REMEMBER_PASSWORD)
    purple_account_set_remember_password(account_, new_state);
  else if (type_ == TYPE_ENABLE_ACCOUNT)
    purple_account_set_enabled(account_, PACKAGE_NAME, new_state);
  else
    purple_account_set_bool(
      account_, purple_account_option_get_setting(option_), new_state);
}

AccountWindow::StringOption::StringOption(
  PurpleAccount *account, PurpleAccountOption *option)
  : Button(FLAG_VALUE), account_(account), option_(option), type_(TYPE_PURPLE)
{
  g_assert(account_ != nullptr);
  g_assert(option_ != nullptr);

  initialize();
}

AccountWindow::StringOption::StringOption(PurpleAccount *account, Type type)
  : Button(FLAG_VALUE), account_(account), option_(nullptr), type_(type)
{
  g_assert(account_ != nullptr);

  initialize();
}

void AccountWindow::StringOption::initialize()
{
  if (type_ == TYPE_PASSWORD)
    setText(_("Password"));
  else if (type_ == TYPE_ALIAS)
    setText(_("Alias"));
  else
    setText(purple_account_option_get_text(option_));

  updateValue();
  signal_activate.connect(sigc::mem_fun(this, &StringOption::onActivate));
}

void AccountWindow::StringOption::updateValue()
{
  if (type_ == TYPE_PASSWORD) {
    setMasked(true);
    setValue(purple_account_get_password(account_));
  }
  else if (type_ == TYPE_ALIAS)
    setValue(purple_account_get_alias(account_));
  else
    setValue(purple_account_get_string(account_,
      purple_account_option_get_setting(option_),
      purple_account_option_get_default_string(option_)));
}

void AccountWindow::StringOption::onActivate(Button & /*activator*/)
{
  auto dialog = new CppConsUI::InputDialog(getText(), getValue());
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

  if (type_ == TYPE_PASSWORD)
    purple_account_set_password(account_, activator.getText());
  else if (type_ == TYPE_ALIAS)
    purple_account_set_alias(account_, activator.getText());
  else
    purple_account_set_string(account_,
      purple_account_option_get_setting(option_), activator.getText());

  updateValue();
}

AccountWindow::IntegerOption::IntegerOption(
  PurpleAccount *account, PurpleAccountOption *option)
  : Button(FLAG_VALUE), account_(account), option_(option)
{
  g_assert(account_ != nullptr);
  g_assert(option_ != nullptr);

  setText(purple_account_option_get_text(option_));
  updateValue();
  signal_activate.connect(sigc::mem_fun(this, &IntegerOption::onActivate));
}

void AccountWindow::IntegerOption::updateValue()
{
  setValue(
    purple_account_get_int(account_, purple_account_option_get_setting(option_),
      purple_account_option_get_default_int(option_)));
}

void AccountWindow::IntegerOption::onActivate(Button & /*activator*/)
{
  auto dialog = new CppConsUI::InputDialog(getText(), getValue());
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
  long i = strtol(text, nullptr, 10);
  if (errno == ERANGE || i > INT_MAX || i < INT_MIN)
    LOG->warning(_("Value is out of range."));
  purple_account_set_int(account_, purple_account_option_get_setting(option_),
    CLAMP(i, INT_MIN, INT_MAX));

  updateValue();
}

AccountWindow::StringListOption::StringListOption(
  PurpleAccount *account, PurpleAccountOption *option)
  : account_(account), option_(option)
{
  g_assert(account_ != nullptr);
  g_assert(option_ != nullptr);

  setText(purple_account_option_get_text(option_));

  const char *def = purple_account_get_string(account_,
    purple_account_option_get_setting(option_),
    purple_account_option_get_default_list_value(option_));
  for (GList *l = purple_account_option_get_list(option_); l != nullptr;
       l = l->next)
    if (l->data != nullptr) {
      PurpleKeyValuePair *kvp = reinterpret_cast<PurpleKeyValuePair *>(l->data);
      addOptionPtr(kvp->key, kvp->value);
      if (kvp->value != nullptr && def != nullptr &&
        std::strcmp(def, reinterpret_cast<const char *>(kvp->value)) == 0)
        setSelectedByDataPtr(kvp->value);
    }

  signal_selection_changed.connect(
    sigc::mem_fun(this, &StringListOption::onSelectionChanged));
}

void AccountWindow::StringListOption::onSelectionChanged(
  ComboBox & /*activator*/, int /*new_entry*/, const char * /*title*/,
  intptr_t data)
{
  purple_account_set_string(account_,
    purple_account_option_get_setting(option_),
    reinterpret_cast<const char *>(data));
}

AccountWindow::SplitOption::SplitOption(PurpleAccount *account,
  PurpleAccountUserSplit *split, AccountEntry *account_entry)
  : Button(FLAG_VALUE), account_(account), split_(split),
    account_entry_(account_entry)
{
  g_assert(account_ != nullptr);

  if (split_ != nullptr)
    setText(purple_account_user_split_get_text(split_));
  else
    setText(_("Username"));

  signal_activate.connect(sigc::mem_fun(this, &SplitOption::onActivate));
}

void AccountWindow::SplitOption::updateSplits()
{
  SplitWidgets *split_widgets = &account_entry_->split_widgets;
  SplitWidgets::iterator split_widget = split_widgets->begin();
  SplitOption *widget = *split_widget;
  const char *val = widget->getValue();
  ++split_widget;

  GString *username = g_string_new(val);
  PurplePluginProtocolInfo *prplinfo = PURPLE_PLUGIN_PROTOCOL_INFO(
    purple_find_prpl(purple_account_get_protocol_id(account_)));

  for (GList *iter = prplinfo->user_splits;
       iter != nullptr && split_widget != split_widgets->end();
       iter = iter->next, ++split_widget) {
    PurpleAccountUserSplit *user_split =
      reinterpret_cast<PurpleAccountUserSplit *>(iter->data);
    widget = *split_widget;

    val = widget->getValue();
    if (val == nullptr || *val == '\0')
      val = purple_account_user_split_get_default_value(user_split);
    g_string_append_printf(username, "%c%s",
      purple_account_user_split_get_separator(user_split), val);
  }

  purple_account_set_username(account_, username->str);
  g_string_free(username, TRUE);
}

void AccountWindow::SplitOption::onActivate(Button & /*activator*/)
{
  auto dialog = new CppConsUI::InputDialog(text_, value_);
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
  PurpleAccount *account, AccountWindow &account_window)
  : account_window_(&account_window), account_(account)
{
  g_assert(account_ != nullptr);

  setText(_("Protocol"));

  for (GList *i = purple_plugins_get_protocols(); i != nullptr; i = i->next)
    addOptionPtr(
      purple_plugin_get_name(reinterpret_cast<PurplePlugin *>(i->data)),
      i->data);

  const char *proto_id = purple_account_get_protocol_id(account_);
  PurplePlugin *plugin = purple_plugins_find_with_id(proto_id);
  setSelectedByDataPtr(plugin);

  signal_selection_changed.connect(
    sigc::mem_fun(this, &ProtocolOption::onProtocolChanged));
}

void AccountWindow::ProtocolOption::onProtocolChanged(ComboBox & /*activator*/,
  size_t /*new_entry*/, const char * /*title*/, intptr_t data)
{
  purple_account_set_protocol_id(
    account_, purple_plugin_get_id(reinterpret_cast<PurplePlugin *>(data)));

  // This deletes us so do not touch any member variable after it.
  account_window_->populateAccount(account_);
}

AccountWindow::ColorOption::ColorOption(PurpleAccount *account)
  : ColorPicker(CppConsUI::Curses::Color::DEFAULT,
      CppConsUI::Curses::Color::DEFAULT, _("Buddylist color:"), true),
    account_(account)
{
  g_assert(account_ != nullptr);

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
    account_, "centerim5", "buddylist-foreground-color", new_fg);
  purple_account_set_ui_int(
    account_, "centerim5", "buddylist-background-color", new_bg);
}

void AccountWindow::clearAccount(PurpleAccount *account, bool full)
{
  AccountEntry *account_entry = &account_entries_[account];

  // Move focus.
  account_entry->parent->grabFocus();

  // Remove the nodes from the tree.
  treeview_->deleteNodeChildren(account_entry->parent_reference, false);
  if (full) {
    treeview_->deleteNode(account_entry->parent_reference, false);
    account_entries_.erase(account);
  }
}

void AccountWindow::populateAccount(PurpleAccount *account)
{
  if (account_entries_.find(account) == account_entries_.end()) {
    // No entry for this account, add one.
    AccountEntry entry;
    entry.parent = nullptr;
    account_entries_[account] = entry;
  }
  else {
    // The account exists, clear all data.
    clearAccount(account, false);
  }

  AccountEntry *account_entry = &account_entries_[account];

  if (account_entry->parent == nullptr) {
    auto button = new CppConsUI::TreeView::ToggleCollapseButton;
    CppConsUI::TreeView::NodeReference parent_reference =
      treeview_->appendNode(treeview_->getRootNode(), *button);
    treeview_->setCollapsed(parent_reference, true);
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

  if (prpl == nullptr) {
    // Settings of an unknown account cannot be changed.
    auto label =
      new CppConsUI::Label(_("Invalid account or protocol plugin not loaded."));
    treeview_->appendNode(account_entry->parent_reference, *label);
  }
  else {
    PurplePluginProtocolInfo *prplinfo = PURPLE_PLUGIN_PROTOCOL_INFO(prpl);

    // Protocols combobox.
    auto combobox = new ProtocolOption(account, *this);
    CppConsUI::TreeView::NodeReference protocol_node =
      treeview_->appendNode(account_entry->parent_reference, *combobox);
    combobox->grabFocus();

    // The username must be treated in a special way because it can contain
    // multiple values (e.g. user@server:port/resource).
    char *username = g_strdup(purple_account_get_username(account));

    for (GList *iter = g_list_last(prplinfo->user_splits); iter != nullptr;
         iter = iter->prev) {
      PurpleAccountUserSplit *split =
        reinterpret_cast<PurpleAccountUserSplit *>(iter->data);

      char *s;
      if (purple_account_user_split_get_reverse(split))
        s = strrchr(username, purple_account_user_split_get_separator(split));
      else
        s = strchr(username, purple_account_user_split_get_separator(split));

      const char *value;
      if (s != nullptr) {
        *s = '\0';
        value = s + 1;
      }
      else
        value = purple_account_user_split_get_default_value(split);

      // Create widget for the username split and remember.
      auto widget_split = new SplitOption(account, split, account_entry);
      widget_split->setValue(value);
      account_entry->split_widgets.push_front(widget_split);

      treeview_->appendNode(account_entry->parent_reference, *widget_split);
    }

    auto widget_split = new SplitOption(account, nullptr, account_entry);
    widget_split->setValue(username);
    account_entry->split_widgets.push_front(widget_split);
    treeview_->insertNodeAfter(protocol_node, *widget_split);
    g_free(username);

    // Password.
    Widget *widget = new StringOption(account, StringOption::TYPE_PASSWORD);
    treeview_->appendNode(account_entry->parent_reference, *widget);

    // Remember password.
    widget = new BoolOption(account, BoolOption::TYPE_REMEMBER_PASSWORD);
    treeview_->appendNode(account_entry->parent_reference, *widget);

    // Alias.
    widget = new StringOption(account, StringOption::TYPE_ALIAS);
    treeview_->appendNode(account_entry->parent_reference, *widget);

    for (GList *pref = prplinfo->protocol_options; pref != nullptr;
         pref = pref->next) {
      PurpleAccountOption *option =
        reinterpret_cast<PurpleAccountOption *>(pref->data);
      PurplePrefType type = purple_account_option_get_type(option);

      switch (type) {
      case PURPLE_PREF_STRING:
        widget = new StringOption(account, option);
        treeview_->appendNode(account_entry->parent_reference, *widget);
        break;
      case PURPLE_PREF_INT:
        widget = new IntegerOption(account, option);
        treeview_->appendNode(account_entry->parent_reference, *widget);
        break;
      case PURPLE_PREF_BOOLEAN:
        widget = new BoolOption(account, option);
        treeview_->appendNode(account_entry->parent_reference, *widget);
        break;
      case PURPLE_PREF_STRING_LIST:
        widget = new StringListOption(account, option);
        treeview_->appendNode(account_entry->parent_reference, *widget);
        break;
      default:
        LOG->error(_("Unhandled account option type '%d'."), type);
        break;
      }
    }

    // Enable/disable account.
    widget = new BoolOption(account, BoolOption::TYPE_ENABLE_ACCOUNT);
    treeview_->appendNode(account_entry->parent_reference, *widget);

    // Color.
    widget = new ColorOption(account);
    treeview_->appendNode(account_entry->parent_reference, *widget);
  }

  // Drop account.
  auto drop_button = new CppConsUI::Button(_("Drop account"));
  drop_button->signal_activate.connect(
    sigc::bind(sigc::mem_fun(this, &AccountWindow::dropAccount), account));
  treeview_->appendNode(account_entry->parent_reference, *drop_button);
}

void AccountWindow::addAccount(CppConsUI::Button & /*activator*/)
{
  GList *i = purple_plugins_get_protocols();
  if (i == nullptr) {
    // Libpurple returns NULL if there are no protocols available.
    LOG->warning(_("No protocol plugins available."));
    return;
  }

  PurpleAccount *account = purple_account_new(_("Username"),
    purple_plugin_get_id(reinterpret_cast<PurplePlugin *>(i->data)));

  // Stop here if libpurple returned an already created account. This happens
  // when user pressed Add button twice in a row. In that case there is
  // already one "empty" account that user can edit.
  if (account_entries_.find(account) == account_entries_.end()) {
    purple_accounts_add(account);

    populateAccount(account);
  }
  account_entries_[account].parent->grabFocus();
}

void AccountWindow::dropAccount(
  CppConsUI::Button & /*activator*/, PurpleAccount *account)
{
  auto dialog = new CppConsUI::MessageDialog(
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

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
