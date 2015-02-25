/*
 * Copyright (C) 2012-2015 by CenterIM developers
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

#include "PluginWindow.h"

#include "Log.h"

#include <cppconsui/Label.h>
#include <errno.h>
#include "gettext.h"

/*
 * TODO
 * - Improve handling of PURPLE_PREF_PATH.
 * - Use these functions and use their results correctly:
 *     purple_plugin_pref_get_max_length(),
 *     purple_plugin_pref_get_masked(),
 *     purple_plugin_pref_get_format_type(),
 *     purple_plugin_pref_get_bounds().
 */

PluginWindow::PluginWindow()
: SplitDialog(0, 0, 80, 24, _("Plugins"))
{
  setColorScheme("generalwindow");

  treeview = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  setContainer(*treeview);

  // show settings for all loaded plugins
  for (GList *iter = purple_plugins_get_loaded(); iter; iter = iter->next) {
    PurplePlugin *plugin = reinterpret_cast<PurplePlugin *>(iter->data);
    if (plugin->info->type == PURPLE_PLUGIN_STANDARD &&
        !(plugin->info->flags & PURPLE_PLUGIN_FLAG_INVISIBLE))
      populatePlugin(plugin);
  }

  buttons->appendItem(_("Add"), sigc::mem_fun(this,
        &PluginWindow::addPlugin));
  buttons->appendSeparator();
  buttons->appendItem(_("Done"), sigc::hide(sigc::mem_fun(this,
          &PluginWindow::close)));

  onScreenResized();
}

PluginWindow::~PluginWindow()
{
  // destroy all allocated pref frames
  for (PluginEntries::iterator i = plugin_entries.begin();
      i != plugin_entries.end(); i++)
    if (i->second.frame)
      purple_plugin_pref_frame_destroy(i->second.frame);
}

void PluginWindow::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::CHAT_AREA));
}

PluginWindow::AddPluginWindow::AddPluginWindow()
: Window(0, 0, 80, 24, _("Add plugin"), TYPE_TOP)
{
  CppConsUI::TreeView *treeview = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  addWidget(*treeview, 0, 0);

  // populate available plugins except the ones that are already enabled
  for (GList *iter = purple_plugins_get_all(); iter; iter = iter->next) {
    PurplePlugin *plugin = reinterpret_cast<PurplePlugin *>(iter->data);
    if (purple_plugin_is_loaded(plugin) ||
        plugin->info->type != PURPLE_PLUGIN_STANDARD ||
        (plugin->info->flags & PURPLE_PLUGIN_FLAG_INVISIBLE))
      continue;

    char *text = g_strdup_printf("%s, %s\n%s", purple_plugin_get_name(plugin),
        purple_plugin_get_version(plugin), purple_plugin_get_summary(plugin));
    CppConsUI::Button *button = new CppConsUI::Button(text);
    g_free(text);

    button->signal_activate.connect(sigc::bind(sigc::mem_fun(this,
            &AddPluginWindow::onPluginButtonActivate), plugin));
    treeview->appendNode(treeview->getRootNode(), *button);
  }

  onScreenResized();
}

void PluginWindow::AddPluginWindow::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenAreaCentered(CenterIM::CHAT_AREA));
}

void PluginWindow::AddPluginWindow::onPluginButtonActivate(
    CppConsUI::Button & /*activator*/, PurplePlugin *plugin)
{
  signal_selection(*this, plugin);
  close();
}

PluginWindow::BoolOption::BoolOption(const char *name, const char *pref_)
: CheckBox(name)
{
  g_assert(name);
  g_assert(pref_);

  pref = g_strdup(pref_);
  setChecked(purple_prefs_get_bool(pref));
  signal_toggle.connect(sigc::mem_fun(this, &BoolOption::onToggle));
}

PluginWindow::BoolOption::~BoolOption()
{
  g_free(pref);
}

void PluginWindow::BoolOption::onToggle(CheckBox & /*activator*/,
    bool new_state)
{
  purple_prefs_set_bool(pref, new_state);
}

PluginWindow::StringOption::StringOption(const char *name, const char *pref_)
: Button(FLAG_VALUE, name)
{
  g_assert(name);
  g_assert(pref_);

  pref = g_strdup(pref_);
  updateValue();
  signal_activate.connect(sigc::mem_fun(this, &StringOption::onActivate));
}

PluginWindow::StringOption::~StringOption()
{
  g_free(pref);
}

void PluginWindow::StringOption::updateValue()
{
  setValue(purple_prefs_get_string(pref));
}

void PluginWindow::StringOption::onActivate(Button & /*activator*/)
{
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(getText(),
      getValue());
  dialog->signal_response.connect(sigc::mem_fun(this,
        &StringOption::responseHandler));
  dialog->show();
}

