/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2013 by CenterIM developers
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
   * status is deleted. */
  purple_savedstatus_delete_by_status(saved_status);
}

void Accounts::OpenPendingRequests()
{
  /* Make sure there si no more than one PendingRequestWindow opened. Note
   * that this should actually never happen because this window is opened only
   * from the general menu and this window is a top window. */
  if (request_window) {
    // bring the current window to the top of the window stack
    request_window->Show();
    return;
  }

  request_window = new PendingRequestWindow(*this, requests);
  request_window->signal_close.connect(sigc::mem_fun(this,
        &Accounts::OnPendingRequestWindowClose));
  request_window->Show();
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

Accounts::PendingRequestWindow::PendingRequestWindow(Accounts& accounts_,
    const Requests& requests)
: SplitDialog(0, 0, 80, 24, _("Pending requests")), accounts(&accounts_)
, dialog(NULL)
{
  SetColorScheme("generalwindow");

  treeview = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  SetContainer(*treeview);

  for (Requests::const_iterator i = requests.begin(); i != requests.end();
      i++)
    AppendRequest(**i);

  buttons->AppendItem(_("Done"), sigc::hide(sigc::mem_fun(this,
          &PendingRequestWindow::Close)));
}

void Accounts::PendingRequestWindow::OnScreenResized()
{
  MoveResizeRect(CENTERIM->GetScreenArea(CenterIM::CHAT_AREA));
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
          &PendingRequestWindow::OnActivate), sigc::ref(request)));

  g_free(text);

  CppConsUI::TreeView::NodeReference node = treeview->AppendNode(
      treeview->GetRootNode(), *b);
  request_map[&request] = node;
}

void Accounts::PendingRequestWindow::RemoveRequest(const Request& request)
{
  RequestMap::iterator i = request_map.find(&request);
  g_assert(i != request_map.end());

  /* If a dialog is opened for this request then close it. This happens only
   * when the request is closed by some external event. This should never
   * happen when the dialog/request is closed by the user. */
  if (dialog && dialog->GetRequest() == &request) {
    // it has to be an auth request
    g_assert(typeid(request) == typeid(AuthRequest));

    dialog->Close();

    /* The dialog should become NULL because the Close() method triggers
     * the OnAuthResponse() handler. */
    g_assert(!dialog);
  }

  treeview->DeleteNode(i->second, false);
  request_map.erase(i);
}

Accounts::PendingRequestWindow::RequestDialog::RequestDialog(
    const Request& request_, const char *title, const char *text)
: AbstractDialog(title), request(&request_)
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

void Accounts::PendingRequestWindow::OnActivate(
    CppConsUI::Button& /*activator*/, const Request& request)
{
  // we can't have more than one request dialog opened
  g_assert(!dialog);

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
    dialog = new RequestDialog(request, _("Add buddy"), text);
    g_free(text);

    dialog->signal_response.connect(sigc::mem_fun(this,
          &PendingRequestWindow::OnAddResponse));
    dialog->Show();
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
    dialog = new RequestDialog(request, _("Authorize buddy"), text);
    g_free(text);

    dialog->signal_response.connect(sigc::mem_fun(this,
          &PendingRequestWindow::OnAuthResponse));
    dialog->Show();
  }
  else
    g_assert_not_reached();
}

void Accounts::PendingRequestWindow::OnAddResponse(RequestDialog& activator,
    ResponseType response)
{
  // stay sane
  g_assert(dialog == &activator);

  const AddRequest *request
    = dynamic_cast<const AddRequest*>(activator.GetRequest());
  g_assert(request);

  /* Set early that there is no active dialog anymore. This is important
   * because otherwise the dialog is deleted twice. Once on the
   * Accounts::CloseRequest() ->
   * Accounts::PendingRequestWindow::RemoveRequest() path and once after
   * returning from this response handling method (because dialogs are closed
   * and deleted after a response is made by the user). */
  dialog = NULL;

  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_YES:
      purple_blist_request_add_buddy(request->account, request->remote_user,
          NULL, request->alias);
      accounts->CloseRequest(*request);
      break;
    case CppConsUI::AbstractDialog::RESPONSE_NO:
      accounts->CloseRequest(*request);
      break;
    default:
      break;
  }
}

void Accounts::PendingRequestWindow::OnAuthResponse(RequestDialog& activator,
    ResponseType response)
{
  // stay sane
  g_assert(dialog == &activator);

  const AuthRequest *request
    = dynamic_cast<const AuthRequest*>(activator.GetRequest());
  g_assert(request);

  /* Set early that there is no active dialog anymore. This is important
   * because otherwise the dialog is deleted twice. Once on the
   * Accounts::CloseRequest() ->
   * Accounts::PendingRequestWindow::RemoveRequest() path and once after
   * returning from this response handling method (because dialogs are closed
   * and deleted after a response is made by the user). */
  dialog = NULL;

  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_YES:
      request->auth_cb(request->data);
      if (!purple_find_buddy(request->account, request->remote_user))
        purple_blist_request_add_buddy(request->account, request->remote_user,
            NULL, request->alias);
      accounts->CloseRequest(*request);
      break;
    case CppConsUI::AbstractDialog::RESPONSE_NO:
      request->deny_cb(request->data);
      accounts->CloseRequest(*request);
      break;
    default:
      break;
  }
}

Accounts::Accounts()
: request_window(NULL)
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

void Accounts::CloseRequest(const Request& request)
{
  /* It isn't really nice to delete an object referenced by a const parameter,
   * but well.. */

  Requests::iterator i = std::find(requests.begin(), requests.end(),
      &request);
  /* It's possible that the request has been already closed but it should be
   * very very rare (and it'd most likely signalize a problem in
   * libpurple). */
  if (i == requests.end())
    return;

  if (request_window)
    request_window->RemoveRequest(**i);
  delete *i;
  requests.erase(i);
  signal_request_count_change(*this, requests.size());
}

void Accounts::OnPendingRequestWindowClose(CppConsUI::FreeWindow& activator)
{
  // the request window is dying
  g_assert(request_window == &activator);
  request_window = NULL;
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
  if (request_window)
    request_window->AppendRequest(*request);

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
  signal_request_count_change(*this, requests.size());
  if (request_window)
    request_window->AppendRequest(*request);

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
  g_return_if_fail(i == requests.end());

  // this code path is only for auth requests
  g_assert(typeid(*i) == typeid(AuthRequest*));

  if (request_window)
    request_window->RemoveRequest(**i);
  delete *i;
  requests.erase(i);
  signal_request_count_change(*this, requests.size());
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
