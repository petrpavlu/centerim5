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

  GList *iter;
  char *text;

  AppendItem(_("All accounts"), sigc::mem_fun(this,
        &AccountStatusMenu::Dummy));
  AppendItem(_("Already logged in only"), sigc::mem_fun(this,
        &AccountStatusMenu::Dummy));
  AppendSeparator();

  for (iter = purple_accounts_get_all(); iter; iter = iter->next) {
    PurpleAccount *account = (PurpleAccount*)iter->data;

    text = g_strdup_printf("[%s] %s",
        purple_account_get_protocol_name(account),
        purple_account_get_username(account));

    AppendItem(text, sigc::bind(sigc::mem_fun(this,
            &AccountStatusMenu::OpenStatusPopup), account));

    g_free(text);
  }
}

AccountStatusMenu::~AccountStatusMenu()
{
}

void AccountStatusMenu::OpenStatusPopup(Button& activator,
    PurpleAccount *account)
{
  AccountStatusMenu::StatusPopup *status_popup = new StatusPopup(xpos, ypos,
      width, height, account);
  status_popup->Show();
}

AccountStatusMenu::StatusPopup::StatusPopup(int x, int y, int w, int h,
    PurpleAccount *account)
: MenuWindow(x, y, w, h)
, account(account)
{
  SetColorScheme("accountstatuspopup");

  GList *iter;
  PurpleStatusType *status_type;
  char *label;
  bool has_independents;
  bool active;

  has_independents = false;
  for (iter = purple_account_get_status_types(account); iter;
      iter = iter->next) {
    status_type = (PurpleStatusType*)iter->data;

    if (purple_status_type_is_independent(status_type)) {
      has_independents = true;
      continue;
    }

    active = purple_presence_is_status_active(
        purple_account_get_presence(account),
        purple_status_type_get_id(status_type));

    if (active)
      label = g_strdup_printf("* %s", purple_status_type_get_name(status_type));
    else
      label = g_strdup(purple_status_type_get_name(status_type));

    AppendItem(label, sigc::bind(
          sigc::mem_fun(this, &AccountStatusMenu::StatusPopup::SetStatus),
          account, status_type, true));

    g_free(label);
  }

  if (!has_independents)
    return;

  AppendSeparator();

  for (iter = purple_account_get_status_types(account); iter;
      iter = iter->next) {
    status_type = (PurpleStatusType*)iter->data;

    if (!purple_status_type_is_independent(status_type))
      continue;

    active = purple_presence_is_status_active(
        purple_account_get_presence(account),
        purple_status_type_get_id(status_type));

    if (active)
      label = g_strdup_printf("* %s",
          purple_status_type_get_name(status_type));
    else
      label = g_strdup(purple_status_type_get_name(status_type));

    AppendItem(label, sigc::bind(
          sigc::mem_fun(this, &AccountStatusMenu::StatusPopup::SetStatus),
          account, status_type, !active));

    g_free(label);
  }

/*
  * Primitive (default) statuses (taken from gtksavedstatuses.c)*
  for (i = PURPLE_STATUS_UNSET + 1; i < PURPLE_STATUS_NUM_PRIMITIVES; i++) {
                if (i == PURPLE_STATUS_MOBILE || i == PURPLE_STATUS_TUNE)
                        *
                         * Special-case these.  They're intended to be independent
                         * status types, so don't show them in the list.
                         *
                        continue;
    PurpleStatusPrimitive primitive = (PurpleStatusPrimitive)i;

    AddItem(purple_primitive_get_name_from_type(primitive),
        sigc::bind(sigc::mem_fun(this, &AccountStatusMenu::StatusPopup::SetStatusPrimitive), account, primitive));
  }


  * Custom statuses *
  //TODO only use `popular' statuses here (like pidgin and finch do)?
  for (iter = purple_savedstatuses_get_all(); iter; iter = iter->next) {
    PurpleSavedStatus *status = (PurpleSavedStatus*)iter->data;

    AddItem(purple_savedstatus_get_title(status),
        sigc::bind(sigc::mem_fun(this, &AccountStatusMenu::StatusPopup::SetSavedStatus), account, status));
  }
*/
}

AccountStatusMenu::StatusPopup::~StatusPopup()
{
}

void AccountStatusMenu::StatusPopup::SetStatus(Button& activator,
    PurpleAccount *account, PurpleStatusType *status_type, bool active)
{
  purple_account_set_status(account, purple_status_type_get_id(status_type),
      active, NULL);
  Close();
}
/*
void AccountStatusMenu::StatusPopup::SetStatus(PurpleAccount *account, PurpleSavedStatus *status)
{
  accounts->SetStatus(account, status);
  Close();
}

void AccountStatusMenu::StatusPopup::SetStatusPrimitive(PurpleAccount *account, PurpleStatusPrimitive primitive)
{
  Accounts *accounts = Accounts::Instance();
  accounts->SetStatus(account, primitive);
  Close();
}*/

void AccountStatusMenu::ScreenResized()
{
  Rect chat = CENTERIM->GetScreenAreaSize(CenterIM::CHAT_AREA);
  MoveResize(chat.x, chat.y, win_w, win_h);
}
