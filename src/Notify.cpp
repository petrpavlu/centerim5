/*
 * Copyright (C) 2010-2012 by CenterIM developers
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

#include "Notify.h"

#include "Log.h"

#include <string.h> // memset
#include "gettext.h"

Notify *Notify::instance = NULL;

Notify *Notify::Instance()
{
  return instance;
}

Notify::UserInfoDialog::UserInfoDialog(const char *title)
: SplitDialog(title)
{
  SetColorScheme("generalwindow");

  treeview = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  SetContainer(*treeview);

  buttons->AppendItem(_("Done"), sigc::hide(sigc::mem_fun(this,
          &UserInfoDialog::Close)));
}

void Notify::UserInfoDialog::OnScreenResized()
{
  MoveResizeRect(CENTERIM->GetScreenArea(CenterIM::CHAT_AREA));
}

void Notify::UserInfoDialog::Update(PurpleNotifyUserInfo *user_info)
{
  treeview->Clear();

  CppConsUI::TreeView::NodeReference parent = treeview->GetRootNode();
  for (GList *i = purple_notify_user_info_get_entries(user_info); i;
      i = i->next) {
    PurpleNotifyUserInfoEntry *entry
      = reinterpret_cast<PurpleNotifyUserInfoEntry*>(i->data);
    PurpleNotifyUserInfoEntryType type
      = purple_notify_user_info_entry_get_type(entry);

    const char *label = purple_notify_user_info_entry_get_label(entry);
    if (!label)
      continue;
    const char *value = purple_notify_user_info_entry_get_value(entry);
    char *nohtml = purple_markup_strip_html(value);
    CppConsUI::Button *button;
    switch (type) {
      case PURPLE_NOTIFY_USER_INFO_ENTRY_PAIR:
        button = new CppConsUI::Button(
            nohtml ? CppConsUI::Button::FLAG_VALUE : 0, label, nohtml);
        treeview->AppendNode(parent, *button);
        break;
      case PURPLE_NOTIFY_USER_INFO_ENTRY_SECTION_BREAK:
        // ignore section breaks
        break;
      case PURPLE_NOTIFY_USER_INFO_ENTRY_SECTION_HEADER:
        button = new CppConsUI::TreeView::ToggleCollapseButton(label);
        parent = treeview->AppendNode(treeview->GetRootNode(), *button);
        break;
      default:
        LOG->Error(_("Unhandled userinfo entry type '%d'."), type);
        break;
    }
    g_free(nohtml);
  }

  treeview->GrabFocus();
}

Notify::Notify()
{
  memset(&centerim_notify_ui_ops, 0, sizeof(centerim_notify_ui_ops));

  // set the purple notify callbacks
  centerim_notify_ui_ops.notify_message = notify_message_;
  //centerim_notify_ui_ops.notify_email = notify_email_;
  //centerim_notify_ui_ops.notify_emails = notify_emails_;
  //centerim_notify_ui_ops.notify_formatted = notify_formatted_;
  //centerim_notify_ui_ops.notify_searchresults = notify_searchresults_;
  //centerim_notify_ui_ops.notify_searchresults_new_rows =
  //  notify_searchresults_new_rows_;
  centerim_notify_ui_ops.notify_userinfo = notify_userinfo_;
  //centerim_notify_ui_ops.notify_uri = notify_uri_;
  centerim_notify_ui_ops.close_notify = close_notify_;
  purple_notify_set_ui_ops(&centerim_notify_ui_ops);
}

Notify::~Notify()
{
  purple_notify_set_ui_ops(NULL);
}

void Notify::Init()
{
  g_assert(!instance);

  instance = new Notify;
}

void Notify::Finalize()
{
  g_assert(instance);

  delete instance;
  instance = NULL;
}

void Notify::OnDialogClose(CppConsUI::FreeWindow& activator,
    PurpleNotifyType type)
{
  UserInfoDialog *dialog = dynamic_cast<UserInfoDialog*>(&activator);
  g_assert(dialog);

  if (notifications.find(dialog) != notifications.end()) {
    notifications.erase(dialog);
    purple_notify_close(type, dialog);
  }
}

void Notify::OnUserInfoDialogClose(CppConsUI::FreeWindow& /*activator*/,
    User user)
{
  // the userinfo dialog is gone
  userinfos.erase(user);
}

void *Notify::notify_message(PurpleNotifyMsgType /*type*/, const char *title,
    const char *primary, const char *secondary)
{
  char *text = g_strdup_printf("%s\n\n%s", primary, secondary);
  CppConsUI::MessageDialog *dialog = new CppConsUI::MessageDialog(title,
      text);
  g_free(text);
  dialog->signal_close.connect(sigc::bind(sigc::mem_fun(this,
          &Notify::OnDialogClose), PURPLE_NOTIFY_MESSAGE));
  dialog->Show();

  notifications.insert(dialog);
  return dialog;
}

void *Notify::notify_userinfo(PurpleConnection *gc, const char *who,
    PurpleNotifyUserInfo *user_info)
{
  User user(purple_connection_get_account(gc), who);
  UserInfo::iterator i = userinfos.find(user);
  UserInfoDialog *dialog;
  if (i == userinfos.end()) {
    // create a new dialog to display this user info
    char *title = g_strdup_printf(_("User information for %s"), who);
    dialog = new UserInfoDialog(title);
    g_free(title);

    dialog->signal_close.connect(sigc::bind(sigc::mem_fun(this,
          &Notify::OnUserInfoDialogClose), user));
    dialog->signal_close.connect(sigc::bind(sigc::mem_fun(this,
            &Notify::OnDialogClose), PURPLE_NOTIFY_USERINFO));
    dialog->Show();

    notifications.insert(dialog);
    userinfos[user] = dialog;
  }
  else {
    // update already opened dialog
    dialog = i->second;
  }

  dialog->Update(user_info);
  return dialog;
}

void Notify::close_notify(PurpleNotifyType type, void *ui_handle)
{
  // only some notifications are currently supported
  g_assert(type == PURPLE_NOTIFY_MESSAGE || type == PURPLE_NOTIFY_USERINFO);

  CppConsUI::MessageDialog *dialog
    = reinterpret_cast<CppConsUI::MessageDialog*>(ui_handle);
  if (notifications.find(dialog) != notifications.end()) {
    notifications.erase(dialog);
    // close the notification dialog if one is still opened
    dialog->Close();
  }
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
