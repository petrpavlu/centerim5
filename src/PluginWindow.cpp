// Copyright (C) 2012-2015 Petr Pavlu <setup@dagobah.cz>
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

#include "PluginWindow.h"

#include "Log.h"
#include "Utils.h"

#include <cppconsui/Label.h>
#include "gettext.h"

// TODO
// - Improve handling of PURPLE_PREF_PATH.
// - Use these functions and use their results correctly:
//     purple_plugin_pref_get_max_length(),
//     purple_plugin_pref_get_masked(),
//     purple_plugin_pref_get_format_type(),
//     purple_plugin_pref_get_bounds().

PluginWindow::PluginWindow() : SplitDialog(0, 0, 80, 24, _("Plugins"))
{
  setColorScheme(CenterIM::SCHEME_GENERALWINDOW);

  treeview_ = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  setContainer(*treeview_);

  // Show settings for all loaded plugins.
  for (GList *iter = purple_plugins_get_loaded(); iter != nullptr;
       iter = iter->next) {
    PurplePlugin *plugin = reinterpret_cast<PurplePlugin *>(iter->data);
    if (plugin->info->type == PURPLE_PLUGIN_STANDARD &&
      !(plugin->info->flags & PURPLE_PLUGIN_FLAG_INVISIBLE))
      populatePlugin(plugin);
  }

  buttons_->appendItem(_("Add"), sigc::mem_fun(this, &PluginWindow::addPlugin));
  buttons_->appendSeparator();
  buttons_->appendItem(
    _("Done"), sigc::hide(sigc::mem_fun(this, &PluginWindow::close)));

  onScreenResized();
}

PluginWindow::~PluginWindow()
{
  // Destroy all allocated pref frames.
  for (PluginEntries::value_type &entry : plugin_entries_)
    if (entry.second.frame != nullptr)
      purple_plugin_pref_frame_destroy(entry.second.frame);
}

void PluginWindow::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::CHAT_AREA));
}

