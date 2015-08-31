// Copyright (C) 2011-2015 Petr Pavlu <setup@dagobah.cz>
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

#include "OptionWindow.h"

#include "Log.h"

#include <cppconsui/TreeView.h>
#include <errno.h>
#include "gettext.h"

OptionWindow::OptionWindow() : SplitDialog(0, 0, 80, 24, _("Config options"))
{
  setColorScheme(CenterIM::SCHEME_GENERALWINDOW);

  auto treeview = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  setContainer(*treeview);

  ChoiceOption *c;
  CppConsUI::TreeView::NodeReference parent;
  parent = treeview->appendNode(treeview->getRootNode(),
    *(new CppConsUI::TreeView::ToggleCollapseButton(_("Buddy list"))));
  treeview->setCollapsed(parent, true);
  treeview->appendNode(parent, *(new BooleanOption(_("Show empty groups"),
                                 CONF_PREFIX "/blist/show_empty_groups")));
  treeview->appendNode(parent, *(new BooleanOption(_("Show offline buddies"),
                                 CONF_PREFIX "/blist/show_offline_buddies")));
  c = new ChoiceOption(_("List mode"), CONF_PREFIX "/blist/list_mode");
  c->addOption(_("Normal"), "normal");
  c->addOption(_("Flat"), "flat");
  treeview->appendNode(parent, *c);
  c = new ChoiceOption(
    _("Group sort mode"), CONF_PREFIX "/blist/group_sort_mode");
  c->addOption(_("Manually"), "user");
  c->addOption(_("By name"), "name");
  treeview->appendNode(parent, *c);
  c = new ChoiceOption(
    _("Buddy sort mode"), CONF_PREFIX "/blist/buddy_sort_mode");
  c->addOption(_("By name"), "name");
  c->addOption(_("By status"), "status");
  c->addOption(_("By activity"), "activity");
  treeview->appendNode(parent, *c);
  c = new ChoiceOption(
    _("Colorization mode"), CONF_PREFIX "/blist/colorization_mode");
  c->addOption(_("None"), "none");
  c->addOption(_("By status"), "status");
  c->addOption(_("By account"), "account");
  treeview->appendNode(parent, *c);

  parent = treeview->appendNode(treeview->getRootNode(),
    *(new CppConsUI::TreeView::ToggleCollapseButton(_("Dimensions"))));
  treeview->setCollapsed(parent, true);
  treeview->appendNode(
    parent, *(new IntegerOption(_("Buddy list window width"),
              CONF_PREFIX "/dimensions/buddylist_width",
              sigc::mem_fun(this, &OptionWindow::getPercentUnit))));
  treeview->appendNode(
    parent, *(new IntegerOption(_("Log window height"),
              CONF_PREFIX "/dimensions/log_height",
              sigc::mem_fun(this, &OptionWindow::getPercentUnit))));
  treeview->appendNode(parent, *(new BooleanOption(_("Show header"),
                                 CONF_PREFIX "/dimensions/show_header")));
  treeview->appendNode(parent, *(new BooleanOption(_("Show footer"),
                                 CONF_PREFIX "/dimensions/show_footer")));

  parent = treeview->appendNode(treeview->getRootNode(),
    *(new CppConsUI::TreeView::ToggleCollapseButton(_("Idle settings"))));
  treeview->setCollapsed(parent, true);
  treeview->appendNode(
    parent, *(new BooleanOption(_("Change to away status when idle"),
              "/purple/away/away_when_idle")));
  treeview->appendNode(
    parent, *(new IntegerOption(_("Time before becoming idle"),
              "/purple/away/mins_before_away",
              sigc::mem_fun(this, &OptionWindow::getMinUnit))));
  c = new ChoiceOption(_("Report idle time"), "/purple/away/idle_reporting");
  c->addOption(_("Never"), "none");
  c->addOption(_("From last sent message"), "purple");
  c->addOption(_("Based on keyboard activity"), "system");
  treeview->appendNode(parent, *c);

  parent = treeview->appendNode(treeview->getRootNode(),
    *(new CppConsUI::TreeView::ToggleCollapseButton(_("Conversations"))));
  treeview->setCollapsed(parent, true);
  treeview->appendNode(parent, *(new BooleanOption(_("Beep on new message"),
                                 CONF_PREFIX "/chat/beep_on_msg")));
  treeview->appendNode(
    parent, *(new BooleanOption(_("Send typing notification"),
              "/purple/conversations/im/send_typing")));

  parent = treeview->appendNode(treeview->getRootNode(),
    *(new CppConsUI::TreeView::ToggleCollapseButton(_("System logging"))));
  treeview->setCollapsed(parent, true);
#define ADD_DEBUG_OPTIONS()                                                    \
  do {                                                                         \
    c->addOption(_("None"), "none");                                           \
    c->addOption(_("Error"), "error");                                         \
    c->addOption(_("Critical"), "critical");                                   \
    c->addOption(_("Warning"), "warning");                                     \
    c->addOption(_("Message"), "message");                                     \
    c->addOption(_("Info"), "info");                                           \
    c->addOption(_("Debug"), "debug");                                         \
  } while (0)
  c = new ChoiceOption(_("CIM log level"), CONF_PREFIX "/log/log_level_cim");
  ADD_DEBUG_OPTIONS();
  treeview->appendNode(parent, *c);

  c = new ChoiceOption(
    _("Purple log level"), CONF_PREFIX "/log/log_level_purple");
  ADD_DEBUG_OPTIONS();
  treeview->appendNode(parent, *c);

  c = new ChoiceOption(_("GLib log level"), CONF_PREFIX "/log/log_level_glib");
  ADD_DEBUG_OPTIONS();
  treeview->appendNode(parent, *c);
#undef ADD_DEBUG_OPTIONS

  parent = treeview->appendNode(treeview->getRootNode(),
    *(new CppConsUI::TreeView::ToggleCollapseButton(_("Libpurple logging"))));
  treeview->setCollapsed(parent, true);
  c = new ChoiceOption(_("Log format"), "/purple/logging/format");
  GList *opts = purple_log_logger_get_options();
  for (GList *o = opts; o != nullptr; o = o->next) {
    const char *human = reinterpret_cast<const char *>(o->data);
    o = o->next;
    g_assert(o != nullptr);
    const char *value = reinterpret_cast<const char *>(o->data);
    c->addOption(human, value);
  }
  g_list_free(opts);
  treeview->appendNode(parent, *c);
  treeview->appendNode(
    parent, *(new BooleanOption(
              _("Log all instant messages"), "/purple/logging/log_ims")));
  treeview->appendNode(parent,
    *(new BooleanOption(_("Log all chats"), "/purple/logging/log_chats")));
  treeview->appendNode(
    parent, *(new BooleanOption(_("Log all status changes to system log"),
              "/purple/logging/log_system")));

  CppConsUI::Button *b;
  parent = treeview->appendNode(treeview->getRootNode(),
    *(new CppConsUI::TreeView::ToggleCollapseButton(_("Config files"))));
  treeview->setCollapsed(parent, true);
  b = new CppConsUI::Button(AUTOSIZE, 1, _("Reload key bindings"));
  b->signal_activate.connect(
    sigc::mem_fun(this, &OptionWindow::reloadKeyBindings));
  treeview->appendNode(parent, *b);
  b = new CppConsUI::Button(AUTOSIZE, 1, _("Reload color schemes"));
  b->signal_activate.connect(
    sigc::mem_fun(this, &OptionWindow::reloadColorSchemes));
  treeview->appendNode(parent, *b);

  buttons_->appendItem(
    _("Done"), sigc::hide(sigc::mem_fun(this, &OptionWindow::close)));

  onScreenResized();
}

