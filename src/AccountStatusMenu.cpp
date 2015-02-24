/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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

#include "AccountStatusMenu.h"

#include "CenterIM.h"

AccountStatusMenu::AccountStatusMenu()
: MenuWindow(0, 0, AUTOSIZE, AUTOSIZE)
{
  setColorScheme("accountstatusmenu");

  /*
  TODO
  AppendItem(_("All accounts"), sigc::mem_fun(this,
        &AccountStatusMenu::Dummy));
  AppendItem(_("Already logged in only"), sigc::mem_fun(this,
        &AccountStatusMenu::Dummy));
  AppendSeparator();
  */

  GList *list, *l;
  list = l = purple_accounts_get_all_active();
  while (l) {
    PurpleAccount *account = reinterpret_cast<PurpleAccount*>(l->data);

    char *text = g_strdup_printf("[%s] %s",
        purple_account_get_protocol_name(account),
        purple_account_get_username(account));
    appendItem(text, sigc::bind(sigc::mem_fun(this,
            &AccountStatusMenu::openStatusPopup), account));
    g_free(text);

    l = l->next;
  }
  g_list_free(list);

  onScreenResized();
}

void AccountStatusMenu::onScreenResized()
{
  CppConsUI::Rect chat = CENTERIM->getScreenArea(CenterIM::CHAT_AREA);
  move(chat.x, chat.y);
}

void AccountStatusMenu::openStatusPopup(CppConsUI::Button& activator,
    PurpleAccount *account)
{
  StatusPopup *status_popup = new StatusPopup(account);
  status_popup->setReferenceWidget(activator);
  status_popup->show();
}

AccountStatusMenu::StatusPopup::StatusPopup(PurpleAccount *account)
: MenuWindow(0, 0, AUTOSIZE, AUTOSIZE)
{
  setColorScheme("accountstatusmenu");

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
    CppConsUI::Button *b = appendItem(label, sigc::bind(sigc::mem_fun(this,
            &StatusPopup::setStatus), account, status_type, true));
    if (active)
      b->grabFocus();
    g_free(label);
  }

  if (has_independents) {
    appendSeparator();

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
      CppConsUI::Button *b = appendItem(label, sigc::bind(sigc::mem_fun(this,
              &StatusPopup::setStatus), account, status_type, !active));
      if (active)
        b->grabFocus();
      g_free(label);
    }
  }

  onScreenResized();
}

void AccountStatusMenu::StatusPopup::setStatus(
    CppConsUI::Button& /*activator*/, PurpleAccount *account,
    PurpleStatusType *status_type, bool active)
{
  purple_account_set_status(account, purple_status_type_get_id(status_type),
      active, NULL);
  close();
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