PluginWindow::AddPluginWindow::AddPluginWindow()
  : Window(0, 0, 80, 24, _("Add plugin"), TYPE_TOP)
{
  auto treeview = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  addWidget(*treeview, 0, 0);

  // Populate available plugins except the ones that are already enabled.
  for (GList *iter = purple_plugins_get_all(); iter != nullptr;
       iter = iter->next) {
    PurplePlugin *plugin = reinterpret_cast<PurplePlugin *>(iter->data);
    if (purple_plugin_is_loaded(plugin) ||
      plugin->info->type != PURPLE_PLUGIN_STANDARD ||
      (plugin->info->flags & PURPLE_PLUGIN_FLAG_INVISIBLE))
      continue;

    char *text = g_strdup_printf("%s, %s\n%s", purple_plugin_get_name(plugin),
      purple_plugin_get_version(plugin), purple_plugin_get_summary(plugin));
    auto button = new CppConsUI::Button(text);
    g_free(text);

    button->signal_activate.connect(sigc::bind(
      sigc::mem_fun(this, &AddPluginWindow::onPluginButtonActivate), plugin));
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

PluginWindow::BoolOption::BoolOption(const char *name, const char *pref)
  : CheckBox(name)
{
  g_assert(name != nullptr);
  g_assert(pref != nullptr);

  pref_ = g_strdup(pref);
  setChecked(purple_prefs_get_bool(pref_));
  signal_toggle.connect(sigc::mem_fun(this, &BoolOption::onToggle));
}

PluginWindow::BoolOption::~BoolOption()
{
  g_free(pref_);
}

void PluginWindow::BoolOption::onToggle(
  CheckBox & /*activator*/, bool new_state)
{
  purple_prefs_set_bool(pref_, new_state);
}

PluginWindow::StringOption::StringOption(const char *name, const char *pref)
  : Button(FLAG_VALUE, name)
{
  g_assert(name != nullptr);
  g_assert(pref != nullptr);

  pref_ = g_strdup(pref);
  updateValue();
  signal_activate.connect(sigc::mem_fun(this, &StringOption::onActivate));
}

PluginWindow::StringOption::~StringOption()
{
  g_free(pref_);
}

void PluginWindow::StringOption::updateValue()
{
  setValue(purple_prefs_get_string(pref_));
}

void PluginWindow::StringOption::onActivate(Button & /*activator*/)
{
  auto dialog = new CppConsUI::InputDialog(getText(), getValue());
  dialog->signal_response.connect(
    sigc::mem_fun(this, &StringOption::responseHandler));
  dialog->show();
}

void PluginWindow::StringOption::responseHandler(
  CppConsUI::InputDialog &activator, AbstractDialog::ResponseType response)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  purple_prefs_set_string(pref_, activator.getText());
  updateValue();
}

PluginWindow::IntegerOption::IntegerOption(const char *name, const char *pref)
  : Button(FLAG_VALUE, name)
{
  g_assert(name != nullptr);
  g_assert(pref != nullptr);

  pref_ = g_strdup(pref);
  updateValue();
  signal_activate.connect(sigc::mem_fun(this, &IntegerOption::onActivate));
}

PluginWindow::IntegerOption::~IntegerOption()
{
  g_free(pref_);
}

void PluginWindow::IntegerOption::updateValue()
{
  setValue(purple_prefs_get_int(pref_));
}

void PluginWindow::IntegerOption::onActivate(Button & /*activator*/)
{
  auto dialog = new CppConsUI::InputDialog(getText(), getValue());
  dialog->setFlags(CppConsUI::TextEntry::FLAG_NUMERIC);
  dialog->signal_response.connect(
    sigc::mem_fun(this, &IntegerOption::responseHandler));
  dialog->show();
}

void PluginWindow::IntegerOption::responseHandler(
  CppConsUI::InputDialog &activator,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  long num;
  if (!Utils::stringToNumber(activator.getText(), INT_MIN, INT_MAX, &num))
    return;
  purple_prefs_set_int(pref_, num);

  updateValue();
}

PluginWindow::PathOption::PathOption(const char *name, const char *pref)
  : Button(FLAG_VALUE, name)
{
  g_assert(name != nullptr);
  g_assert(pref != nullptr);

  pref_ = g_strdup(pref);
  updateValue();
  signal_activate.connect(sigc::mem_fun(this, &PathOption::onActivate));
}

PluginWindow::PathOption::~PathOption()
{
  g_free(pref_);
}

void PluginWindow::PathOption::updateValue()
{
  setValue(purple_prefs_get_path(pref_));
}

void PluginWindow::PathOption::onActivate(Button & /*activator*/)
{
  auto dialog = new CppConsUI::InputDialog(getText(), getValue());
  dialog->signal_response.connect(
    sigc::mem_fun(this, &PathOption::responseHandler));
  dialog->show();
}

void PluginWindow::PathOption::responseHandler(
  CppConsUI::InputDialog &activator, AbstractDialog::ResponseType response)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  purple_prefs_set_path(pref_, activator.getText());
  updateValue();
}

void PluginWindow::clearPlugin(PurplePlugin *plugin)
{
  PluginEntry *plugin_entry = &plugin_entries_[plugin];
  treeview_->deleteNode(plugin_entry->parent_reference, false);
  if (plugin_entry->frame != nullptr)
    purple_plugin_pref_frame_destroy(plugin_entry->frame);
  plugin_entries_.erase(plugin);
}

