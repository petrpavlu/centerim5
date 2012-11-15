/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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

#include "Accounts.h"

#include "Log.h"

#include <typeinfo>
#include "gettext.h"

Accounts *Accounts::instance = NULL;

Accounts *Accounts::Instance()
{
  return instance;
}

void Accounts::RestoreStatuses(bool offline)
{
  if (!offline) {
    // simply restore statuses
    purple_accounts_restore_current_statuses();
    return;
  }

  // note: the next lines are taken from Pidgin's main() function

  // set all accounts to "offline"
  PurpleSavedStatus *saved_status;

  // if we've used this type+message before, lookup the transient status
  saved_status = purple_savedstatus_find_transient_by_type_and_message(
      PURPLE_STATUS_OFFLINE, NULL);

  // if this type+message is unique then create a new transient saved status
  if (!saved_status)
    saved_status = purple_savedstatus_new(NULL, PURPLE_STATUS_OFFLINE);

  // Set the status for each account
  purple_savedstatus_activate(saved_status);

  /* XXX We currently don't support saved statuses correctly so make sure this
   * status deleted. */
  purple_savedstatus_delete_by_status(saved_status);
}

void Accounts::OpenPendingRequests()
{
  PendingRequestWindow *win = new PendingRequestWindow(*this);
  win->Show();
}

Accounts::Request::Request(PurpleAccount *account_, const char *remote_user_,
    const char *id_, const char *alias_)
: account(account_)
{
  remote_user = g_strdup(remote_user_);
  id = g_strdup(id_);
  alias = g_strdup(alias_);
}

Accounts::Request::~Request()
{
  if (remote_user)
    g_free(remote_user);
  if (id)
    g_free(id);
  if (alias)
    g_free(alias);
}

Accounts::AddRequest::AddRequest(PurpleAccount *account_,
    const char *remote_user_, const char *id_, const char *alias_)
: Request(account_, remote_user_, id_, alias_)
{
}

Accounts::AuthRequest::AuthRequest(PurpleAccount *account_,
    const char *remote_user_, const char *id_, const char *alias_,
    const char *message_, bool /*on_list_*/,
    PurpleAccountRequestAuthorizationCb auth_cb_,
    PurpleAccountRequestAuthorizationCb deny_cb_, void *data_)
: Request(account_, remote_user_, id_, alias_), auth_cb(auth_cb_)
, deny_cb(deny_cb_), data(data_)
{
  message = g_strdup(message_);
}

Accounts::AuthRequest::~AuthRequest()
{
  if (message)
    g_free(message);
}

Accounts::PendingRequestWindow::PendingRequestWindow(Accounts& accounts_)
: SplitDialog(0, 0, 80, 24, _("Pending requests"))
, accounts(&accounts_)
{
  SetColorScheme("generalwindow");

  requests = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  SetContainer(*requests);

  Populate();

  accounts->signal_request_add.connect(sigc::mem_fun(this,
        &Accounts::PendingRequestWindow::OnRequestAdd));
  accounts->signal_request_remove.connect(sigc::mem_fun(this,
        &Accounts::PendingRequestWindow::OnRequestRemove));

  buttons->AppendItem(_("Done"), sigc::hide(sigc::mem_fun(this,
          &PendingRequestWindow::Close)));
}

void Accounts::PendingRequestWindow::OnScreenResized()
{
  MoveResizeRect(CENTERIM->GetScreenArea(CenterIM::CHAT_AREA));
}

Accounts::PendingRequestWindow::RequestDialog::RequestDialog(
    const char *title, const char *text)
: AbstractDialog(title)
{
  AddButton(YES_BUTTON_TEXT, RESPONSE_YES);
  AddSeparator();
  AddButton(NO_BUTTON_TEXT, RESPONSE_NO);
  // never give focus to the textview
  buttons->SetFocusCycle(FOCUS_CYCLE_LOCAL);

  CppConsUI::TextView *textview = new CppConsUI::TextView(AUTOSIZE, AUTOSIZE);
  textview->Append(text);
  layout->InsertWidget(0, *textview);
}

void Accounts::PendingRequestWindow::RequestDialog::EmitResponse(
    ResponseType response)
{
  signal_response(*this, response);
}

void Accounts::PendingRequestWindow::Populate()
{
  const Requests *requests = accounts->GetRequests();
  for (Requests::const_iterator i = requests->begin(); i != requests->end();
      i++)
    AppendRequest(**i);
}

