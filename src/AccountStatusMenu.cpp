/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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

#include "AccountStatusMenu.h"

#include "Accounts.h"
#include "CenterIM.h"

#include "gettext.h"

AccountStatusMenu::AccountStatusMenu()
: MenuWindow(0, 0, 40, 20)
{
  SetColorScheme("accountstatusmenu");

  /*
  TODO
  AppendItem(_("All accounts"), sigc::mem_fun(this,
        &AccountStatusMenu::Dummy));
  AppendItem(_("Already logged in only"), sigc::mem_fun(this,
        &AccountStatusMenu::Dummy));
  AppendSeparator();
  */

  for (GList *iter = purple_accounts_get_all_active(); iter;
      iter = iter->next) {
    PurpleAccount *account = reinterpret_cast<PurpleAccount*>(iter->data);

    char *text = g_strdup_printf("[%s] %s",
        purple_account_get_protocol_name(account),
        purple_account_get_username(account));
    AppendItem(text, sigc::bind(sigc::mem_fun(this,
            &AccountStatusMenu::OpenStatusPopup), account));
    g_free(text);
  }
}

void AccountStatusMenu::ScreenResized()
{
  Rect chat = CENTERIM->GetScreenAreaSize(CenterIM::CHAT_AREA);
  MoveResize(chat.x, chat.y, win_w, win_h);
}

AccountStatusMenu::StatusPopup::StatusPopup(int x, int y, int w, int h,
    PurpleAccount *account)
: MenuWindow(x, y, w, h), account(account)
{
  SetColorScheme("accountstatusmenu");

  int height = 2;

  bool has_independents = false;
  for (GList *iter = purple_account_get_status_types(account); iter;
      iter = iter->next) {
    PurpleStatusType *status_type
      = reinterpret_cast<PurpleStatusType*>(iter->data);

    if (purple_status_type_is_independent(status_type)) {
      has_independents = true;
      continue;
    }

    bool active = purple_presence_is_status_active(
        purple_account_get_presence(account),
        purple_status_type_get_id(status_type));

    char *label;
    if (active)
      label = g_strdup_printf("* %s",
          purple_status_type_get_name(status_type));
    else
      label = g_strdup(purple_status_type_get_name(status_type));
    Button *b = AppendItem(label, sigc::bind(
          sigc::mem_fun(this, &AccountStatusMenu::StatusPopup::SetStatus),
          account, status_type, true));
    if (active)
      b->GrabFocus();
    g_free(label);
    height++;
  }

  if (has_independents) {
    AppendSeparator();
    height++;

    for (GList *iter = purple_account_get_status_types(account); iter;
        iter = iter->next) {
      PurpleStatusType *status_type
        = reinterpret_cast<PurpleStatusType*>(iter->data);

      if (!purple_status_type_is_independent(status_type))
        continue;

      bool active = purple_presence_is_status_active(
          purple_account_get_presence(account),
          purple_status_type_get_id(status_type));

      char *label;
      if (active)
        label = g_strdup_printf("* %s",
            purple_status_type_get_name(status_type));
      else
        label = g_strdup(purple_status_type_get_name(status_type));
      Button *b = AppendItem(label, sigc::bind(
            sigc::mem_fun(this, &AccountStatusMenu::StatusPopup::SetStatus),
            account, status_type, !active));
      if (active)
        b->GrabFocus();
      g_free(label);
      height++;
    }
  }

  MoveResize(Left(), Top(), Width(), height);
}

void AccountStatusMenu::StatusPopup::ScreenResized()
{
  Rect chat = CENTERIM->GetScreenAreaSize(CenterIM::CHAT_AREA);
  MoveResize(chat.x, chat.y, win_w, win_h);
}

void AccountStatusMenu::StatusPopup::SetStatus(Button& activator,
    PurpleAccount *account, PurpleStatusType *status_type, bool active)
{
  purple_account_set_status(account, purple_status_type_get_id(status_type),
      active, NULL);
  Close();
}

void AccountStatusMenu::OpenStatusPopup(Button& activator,
    PurpleAccount *account)
{
  AccountStatusMenu::StatusPopup *status_popup = new StatusPopup(win_x, win_y,
      win_w, win_h, account);
  status_popup->Show();
}