void OptionWindow::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::CHAT_AREA));
}

OptionWindow::BooleanOption::BooleanOption(const char *text, const char *config)
  : CheckBox(text)
{
  g_assert(text != nullptr);
  g_assert(config != nullptr);

  pref_ = g_strdup(config);
  setChecked(purple_prefs_get_bool(pref_));
  signal_toggle.connect(sigc::mem_fun(this, &BooleanOption::onToggle));
}

OptionWindow::BooleanOption::~BooleanOption()
{
  g_free(pref_);
}

void OptionWindow::BooleanOption::onToggle(
  CheckBox & /*activator*/, bool new_state)
{
  purple_prefs_set_bool(pref_, new_state);
}

OptionWindow::StringOption::StringOption(const char *text, const char *config)
  : Button(FLAG_VALUE, text)
{
  g_assert(text != nullptr);
  g_assert(config != nullptr);

  pref_ = g_strdup(config);
  setValue(purple_prefs_get_string(pref_));
  signal_activate.connect(sigc::mem_fun(this, &StringOption::onActivate));
}

OptionWindow::StringOption::~StringOption()
{
  g_free(pref_);
}

void OptionWindow::StringOption::onActivate(Button & /*activator*/)
{
  auto dialog = new CppConsUI::InputDialog(getText(), getValue());
  dialog->signal_response.connect(
    sigc::mem_fun(this, &StringOption::responseHandler));
  dialog->show();
}