void Accounts::PendingRequestWindow::AppendRequest(const Request& request)
{
  const char *req;
  if (typeid(request) == typeid(AddRequest))
    req = _("Add");
  else if (typeid(request) == typeid(AuthRequest))
    req = _("Authorize");
  else
    g_assert_not_reached();

  char *text;
  if (request.alias)
    text = g_strdup_printf("[%s] %s: %s %s (%s)",
        purple_account_get_protocol_name(request.account),
        purple_account_get_username(request.account), req,
        request.remote_user, request.alias);
  else
    text = g_strdup_printf("[%s] %s: %s %s",
        purple_account_get_protocol_name(request.account),
        purple_account_get_username(request.account), req,
        request.remote_user);

  CppConsUI::Button *b = new CppConsUI::Button(text);
  b->signal_activate.connect(sigc::bind(sigc::mem_fun(this,
          &Accounts::PendingRequestWindow::OnActivate), sigc::ref(request)));

  g_free(text);

  CppConsUI::TreeView::NodeReference node = requests->AppendNode(
      requests->GetRootNode(), *b);
  request_map[&request] = node;
}

void Accounts::PendingRequestWindow::OnRequestAdd(Accounts& /*accounts*/,
    const Request& request)
{
  AppendRequest(request);
}

void Accounts::PendingRequestWindow::OnRequestRemove(Accounts& /*accounts*/,
    const Request& request)
{
  RequestMap::iterator i = request_map.find(&request);
  g_assert(i != request_map.end());
  requests->DeleteNode(i->second, false);
  request_map.erase(i);
}

void Accounts::PendingRequestWindow::OnActivate(
    CppConsUI::Button& /*activator*/, const Request& request)
{
  if (typeid(request) == typeid(AddRequest)) {
    const AddRequest *add_request = dynamic_cast<const AddRequest*>(&request);
    g_assert(add_request);

    char *text;
    if (request.alias)
      text = g_strdup_printf(_("Add %s (%s) to your buddy list?"),
          request.remote_user, request.alias);
    else
      text = g_strdup_printf(_("Add %s to your buddy list?"),
          request.remote_user);
    RequestDialog *win = new RequestDialog(_("Add buddy"), text);
    g_free(text);

    win->signal_response.connect(sigc::bind(sigc::mem_fun(this,
          &Accounts::PendingRequestWindow::OnAddResponse),
          sigc::ref(*add_request)));
    win->Show();
  }
  else if (typeid(request) == typeid(AuthRequest)) {
    const AuthRequest *auth_request
      = dynamic_cast<const AuthRequest*>(&request);
    g_assert(auth_request);

    char *text;
    if (request.alias)
      text = g_strdup_printf(
          _("Allow %s (%s) to add you to his or her buddy list?\n"
            "Message: %s"), request.remote_user, request.alias,
          auth_request->message);
    else
      text = g_strdup_printf(
          _("Allow %s to add you to his or her buddy list?\n"
            "Message: %s"), request.remote_user, auth_request->message);
    RequestDialog *win = new RequestDialog(_("Authorize buddy"), text);
    g_free(text);

    win->signal_response.connect(sigc::bind(sigc::mem_fun(this,
          &Accounts::PendingRequestWindow::OnAuthResponse),
          sigc::ref(*auth_request)));
    win->Show();
  }
  else
    g_assert_not_reached();
}

void Accounts::PendingRequestWindow::OnAddResponse(
    RequestDialog& /*activator*/, ResponseType response,
    const AddRequest& request)
{
  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_YES:
      purple_blist_request_add_buddy(request.account, request.remote_user,
          NULL, request.alias);
      accounts->RemoveRequest(request);
      break;
    case CppConsUI::AbstractDialog::RESPONSE_NO:
      accounts->RemoveRequest(request);
      break;
    default:
      break;
  }
}

void Accounts::PendingRequestWindow::OnAuthResponse(
    RequestDialog& /*activator*/, ResponseType response,
    const AuthRequest& request)
{
  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_YES:
      request.auth_cb(request.data);
      if (!purple_find_buddy(request.account, request.remote_user))
        purple_blist_request_add_buddy(request.account, request.remote_user,
            NULL, request.alias);
      accounts->RemoveRequest(request);
      break;
    case CppConsUI::AbstractDialog::RESPONSE_NO:
      request.deny_cb(request.data);
      accounts->RemoveRequest(request);
      break;
    default:
      break;
  }
}

