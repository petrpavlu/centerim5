/*
 * Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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

#include "GeneralMenu.h"

#include "AccountStatusMenu.h"
#include "AccountWindow.h"
#include "Accounts.h"
#include "Log.h"
#include "OptionWindow.h"
#include "PluginWindow.h"

#include "gettext.h"

GeneralMenu::GeneralMenu() : MenuWindow(0, 0, AUTOSIZE, AUTOSIZE)
{
  setColorScheme("generalmenu");

  appendItem(
    _("Change status"), sigc::mem_fun(this, &GeneralMenu::openStatusWindow));
  appendItem(
    _("Accounts..."), sigc::mem_fun(this, &GeneralMenu::openAccountWindow));
  appendItem(
    _("Add buddy..."), sigc::mem_fun(this, &GeneralMenu::openAddBuddyRequest));
  appendItem(
    _("Add chat..."), sigc::mem_fun(this, &GeneralMenu::openAddChatRequest));
  appendItem(
    _("Add group..."), sigc::mem_fun(this, &GeneralMenu::openAddGroupRequest));
  appendItem(_("Pending requests..."),
    sigc::mem_fun(this, &GeneralMenu::openPendingRequests));
  appendItem(_("Config options..."),
    sigc::mem_fun(this, &GeneralMenu::openOptionWindow));
  appendItem(
    _("Plugins..."), sigc::mem_fun(this, &GeneralMenu::openPluginWindow));
  appendSeparator();
#ifdef DEBUG
  MenuWindow *submenu = new MenuWindow(0, 0, AUTOSIZE, AUTOSIZE);
  submenu->setColorScheme("generalmenu");
  submenu->appendItem(
    "Request input", sigc::mem_fun(this, &GeneralMenu::openRequestInputTest));
  submenu->appendItem(
    "Request choice", sigc::mem_fun(this, &GeneralMenu::openRequestChoiceTest));
  submenu->appendItem(
    "Request action", sigc::mem_fun(this, &GeneralMenu::openRequestActionTest));
  submenu->appendItem(
    "Request fields", sigc::mem_fun(this, &GeneralMenu::openRequestFieldsTest));
  appendSubMenu("Debug...", *submenu);
  appendSeparator();
#endif // DEBUG
  appendItem(_("Quit"), sigc::hide(sigc::mem_fun(CENTERIM, &CenterIM::quit)));

  onScreenResized();
}

void GeneralMenu::onScreenResized()
{
  CppConsUI::Rect chat = CENTERIM->getScreenArea(CenterIM::CHAT_AREA);
  move(chat.x, chat.y);
}

void GeneralMenu::openStatusWindow(CppConsUI::Button & /*activator*/)
{
  AccountStatusMenu *win = new AccountStatusMenu;
  win->show();
  close();
}

void GeneralMenu::openAccountWindow(CppConsUI::Button & /*activator*/)
{
  AccountWindow *win = new AccountWindow;
  win->show();
  close();
}

void GeneralMenu::openAddBuddyRequest(CppConsUI::Button & /*activator*/)
{
  purple_blist_request_add_buddy(NULL, NULL, NULL, NULL);
  close();
}

void GeneralMenu::openAddChatRequest(CppConsUI::Button & /*activator*/)
{
  purple_blist_request_add_chat(NULL, NULL, NULL, NULL);
  close();
}

void GeneralMenu::openAddGroupRequest(CppConsUI::Button & /*activator*/)
{
  purple_blist_request_add_group();
  close();
}

void GeneralMenu::openPendingRequests(CppConsUI::Button & /*activator*/)
{
  ACCOUNTS->openPendingRequests();
  close();
}

void GeneralMenu::openOptionWindow(CppConsUI::Button & /*activator*/)
{
  OptionWindow *win = new OptionWindow;
  win->show();
  close();
}

void GeneralMenu::openPluginWindow(CppConsUI::Button & /*activator*/)
{
  PluginWindow *win = new PluginWindow;
  win->show();
  close();
}

#ifdef DEBUG
void GeneralMenu::openRequestInputTest(CppConsUI::Button & /*activator*/)
{
  purple_request_input(NULL, "Title", "Primary", "Secondary", "default value",
    FALSE, FALSE, NULL, "Ok", G_CALLBACK(input_ok_cb_), "Cancel", NULL, NULL,
    NULL, NULL, this);

  close();
}

void GeneralMenu::openRequestChoiceTest(CppConsUI::Button & /*activator*/)
{
  purple_request_choice(NULL, "Title", "Primary", "Secondary", 1, "Ok",
    G_CALLBACK(choice_ok_cb_), "Cancel", NULL, NULL, NULL, NULL, this,
    "Option 0", 0, "Option 1", 1, "Option 2", 2, NULL);

  close();
}