void PluginWindow::StringOption::responseHandler(
    CppConsUI::InputDialog &activator,
    AbstractDialog::ResponseType response)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  purple_prefs_set_string(pref, activator.getText());
  updateValue();
}

PluginWindow::IntegerOption::IntegerOption(const char *name,
    const char *pref_)
: Button(FLAG_VALUE, name)
{
  g_assert(name);
  g_assert(pref_);

  pref = g_strdup(pref_);
  updateValue();
  signal_activate.connect(sigc::mem_fun(this, &IntegerOption::onActivate));
}

PluginWindow::IntegerOption::~IntegerOption()
{
  g_free(pref);
}

void PluginWindow::IntegerOption::updateValue()
{
  setValue(purple_prefs_get_int(pref));
}

void PluginWindow::IntegerOption::onActivate(Button & /*activator*/)
{
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(getText(),
      getValue());
  dialog->setFlags(CppConsUI::TextEntry::FLAG_NUMERIC);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &IntegerOption::responseHandler));
  dialog->show();
}

void PluginWindow::IntegerOption::responseHandler(
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
  purple_prefs_set_int(pref, CLAMP(i, INT_MIN, INT_MAX));
  updateValue();
}

PluginWindow::PathOption::PathOption(const char *name, const char *pref_)
: Button(FLAG_VALUE, name)
{
  g_assert(name);
  g_assert(pref_);

  pref = g_strdup(pref_);
  updateValue();
  signal_activate.connect(sigc::mem_fun(this, &PathOption::onActivate));
}

PluginWindow::PathOption::~PathOption()
{
  g_free(pref);
}

void PluginWindow::PathOption::updateValue()
{
  setValue(purple_prefs_get_path(pref));
}

void PluginWindow::PathOption::onActivate(Button & /*activator*/)
{
  CppConsUI::InputDialog *dialog = new CppConsUI::InputDialog(getText(),
      getValue());
  dialog->signal_response.connect(sigc::mem_fun(this,
        &PathOption::responseHandler));
  dialog->show();
}

void PluginWindow::PathOption::responseHandler(
    CppConsUI::InputDialog &activator,
    AbstractDialog::ResponseType response)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  purple_prefs_set_path(pref, activator.getText());
  updateValue();
}

void PluginWindow::clearPlugin(PurplePlugin *plugin)
{
  PluginEntry *plugin_entry = &plugin_entries[plugin];
  treeview->deleteNode(plugin_entry->parent_reference, false);
  if (plugin_entry->frame)
    purple_plugin_pref_frame_destroy(plugin_entry->frame);
  plugin_entries.erase(plugin);
}

