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

#include "Notify.h"

#include "Log.h"

#include "gettext.h"
#include <cstring>

Notify *Notify::my_instance_ = nullptr;

Notify *Notify::instance()
{
  return my_instance_;
}

Notify::UserInfoDialog::UserInfoDialog(const char *title) : SplitDialog(title)
{
  setColorScheme(CenterIM::SCHEME_GENERALWINDOW);

  treeview_ = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  setContainer(*treeview_);

  buttons_->appendItem(
    _("Done"), sigc::hide(sigc::mem_fun(this, &UserInfoDialog::close)));

  onScreenResized();
}

void Notify::UserInfoDialog::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::CHAT_AREA));
}

void Notify::UserInfoDialog::update(
  PurpleConnection *gc, const char *who, PurpleNotifyUserInfo *user_info)
{
  treeview_->clear();
  CppConsUI::TreeView::NodeReference parent;
  CppConsUI::Button *button;

  // Local information.
  PurpleAccount *account = purple_connection_get_account(gc);
  PurpleBuddy *buddy = purple_find_buddy(account, who);
  if (buddy != nullptr) {
    // Note that we should always be able to find the specified buddy, unless
    // something goes very wrong.
    button =
      new CppConsUI::TreeView::ToggleCollapseButton(_("Local information"));
    parent = treeview_->appendNode(treeview_->getRootNode(), *button);

    button = new CppConsUI::Button(
      CppConsUI::Button::FLAG_VALUE, _("Alias"), purple_buddy_get_alias(buddy));
    treeview_->appendNode(parent, *button);

    time_t saved_time;
    struct tm local_time;
    const char *formatted_time;

    // Last seen.
    if (PURPLE_BUDDY_IS_ONLINE(buddy))
      formatted_time = _("Now");
    else {
      saved_time = static_cast<time_t>(
        purple_blist_node_get_int(PURPLE_BLIST_NODE(buddy), "last_seen"));
      if (saved_time != 0 && localtime_r(&saved_time, &local_time) != nullptr)
        formatted_time = purple_date_format_long(&local_time);
      else
        formatted_time = _("Unknown");
    }
    button = new CppConsUI::Button(
      CppConsUI::Button::FLAG_VALUE, _("Last seen"), formatted_time);
    treeview_->appendNode(parent, *button);

    // Last activity.
    saved_time = static_cast<time_t>(
      purple_blist_node_get_int(PURPLE_BLIST_NODE(buddy), "last_activity"));
    if (saved_time != 0 && localtime_r(&saved_time, &local_time) != nullptr)
      formatted_time = purple_date_format_long(&local_time);
    else
      formatted_time = _("Unknown");
    button = new CppConsUI::Button(
      CppConsUI::Button::FLAG_VALUE, _("Last activity"), formatted_time);
    treeview_->appendNode(parent, *button);
  }

  // Remote information.
  button =
    new CppConsUI::TreeView::ToggleCollapseButton(_("Remote information"));
  parent = treeview_->appendNode(treeview_->getRootNode(), *button);
  CppConsUI::TreeView::NodeReference subparent = parent;
  for (GList *i = purple_notify_user_info_get_entries(user_info); i != nullptr;
       i = i->next) {
    PurpleNotifyUserInfoEntry *entry =
      reinterpret_cast<PurpleNotifyUserInfoEntry *>(i->data);
    PurpleNotifyUserInfoEntryType type =
      purple_notify_user_info_entry_get_type(entry);

    const char *label = purple_notify_user_info_entry_get_label(entry);
    if (label == nullptr)
      continue;
    const char *value = purple_notify_user_info_entry_get_value(entry);
    char *nohtml = purple_markup_strip_html(value);
    switch (type) {
    case PURPLE_NOTIFY_USER_INFO_ENTRY_PAIR:
      button = new CppConsUI::Button(
        nohtml ? CppConsUI::Button::FLAG_VALUE : 0, label, nohtml);
      treeview_->appendNode(subparent, *button);
      break;
    case PURPLE_NOTIFY_USER_INFO_ENTRY_SECTION_BREAK:
      // Ignore section breaks.
      break;
    case PURPLE_NOTIFY_USER_INFO_ENTRY_SECTION_HEADER:
      button = new CppConsUI::TreeView::ToggleCollapseButton(label);
      subparent = treeview_->appendNode(parent, *button);
      break;
    default:
      LOG->error(_("Unhandled userinfo entry type '%d'."), type);
      break;
    }
    g_free(nohtml);
  }

  treeview_->grabFocus();
}

