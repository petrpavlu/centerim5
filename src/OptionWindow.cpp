/*
 * Copyright (C) 2011 by CenterIM developers
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

#include "OptionWindow.h"

#include "BuddyList.h"
#include "CenterIM.h"
#include "Log.h"

#include <cppconsui/KeyConfig.h>
#include <cppconsui/TreeView.h>
#include <errno.h>
#include "gettext.h"

OptionWindow::OptionWindow()
: SplitDialog(0, 0, 80, 24, _("Config options"))
{
  SetColorScheme("generalwindow");

  CppConsUI::TreeView *tree = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  SetContainer(*tree);

  ChoiceOption *c;
  CppConsUI::TreeView::NodeReference parent;
  parent = tree->AppendNode(tree->GetRootNode(),
      *(new CppConsUI::TreeView::ToggleCollapseButton(_("Buddy list"))));
  tree->AppendNode(parent, *(new BooleanOption(_("Show empty groups"),
          CONF_PREFIX "/blist/show_empty_groups")));
  tree->AppendNode(parent, *(new BooleanOption(_("Show offline buddies"),
          CONF_PREFIX "/blist/show_offline_buddies")));

  parent = tree->AppendNode(tree->GetRootNode(),
      *(new CppConsUI::TreeView::ToggleCollapseButton(
          _("Dimensions (percentage)"))));
  tree->AppendNode(parent, *(new IntegerOption(_("Buddy list window width"),
          CONF_PREFIX "/dimensions/buddylist_width")));
  tree->AppendNode(parent, *(new IntegerOption(_("Log window height"),
          CONF_PREFIX "/dimensions/log_height")));

  parent = tree->AppendNode(tree->GetRootNode(),
      *(new CppConsUI::TreeView::ToggleCollapseButton(
          _("Idle settings"))));
  tree->AppendNode(parent, *(new BooleanOption(
          _("Change to away status when idle"),
          "/purple/away/away_when_idle")));
  tree->AppendNode(parent, *(new IntegerOption(
          _("Minutes before becoming idle"),
          "/purple/away/mins_before_away")));
  c = new ChoiceOption(_("Report idle time"),
      "/purple/away/idle_reporting");
  c->AddOption(_("Never"), "none");
  c->AddOption(_("From last sent message"), "purple");
  c->AddOption(_("Based on keyboard"), "system");
  tree->AppendNode(parent, *c);

  parent = tree->AppendNode(tree->GetRootNode(),
      *(new CppConsUI::TreeView::ToggleCollapseButton(_("Logging"))));
#define ADD_DEBUG_OPTIONS()                \
do {                                       \
  c->AddOption(_("None"), "none");         \
  c->AddOption(_("Error"), "error");       \
  c->AddOption(_("Critical"), "critical"); \
  c->AddOption(_("Warning"), "warning");   \
  c->AddOption(_("Message"), "message");   \
  c->AddOption(_("Info"), "info");         \
  c->AddOption(_("Debug"), "debug");       \
} while (0)
  c = new ChoiceOption(_("CIM log level"),
    CONF_PREFIX "/log/log_level_cim");
  ADD_DEBUG_OPTIONS();
  tree->AppendNode(parent, *c);

  c = new ChoiceOption(_("CppConsUI log level"),
      CONF_PREFIX "/log/log_level_cppconsui");
  ADD_DEBUG_OPTIONS();
  tree->AppendNode(parent, *c);

  c = new ChoiceOption(_("Purple log level"),
      CONF_PREFIX "/log/log_level_purple");
  ADD_DEBUG_OPTIONS();
  tree->AppendNode(parent, *c);

  c = new ChoiceOption(_("GLib log level"),
      CONF_PREFIX "/log/log_level_glib");
  ADD_DEBUG_OPTIONS();
  tree->AppendNode(parent, *c);
#undef ADD_DEBUG_OPTIONS

  CppConsUI::Button *b = new CppConsUI::Button(AUTOSIZE, 1,
      _("Reload keybinding file"));
  b->signal_activate.connect(sigc::hide_return(sigc::hide(sigc::mem_fun(KEYCONFIG,
            &CppConsUI::KeyConfig::Reconfig))));
  tree->AppendNode(tree->GetRootNode(), *b);

  buttons->AppendItem(_("Done"), sigc::hide(sigc::mem_fun(this,
          &OptionWindow::Close)));
}

void OptionWindow::ScreenResized()
{
  MoveResizeRect(CENTERIM->GetScreenAreaSize(CenterIM::CHAT_AREA));
}

OptionWindow::BooleanOption::BooleanOption(const char *text,
    const char *config)
: CheckBox(text)
{
  g_assert(text);
  g_assert(config);

  pref = g_strdup(config);
  SetState(purple_prefs_get_bool(config));
  signal_toggle.connect(sigc::mem_fun(this, &BooleanOption::OnToggle));
}

OptionWindow::BooleanOption::~BooleanOption()
{
  g_free(pref);
}

void OptionWindow::BooleanOption::OnToggle(CheckBox& activator,
    bool new_state)
{
  purple_prefs_set_bool(pref, new_state);
}

OptionWindow::StringOption::StringOption(const char *text,
    const char *config)
: Button(TYPE_DOUBLE, text)
{
  g_assert(text);
  g_assert(config);

  pref = g_strdup(config);
  SetValue(purple_prefs_get_string(config));
  signal_activate.connect(sigc::mem_fun(this, &StringOption::OnActivate));
}

OptionWindow::StringOption::~StringOption()
{
  g_free(pref);
}

void OptionWindow::StringOption::OnActivate(Button& activator)
{
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(GetText(),
      GetValue());
  dialog->signal_response.connect(sigc::mem_fun(this,
        &StringOption::ResponseHandler));
  dialog->Show();
}

void OptionWindow::StringOption::ResponseHandler(
    CppConsUI::InputDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_OK:
      purple_prefs_set_string(pref, activator.GetText());
      SetValue(purple_prefs_get_string(pref));
      break;
    default:
      break;
  }
}

OptionWindow::IntegerOption::IntegerOption(const char *text,
    const char *config)
: Button(TYPE_DOUBLE, text)
{
  g_assert(text);
  g_assert(config);

  pref = g_strdup(config);
  SetValue(purple_prefs_get_int(config));
  signal_activate.connect(sigc::mem_fun(this, &IntegerOption::OnActivate));
}

OptionWindow::IntegerOption::~IntegerOption()
{
  g_free(pref);
}

void OptionWindow::IntegerOption::OnActivate(Button& activator)
{
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(GetText(),
      GetValue());
  dialog->SetFlags(CppConsUI::TextEntry::FLAG_NUMERIC);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &IntegerOption::ResponseHandler));
  dialog->Show();
}

void OptionWindow::IntegerOption::ResponseHandler(
    CppConsUI::InputDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  const char *text;
  long int i;

  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_OK:
      text = activator.GetText();
      errno = 0;
      i = strtol(text, NULL, 10);
      if (errno == ERANGE)
        LOG->Warning(_("Value is out of range."));

      purple_prefs_set_int(pref, i);
      SetValue(purple_prefs_get_int(pref));
      break;
    default:
      break;
  }
}

OptionWindow::ChoiceOption::ChoiceOption(const char *text,
    const char *config)
: ComboBox(text)
{
  g_assert(text);
  g_assert(config);

  pref = g_strdup(config);
  signal_selection_changed.connect(sigc::mem_fun(this,
        &ChoiceOption::OnSelectionChanged));
}

OptionWindow::ChoiceOption::~ChoiceOption()
{
  for (ComboBoxEntries::iterator i = options.begin(); i != options.end(); i++)
    g_free(reinterpret_cast<char*>(i->data));

  g_free(pref);
}

void OptionWindow::ChoiceOption::AddOption(const char *title,
    const char *value)
{
  g_assert(title);
  g_assert(value);

  int item = AddOptionPtr(title, g_strdup(value));
  if (!g_ascii_strcasecmp(purple_prefs_get_string(pref), value))
    SetSelected(item);
}

void OptionWindow::ChoiceOption::OnSelectionChanged(ComboBox& activator,
    int new_entry, const char *title, intptr_t data)
{
  purple_prefs_set_string(pref, reinterpret_cast<const char*>(data));
}