void OptionWindow::StringOption::responseHandler(
  CppConsUI::InputDialog &activator,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  purple_prefs_set_string(pref_, activator.getText());
  setValue(purple_prefs_get_string(pref_));
}

OptionWindow::IntegerOption::IntegerOption(const char *text, const char *config)
  : Button(FLAG_VALUE, text), unit_(false)
{
  g_assert(text != nullptr);
  g_assert(config != nullptr);

  pref_ = g_strdup(config);
  setValue(purple_prefs_get_int(pref_));
  signal_activate.connect(sigc::mem_fun(this, &IntegerOption::onActivate));
}

OptionWindow::IntegerOption::IntegerOption(
  const char *text, const char *config, sigc::slot<const char *, int> unit_fun)
  : Button(FLAG_VALUE | FLAG_UNIT, text), unit_(true), unit_fun_(unit_fun)
{
  g_assert(text != nullptr);
  g_assert(config != nullptr);

  pref_ = g_strdup(config);
  int val = purple_prefs_get_int(pref_);
  setValue(val);
  setUnit(unit_fun_(val));
  signal_activate.connect(sigc::mem_fun(this, &IntegerOption::onActivate));
}

OptionWindow::IntegerOption::~IntegerOption()
{
  g_free(pref_);
}

void OptionWindow::IntegerOption::onActivate(Button & /*activator*/)
{
  auto dialog = new CppConsUI::InputDialog(getText(), getValue());
  dialog->setFlags(CppConsUI::TextEntry::FLAG_NUMERIC);
  dialog->signal_response.connect(
    sigc::mem_fun(this, &IntegerOption::responseHandler));
  dialog->show();
}

void OptionWindow::IntegerOption::responseHandler(
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

  purple_prefs_set_int(pref_, CLAMP(i, INT_MIN, INT_MAX));
  int val = purple_prefs_get_int(pref_);
  setValue(purple_prefs_get_int(pref_));
  if (unit_)
    setUnit(unit_fun_(val));
}

OptionWindow::ChoiceOption::ChoiceOption(const char *text, const char *config)
  : ComboBox(text)
{
  g_assert(text != nullptr);
  g_assert(config != nullptr);

  pref_ = g_strdup(config);
  signal_selection_changed.connect(
    sigc::mem_fun(this, &ChoiceOption::onSelectionChanged));
}

OptionWindow::ChoiceOption::~ChoiceOption()
{
  for (ComboBoxEntry &entry : options_)
    g_free(reinterpret_cast<char *>(entry.data));

  g_free(pref_);
}

void OptionWindow::ChoiceOption::addOption(const char *title, const char *value)
{
  g_assert(title != nullptr);
  g_assert(value != nullptr);

  int item = addOptionPtr(title, g_strdup(value));
  if (g_ascii_strcasecmp(purple_prefs_get_string(pref_), value) == 0)
    setSelected(item);
}

void OptionWindow::ChoiceOption::onSelectionChanged(ComboBox & /*activator*/,
  int /*new_entry*/, const char * /*title*/, intptr_t data)
{
  purple_prefs_set_string(pref_, reinterpret_cast<const char *>(data));
}

const char *OptionWindow::getPercentUnit(int /*i*/) const
{
  return "%";
}

const char *OptionWindow::getMinUnit(int i) const
{
  return ngettext("minute", "minutes", i);
}

void OptionWindow::reloadKeyBindings(CppConsUI::Button & /*activator*/) const
{
  if (CENTERIM->loadKeyConfig())
    LOG->message(_("Keybinding file was successfully reloaded."));
}

void OptionWindow::reloadColorSchemes(CppConsUI::Button & /*activator*/) const
{
  if (CENTERIM->loadColorSchemeConfig())
    LOG->message(_("Colorscheme file was successfully reloaded."));
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