void GeneralMenu::openRequestActionTest(CppConsUI::Button & /*activator*/)
{
  purple_request_action(NULL, "Title", "Primary", "Secondary", 1, NULL, NULL,
    NULL, this, 3, "Action 0", G_CALLBACK(action_cb_), "Action 1", NULL,
    "Action 2", G_CALLBACK(action_cb_));

  close();
}

void GeneralMenu::openRequestFieldsTest(CppConsUI::Button & /*activator*/)
{
  PurpleRequestFields *fields = purple_request_fields_new();
  PurpleRequestFieldGroup *g = purple_request_field_group_new("Group 0");

  purple_request_fields_add_group(fields, g);

  PurpleRequestField *f;
  f = purple_request_field_string_new("text0", "String field 0", "def0", FALSE);
  purple_request_field_group_add_field(g, f);
  f = purple_request_field_string_new("text1", "String field 1", NULL, FALSE);
  purple_request_field_group_add_field(g, f);
  f = purple_request_field_int_new("int0", "Int field 0", INT_MAX);
  purple_request_field_group_add_field(g, f);
  f = purple_request_field_bool_new("bool0", "Bool field 0", FALSE);
  purple_request_field_group_add_field(g, f);

  f = purple_request_field_choice_new("choice0", "Choice field 0", 2);
  purple_request_field_choice_add(f, "One");
  purple_request_field_choice_add(f, "Two");
  purple_request_field_choice_add(f, "Three");
  purple_request_field_choice_add(f, "Four");
  purple_request_field_group_add_field(g, f);

  f = purple_request_field_list_new("list0", "List field 0, multiple");
  purple_request_field_list_set_multi_select(f, TRUE);
  purple_request_field_list_add(f, "One", this);
  purple_request_field_list_add(f, "Two", this);
  purple_request_field_list_add(f, "Three", this);
  purple_request_field_list_add(f, "Four", this);
  purple_request_field_list_add_selected(f, "Three");
  purple_request_field_group_add_field(g, f);

  f = purple_request_field_list_new("list1", "List field 1, single");
  purple_request_field_list_set_multi_select(f, FALSE);
  purple_request_field_list_add(f, "One", this);
  purple_request_field_list_add(f, "Two", this);
  purple_request_field_list_add(f, "Three", this);
  purple_request_field_list_add(f, "Four", this);
  purple_request_field_list_add_selected(f, "Three");
  purple_request_field_group_add_field(g, f);

  f = purple_request_field_account_new("account0", "Account field 0", NULL);
  purple_request_field_account_set_show_all(f, TRUE);
  purple_request_field_group_add_field(g, f);

  purple_request_fields(NULL, "Title", "Primary", "Secondary", fields, "Ok",
    G_CALLBACK(fields_ok_cb_), "Cancel", NULL, NULL, NULL, NULL, this);

  close();
}

void GeneralMenu::input_ok_cb(const char *text)
{
  LOG->debug("input_ok_cb: %s", text);
}

void GeneralMenu::choice_ok_cb(int selected)
{
  LOG->debug("choice_ok_cb: %d", selected);
}

void GeneralMenu::action_cb(int action)
{
  LOG->debug("action_cb: %d", action);
}

void GeneralMenu::fields_ok_cb(PurpleRequestFields *fields)
{
  LOG->debug(
    "fields_ok_cb: text0=%s, text1=%s, int0=%d, bool0=%d, choice0=%d\n",
    purple_request_fields_get_string(fields, "text0"),
    purple_request_fields_get_string(fields, "text1"),
    purple_request_fields_get_integer(fields, "int0"),
    purple_request_fields_get_bool(fields, "bool0"),
    purple_request_fields_get_choice(fields, "choice0"));

  for (GList *list = purple_request_field_list_get_selected(
         purple_request_fields_get_field(fields, "list0"));
       list; list = list->next)
    LOG->debug(
      "fields_ok_cb: list0=%s", reinterpret_cast<const char *>(list->data));

  for (GList *list = purple_request_field_list_get_selected(
         purple_request_fields_get_field(fields, "list1"));
       list; list = list->next)
    LOG->debug(
      "fields_ok_cb: list1=%s", reinterpret_cast<const char *>(list->data));

  PurpleAccount *account =
    purple_request_fields_get_account(fields, "account0");
  LOG->debug("fields_ok_cb: account0=[%s] %s",
    purple_account_get_protocol_name(account),
    purple_account_get_username(account));
}
#endif // DEBUG

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