void PluginWindow::populatePlugin(PurplePlugin *plugin)
{
  // show settings for a single plugin
  CppConsUI::Button *button = new CppConsUI::TreeView::ToggleCollapseButton(
      purple_plugin_get_name(plugin));
  CppConsUI::TreeView::NodeReference parent_reference
    = treeview->appendNode(treeview->getRootNode(), *button);
  treeview->setCollapsed(parent_reference, true);

  // record this plugin in plugin_entries
  PluginEntry entry;
  entry.parent = button;
  entry.parent_reference = parent_reference;
  entry.frame = NULL;
  plugin_entries[plugin] = entry;

  if (PURPLE_PLUGIN_HAS_PREF_FRAME(plugin)) {
    PurplePluginUiInfo *ui_info = PURPLE_PLUGIN_UI_INFO(plugin);
    PurplePluginPrefFrame *frame = ui_info->get_plugin_pref_frame(plugin);

    // note this pref frame
    plugin_entries[plugin].frame = frame;

    // group node (set to an "invalid" value)
    CppConsUI::TreeView::NodeReference group = treeview->getRootNode();

    for (GList *iter = purple_plugin_pref_frame_get_prefs(frame); iter;
        iter = iter->next) {
      PurplePluginPref *pref
        = reinterpret_cast<PurplePluginPref *>(iter->data);
      PurplePluginPrefType type = purple_plugin_pref_get_type(pref);
      const char *label = purple_plugin_pref_get_label(pref);
      const char *name = purple_plugin_pref_get_name(pref);
      if (!name) {
        if (!label)
          continue;

        if (type == PURPLE_PLUGIN_PREF_INFO) {
          // no value label
          treeview->appendNode(parent_reference,
              *(new CppConsUI::Label(label)));
        }
        else {
          // a new group
          group = treeview->appendNode(parent_reference,
              *(new CppConsUI::TreeView::ToggleCollapseButton(label)));
        }
        continue;
      }

      PurplePrefType value_type = purple_prefs_get_type(name);
      CppConsUI::Widget *pref_widget;
      if (type == PURPLE_PLUGIN_PREF_CHOICE) {
        CppConsUI::ComboBox *combo = new CppConsUI::ComboBox(label);
        pref_widget = combo;

        // add possible options
        for (GList *pref_iter = purple_plugin_pref_get_choices(pref);
            pref_iter && pref_iter->next; pref_iter = pref_iter->next->next)
          combo->addOptionPtr(reinterpret_cast<const char *>(pref_iter->data),
              pref_iter->next->data);

        // set default value
        intptr_t current_value;
        if (value_type == PURPLE_PREF_BOOLEAN)
          current_value = purple_prefs_get_bool(name);
        else if (value_type == PURPLE_PREF_INT)
          current_value = purple_prefs_get_int(name);
        else if (value_type == PURPLE_PREF_STRING)
          current_value = reinterpret_cast<intptr_t>(
              purple_prefs_get_string(name));
        else if (value_type == PURPLE_PREF_PATH)
          current_value = reinterpret_cast<intptr_t>(
              purple_prefs_get_path(name));
        else {
          LOG->error(_("Unhandled plugin preference type '%d'."), value_type);
          continue;
        }
        combo->setSelectedByData(current_value);
      }
      else {
        if (value_type == PURPLE_PREF_BOOLEAN)
          pref_widget = new BoolOption(label, name);
        else if (value_type == PURPLE_PREF_INT)
          pref_widget = new IntegerOption(label, name);
        else if (value_type == PURPLE_PREF_STRING)
          pref_widget = new StringOption(label, name);
        else if (value_type == PURPLE_PREF_PATH)
          pref_widget = new PathOption(label, name);
        else {
          LOG->error(_("Unhandled plugin preference type '%d'."), value_type);
          continue;
        }
      }

      if (group == treeview->getRootNode()) {
        // no group has been created, so create one
        group = treeview->appendNode(parent_reference,
            *(new CppConsUI::TreeView::ToggleCollapseButton(
                _("Preferences"))));
      }
      treeview->appendNode(group, *pref_widget);
    }
  }

  // disable plugin
  CppConsUI::Button *disable_button
    = new CppConsUI::Button(_("Disable plugin"));
  disable_button->signal_activate.connect(sigc::bind(sigc::mem_fun(this,
          &PluginWindow::disablePlugin), plugin));
  treeview->appendNode(parent_reference, *disable_button);
}

void PluginWindow::addPlugin(CppConsUI::Button & /*activator*/)
{
  // show a window for the user to select a plugin that he wants to add
  AddPluginWindow *win = new AddPluginWindow;
  win->signal_selection.connect(sigc::mem_fun(this,
        &PluginWindow::onAddPluginSelection));
  win->show();
}

void PluginWindow::onAddPluginSelection(AddPluginWindow & /*activator*/,
    PurplePlugin *plugin)
{
  // the user has selected a plugin that he wants to add, so do it
  if (!purple_plugin_load(plugin)) {
    LOG->error(_("Error loading plugin '%s'."),
        purple_plugin_get_name(plugin));
    return;
  }

  purple_plugins_save_loaded(CONF_PLUGINS_SAVE_PREF);

  // populate this plugin
  populatePlugin(plugin);

  // focus the button for this plugin
  PluginEntries::iterator i = plugin_entries.find(plugin);
  g_assert(i != plugin_entries.end());
  g_assert(i->second.parent);
  i->second.parent->grabFocus();
}

void PluginWindow::disablePlugin(CppConsUI::Button & /*activator*/,
    PurplePlugin *plugin)
{
  CppConsUI::MessageDialog *dialog = new CppConsUI::MessageDialog(
      _("Disable plugin"),
      _("Are you sure you want to disable this plugin?"));
  dialog->signal_response.connect(sigc::bind(sigc::mem_fun(this,
          &PluginWindow::disablePluginResponseHandler), plugin));
  dialog->show();
}

void PluginWindow::disablePluginResponseHandler(
    CppConsUI::MessageDialog & /*activator*/,
    CppConsUI::AbstractDialog::ResponseType response, PurplePlugin *plugin)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  if (!purple_plugin_unload(plugin)) {
    LOG->error(_("Error unloading plugin '%s'. The plugin could not be "
          "unloaded now, but will be disabled at the next startup."),
        purple_plugin_get_name(plugin));

    purple_plugin_disable(plugin);
  }
  else
    clearPlugin(plugin);
  purple_plugins_save_loaded(CONF_PLUGINS_SAVE_PREF);
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