Accounts::Accounts()
{
  // if the statuses are not known, set them all to the default
  if (!purple_prefs_get_bool("/purple/savedstatus/startup_current_status"))
    purple_savedstatus_activate(purple_savedstatus_get_startup());

  // set the purple account callbacks
  memset(&centerim_account_ui_ops, 0, sizeof(centerim_account_ui_ops));
  centerim_account_ui_ops.notify_added = notify_added_;
  centerim_account_ui_ops.status_changed = status_changed_;
  centerim_account_ui_ops.request_add = request_add_;
  centerim_account_ui_ops.request_authorize = request_authorize_;
  centerim_account_ui_ops.close_account_request = close_account_request_;
  purple_accounts_set_ui_ops(&centerim_account_ui_ops);
}

Accounts::~Accounts()
{
  purple_accounts_set_ui_ops(NULL);
}

void Accounts::Init()
{
  g_assert(!instance);

  instance = new Accounts;
}

void Accounts::Finalize()
{
  g_assert(instance);

  delete instance;
  instance = NULL;
}

void Accounts::RemoveRequest(const Request& request)
{
  Requests::iterator i = std::find(requests.begin(), requests.end(),
      &request);
  g_assert(i != requests.end());

  signal_request_remove(*this, **i);
  delete *i;
  requests.erase(i);
  signal_request_count_change(*this, requests.size());
}

void Accounts::notify_added(PurpleAccount *account, const char *remote_user,
    const char * /*id*/, const char *alias, const char *message)
{
  const char *proto = purple_account_get_protocol_name(account);
  const char *uname = purple_account_get_username(account);

  /* The code below creates four translation strings in the pot file. It's
   * possible to reduce it only to one string but then the string contains
   * many quirks and it's impossible to translate it without looking into the
   * code. */
  if (message) {
    if (alias)
      LOG->Message(_("+ [%s] %s: %s (%s) has made you his or her buddy: %s"),
          proto, uname, remote_user, alias, message);
    else
      LOG->Message(_("+ [%s] %s: %s has made you his or her buddy: %s"),
          proto, uname, remote_user, message);
  }
  else {
    if (alias)
      LOG->Message(_("+ [%s] %s: %s (%s) has made you his or her buddy"),
          proto, uname, remote_user, alias);
    else
      LOG->Message(_("+ [%s] %s: %s has made you his or her buddy"),
          proto, uname, remote_user);
  }
}

void Accounts::status_changed(PurpleAccount *account, PurpleStatus *status)
{
  if (!purple_account_get_enabled(account, PACKAGE_NAME))
    return;

  LOG->Message(_("+ [%s] %s: Status changed to: %s"),
      purple_account_get_protocol_name(account),
      purple_account_get_username(account),
      purple_status_get_name(status));
}

void Accounts::request_add(PurpleAccount *account, const char *remote_user,
    const char *id, const char *alias, const char * /*message*/)
{
  AddRequest *request = new AddRequest(account, remote_user, id, alias);
  requests.push_back(request);
  signal_request_add(*this, *request);
  signal_request_count_change(*this, requests.size());

  const char *proto = purple_account_get_protocol_name(account);
  const char *uname = purple_account_get_username(account);
  if (alias)
    LOG->Message(_("+ [%s] %s: New add request from %s (%s)"), proto, uname,
        remote_user, alias);
  else
    LOG->Message(_("+ [%s] %s: New add request from %s"), proto, uname,
        remote_user);
}

void *Accounts::request_authorize(PurpleAccount *account,
    const char *remote_user, const char *id, const char *alias,
    const char *message, gboolean on_list,
    PurpleAccountRequestAuthorizationCb authorize_cb,
    PurpleAccountRequestAuthorizationCb deny_cb,
    void *user_data)
{
  AuthRequest *request = new AuthRequest(account, remote_user, id, alias,
      message, on_list, authorize_cb, deny_cb, user_data);
  requests.push_back(request);
  signal_request_add(*this, *request);
  signal_request_count_change(*this, requests.size());

  const char *proto = purple_account_get_protocol_name(account);
  const char *uname = purple_account_get_username(account);
  if (alias)
    LOG->Message(_("+ [%s] %s: New authorization request from %s (%s)"),
        proto, uname, remote_user, alias);
  else
    LOG->Message(_("+ [%s] %s: New authorization request from %s"), proto,
        uname, remote_user);

  return request;
}

void Accounts::close_account_request(void *ui_handle)
{
  Requests::iterator i = std::find(requests.begin(), requests.end(),
      ui_handle);
  if (i == requests.end())
    return;

  signal_request_remove(*this, **i);
  delete *i;
  requests.erase(i);
  signal_request_count_change(*this, requests.size());
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
