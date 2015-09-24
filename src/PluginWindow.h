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

#ifndef PLUGINWINDOW_H
#define PLUGINWINDOW_H

#include <cppconsui/Button.h>
#include <cppconsui/CheckBox.h>
#include <cppconsui/ComboBox.h>
#include <cppconsui/InputDialog.h>
#include <cppconsui/MessageDialog.h>
#include <cppconsui/SplitDialog.h>
#include <cppconsui/TreeView.h>
#include <libpurple/purple.h>

class PluginWindow : public CppConsUI::SplitDialog {
public:
  PluginWindow();
  virtual ~PluginWindow() override;

  // FreeWindow
  virtual void onScreenResized() override;

private:
  class AddPluginWindow : public CppConsUI::Window {
  public:
    AddPluginWindow();
    virtual ~AddPluginWindow() override {}

    // FreeWindow
    virtual void onScreenResized() override;

    // Signal that the user has selected a plugin that he wants to add.
    sigc::signal<void, AddPluginWindow &, PurplePlugin *> signal_selection;

  protected:
    void onPluginButtonActivate(
      CppConsUI::Button &activator, PurplePlugin *plugin);

  private:
    CONSUI_DISABLE_COPY(AddPluginWindow);
  };

  class BoolOption : public CppConsUI::CheckBox {
  public:
    BoolOption(const char *name, const char *pref);
    virtual ~BoolOption() override;

  protected:
    char *pref_;

    void onToggle(CppConsUI::CheckBox &activator, bool new_state);

  private:
    CONSUI_DISABLE_COPY(BoolOption);
  };

  class StringOption : public CppConsUI::Button {
  public:
    StringOption(const char *name, const char *pref);
    virtual ~StringOption() override;

  protected:
    char *pref_;

    void updateValue();
    void onActivate(CppConsUI::Button &activator);
    void responseHandler(CppConsUI::InputDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);

  private:
    CONSUI_DISABLE_COPY(StringOption);
  };

  class IntegerOption : public CppConsUI::Button {
  public:
    IntegerOption(const char *name, const char *pref);
    virtual ~IntegerOption() override;

  protected:
    char *pref_;

    void updateValue();
    void onActivate(CppConsUI::Button &activator);
    void responseHandler(CppConsUI::InputDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);

  private:
    CONSUI_DISABLE_COPY(IntegerOption);
  };

  class PathOption : public CppConsUI::Button {
  public:
    PathOption(const char *name, const char *pref);
    virtual ~PathOption() override;

  protected:
    char *pref_;

    void updateValue();
    void onActivate(CppConsUI::Button &activator);
    void responseHandler(CppConsUI::InputDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);

  private:
    CONSUI_DISABLE_COPY(PathOption);
  };

  struct PluginEntry {
    CppConsUI::Button *parent;
    CppConsUI::TreeView::NodeReference parent_reference;
    PurplePluginPrefFrame *frame;
  };
  typedef std::map<PurplePlugin *, PluginEntry> PluginEntries;

  CppConsUI::TreeView *treeview_;
  PluginEntries plugin_entries_;

  CONSUI_DISABLE_COPY(PluginWindow);

  void clearPlugin(PurplePlugin *plugin);
  void populatePlugin(PurplePlugin *plugin);

  void addPlugin(CppConsUI::Button &activator);
  void onAddPluginSelection(AddPluginWindow &activator, PurplePlugin *plugin);

  void disablePlugin(CppConsUI::Button &activator, PurplePlugin *plugin);
  void disablePluginResponseHandler(CppConsUI::MessageDialog &activator,
    CppConsUI::AbstractDialog::ResponseType response, PurplePlugin *plugin);
};

#endif // PLUGINWINDOW_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