void PluginWindow::populatePlugin(PurplePlugin *plugin)
{
  // Show settings for a single plugin.
  CppConsUI::Button *button = new CppConsUI::TreeView::ToggleCollapseButton(
    purple_plugin_get_name(plugin));
  CppConsUI::TreeView::NodeReference parent_reference =
    treeview_->appendNode(treeview_->getRootNode(), *button);
  treeview_->setCollapsed(parent_reference, true);

  // Record this plugin in plugin_entries_.
  PluginEntry entry;
  entry.parent = button;
  entry.parent_reference = parent_reference;
  entry.frame = nullptr;
  plugin_entries_[plugin] = entry;

  if (PURPLE_PLUGIN_HAS_PREF_FRAME(plugin)) {
    PurplePluginUiInfo *ui_info = PURPLE_PLUGIN_UI_INFO(plugin);
    PurplePluginPrefFrame *frame = ui_info->get_plugin_pref_frame(plugin);

    // Note this pref frame.
    plugin_entries_[plugin].frame = frame;

    // Group node (set to an "invalid" value).
    CppConsUI::TreeView::NodeReference group = treeview_->getRootNode();

    for (GList *iter = purple_plugin_pref_frame_get_prefs(frame);
         iter != nullptr; iter = iter->next) {
      PurplePluginPref *pref = reinterpret_cast<PurplePluginPref *>(iter->data);
      PurplePluginPrefType type = purple_plugin_pref_get_type(pref);
      const char *label = purple_plugin_pref_get_label(pref);
      const char *name = purple_plugin_pref_get_name(pref);
      if (name == nullptr) {
        if (label == nullptr)
          continue;

        if (type == PURPLE_PLUGIN_PREF_INFO) {
          // No value label.
          treeview_->appendNode(
            parent_reference, *(new CppConsUI::Label(label)));
        }
        else {
          // New group.
          group = treeview_->appendNode(parent_reference,
            *(new CppConsUI::TreeView::ToggleCollapseButton(label)));
        }
        continue;
      }

      PurplePrefType value_type = purple_prefs_get_type(name);
      CppConsUI::Widget *pref_widget;
      if (type == PURPLE_PLUGIN_PREF_CHOICE) {
        auto combo = new CppConsUI::ComboBox(label);
        pref_widget = combo;

        // Add possible options.
        for (GList *pref_iter = purple_plugin_pref_get_choices(pref);
             pref_iter != nullptr && pref_iter->next != nullptr;
             pref_iter = pref_iter->next->next)
          combo->addOptionPtr(reinterpret_cast<const char *>(pref_iter->data),
            pref_iter->next->data);

        // Set default value.
        intptr_t current_value;
        if (value_type == PURPLE_PREF_BOOLEAN)
          current_value = purple_prefs_get_bool(name);
        else if (value_type == PURPLE_PREF_INT)
          current_value = purple_prefs_get_int(name);
        else if (value_type == PURPLE_PREF_STRING)
          current_value =
            reinterpret_cast<intptr_t>(purple_prefs_get_string(name));
        else if (value_type == PURPLE_PREF_PATH)
          current_value =
            reinterpret_cast<intptr_t>(purple_prefs_get_path(name));
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

      if (group == treeview_->getRootNode()) {
        // No group has been created, so create one.
        group = treeview_->appendNode(parent_reference,
          *(new CppConsUI::TreeView::ToggleCollapseButton(_("Preferences"))));
      }
      treeview_->appendNode(group, *pref_widget);
    }
  }

  // Disable plugin.
  auto disable_button = new CppConsUI::Button(_("Disable plugin"));
  disable_button->signal_activate.connect(
    sigc::bind(sigc::mem_fun(this, &PluginWindow::disablePlugin), plugin));
  treeview_->appendNode(parent_reference, *disable_button);
}

void PluginWindow::addPlugin(CppConsUI::Button & /*activator*/)
{
  // Show a window for the user to select a plugin that he wants to add.
  auto win = new AddPluginWindow;
  win->signal_selection.connect(
    sigc::mem_fun(this, &PluginWindow::onAddPluginSelection));
  win->show();
}

void PluginWindow::onAddPluginSelection(
  AddPluginWindow & /*activator*/, PurplePlugin *plugin)
{
  // The user has selected a plugin that he wants to add, so do it.
  if (!purple_plugin_load(plugin)) {
    LOG->error(_("Error loading plugin '%s'."), purple_plugin_get_name(plugin));
    return;
  }

  purple_plugins_save_loaded(CONF_PLUGINS_SAVE_PREF);

  // Populate this plugin.
  populatePlugin(plugin);

  // Make the button for this plugin focused.
  PluginEntries::iterator i = plugin_entries_.find(plugin);
  g_assert(i != plugin_entries_.end());
  g_assert(i->second.parent != nullptr);
  i->second.parent->grabFocus();
}

void PluginWindow::disablePlugin(
  CppConsUI::Button & /*activator*/, PurplePlugin *plugin)
{
  auto dialog = new CppConsUI::MessageDialog(
    _("Disable plugin"), _("Are you sure you want to disable this plugin?"));
  dialog->signal_response.connect(sigc::bind(
    sigc::mem_fun(this, &PluginWindow::disablePluginResponseHandler), plugin));
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

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