Notify::Notify()
{
  std::memset(&centerim_notify_ui_ops_, 0, sizeof(centerim_notify_ui_ops_));

  // Set the purple notify callbacks.
  centerim_notify_ui_ops_.notify_message = notify_message_;
  // centerim_notify_ui_ops_.notify_email = notify_email_;
  // centerim_notify_ui_ops_.notify_emails = notify_emails_;
  // centerim_notify_ui_ops_.notify_formatted = notify_formatted_;
  // centerim_notify_ui_ops_.notify_searchresults = notify_searchresults_;
  // centerim_notify_ui_ops_.notify_searchresults_new_rows =
  //  notify_searchresults_new_rows_;
  centerim_notify_ui_ops_.notify_userinfo = notify_userinfo_;
  // centerim_notify_ui_ops_.notify_uri = notify_uri_;
  centerim_notify_ui_ops_.close_notify = close_notify_;
  purple_notify_set_ui_ops(&centerim_notify_ui_ops_);
}

Notify::~Notify()
{
  purple_notify_set_ui_ops(nullptr);
}

void Notify::init()
{
  g_assert(my_instance_ == nullptr);

  my_instance_ = new Notify;
}

void Notify::finalize()
{
  g_assert(my_instance_ != nullptr);

  delete my_instance_;
  my_instance_ = nullptr;
}

void Notify::onDialogClose(CppConsUI::Window &activator, PurpleNotifyType type)
{
  CppConsUI::AbstractDialog *dialog =
    dynamic_cast<CppConsUI::AbstractDialog *>(&activator);
  g_assert(dialog != nullptr);

  if (notifications_.find(dialog) == notifications_.end())
    return;

  notifications_.erase(dialog);
  purple_notify_close(type, dialog);
}

void Notify::onUserInfoDialogClose(CppConsUI::Window & /*activator*/, User user)
{
  // The userinfo dialog is gone.
  user_infos_.erase(user);
}

void *Notify::notify_message(PurpleNotifyMsgType /*type*/, const char *title,
  const char *primary, const char *secondary)
{
  char *text = g_strdup_printf("%s\n\n%s", primary, secondary);
  auto dialog = new CppConsUI::MessageDialog(title, text);
  g_free(text);
  dialog->signal_close.connect(sigc::bind(
    sigc::mem_fun(this, &Notify::onDialogClose), PURPLE_NOTIFY_MESSAGE));
  dialog->show();

  notifications_.insert(dialog);
  return dialog;
}

void *Notify::notify_userinfo(
  PurpleConnection *gc, const char *who, PurpleNotifyUserInfo *user_info)
{
  User user(purple_connection_get_account(gc), who);
  UserInfos::iterator i = user_infos_.find(user);
  UserInfoDialog *dialog;
  if (i == user_infos_.end()) {
    // Create a new dialog to display this user info.
    char *title = g_strdup_printf(_("User information for %s"), who);
    dialog = new UserInfoDialog(title);
    g_free(title);

    dialog->signal_close.connect(
      sigc::bind(sigc::mem_fun(this, &Notify::onUserInfoDialogClose), user));
    dialog->signal_close.connect(sigc::bind(
      sigc::mem_fun(this, &Notify::onDialogClose), PURPLE_NOTIFY_USERINFO));
    dialog->show();

    notifications_.insert(dialog);
    user_infos_[user] = dialog;
  }
  else {
    // Update already opened dialog.
    dialog = i->second;
  }

  dialog->update(gc, who, user_info);
  return dialog;
}

void Notify::close_notify(PurpleNotifyType type, void *ui_handle)
{
  // Only some notifications are currently supported.
  g_assert(type == PURPLE_NOTIFY_MESSAGE || type == PURPLE_NOTIFY_USERINFO);

  CppConsUI::AbstractDialog *dialog =
    reinterpret_cast<CppConsUI::AbstractDialog *>(ui_handle);
  if (notifications_.find(dialog) == notifications_.end())
    return;

  notifications_.erase(dialog);
  // Close the notification dialog if one is still opened.
  dialog->close();
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
