/*
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

#ifndef __NOTIFY_H__
#define __NOTIFY_H__

#include <cppconsui/MessageDialog.h>
#include <cppconsui/SplitDialog.h>
#include <cppconsui/TreeView.h>
#include <libpurple/purple.h>

#define NOTIFY (Notify::instance())

class Notify
{
public:
  static Notify *instance();

protected:

private:
  class UserInfoDialog
  : public CppConsUI::SplitDialog
  {
  public:
    UserInfoDialog(const char *title);
    virtual ~UserInfoDialog() {}

    // Window
    virtual void onScreenResized();

    void update(PurpleConnection *gc, const char *who,
        PurpleNotifyUserInfo *user_info);

  protected:
    CppConsUI::TreeView *treeview;

  private:
    CONSUI_DISABLE_COPY(UserInfoDialog);
  };

  typedef std::set<CppConsUI::AbstractDialog *> Notifications;
  typedef std::pair<PurpleAccount *, std::string> User;
  typedef std::map<User, UserInfoDialog *> UserInfo;

  /* Track all opened notifications so it is possible to break the
   * purple_notify_close() -> AbstractDialog::Close() -> purple_notify_close()
   * -> etc. loop. */
  Notifications notifications;
  /* Keep track of all opened user info dialogs so they can be updated when
   * notify_userinfo() is called for them several times. */
  UserInfo userinfos;

  PurpleNotifyUiOps centerim_notify_ui_ops;

  static Notify *my_instance;

  Notify();
  ~Notify();
  CONSUI_DISABLE_COPY(Notify);

  static void init();
  static void finalize();
  friend class CenterIM;

  void onDialogClose(CppConsUI::Window &activator, PurpleNotifyType type);
  void onUserInfoDialogClose(CppConsUI::Window &activator, User user);

  static void *notify_message_(PurpleNotifyMsgType type, const char *title,
      const char *primary, const char *secondary)
    { return NOTIFY->notify_message(type, title, primary, secondary); }
  static void *notify_userinfo_(PurpleConnection *gc, const char *who,
      PurpleNotifyUserInfo *user_info)
    { return NOTIFY->notify_userinfo(gc, who, user_info); }
  static void close_notify_(PurpleNotifyType type, void *ui_handle)
    { NOTIFY->close_notify(type, ui_handle); }

  void *notify_message(PurpleNotifyMsgType type, const char *title,
      const char *primary, const char *secondary);
  void *notify_userinfo(PurpleConnection *gc, const char *who,
      PurpleNotifyUserInfo *user_info);
  void close_notify(PurpleNotifyType type, void *ui_handle);
};

#endif // __NOTIFY_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
