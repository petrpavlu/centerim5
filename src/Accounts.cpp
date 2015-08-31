// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Accounts.h"

#include "Log.h"

#include <typeinfo>
#include "gettext.h"

Accounts *Accounts::my_instance_ = nullptr;

Accounts *Accounts::instance()
{
  return my_instance_;
}

void Accounts::restoreStatuses(bool offline)
{
  if (!offline) {
    // Simply restore statuses.
    purple_accounts_restore_current_statuses();
    return;
  }

  // Set all accounts to "offline".
  PurpleSavedStatus *saved_status;

  // If we have used this type+message before, lookup the transient status.
  saved_status = purple_savedstatus_find_transient_by_type_and_message(
    PURPLE_STATUS_OFFLINE, nullptr);

  // If this type+message is unique then create a new transient saved status.
  if (!saved_status)
    saved_status = purple_savedstatus_new(nullptr, PURPLE_STATUS_OFFLINE);

  // Set the status for each account.
  purple_savedstatus_activate(saved_status);

  // We currently do not support saved statuses correctly so make sure this
  // status is deleted.
  purple_savedstatus_delete_by_status(saved_status);
}

void Accounts::openPendingRequests()
{
  // Make sure there si no more than one PendingRequestWindow opened. Note that
  // this should actually never happen because this window is opened only from
  // the general menu and this window is a top window.
  if (request_window_ != nullptr) {
    // Bring the current window to the top of the window stack.
    request_window_->show();
    return;
  }

  request_window_ = new PendingRequestWindow(*this, requests_);
  request_window_->signal_close.connect(
    sigc::mem_fun(this, &Accounts::onPendingRequestWindowClose));
  request_window_->show();
}

Accounts::Request::Request(PurpleAccount *account, const char *remote_user,
  const char *id, const char *alias)
  : account_(account)
{
  remote_user_ = g_strdup(remote_user);
  id_ = g_strdup(id);
  alias_ = g_strdup(alias);
}

Accounts::Request::~Request()
{
  g_free(remote_user_);
  g_free(id_);
  g_free(alias_);
}

Accounts::AddRequest::AddRequest(PurpleAccount *account,
  const char *remote_user, const char *id, const char *alias)
  : Request(account, remote_user, id, alias)
{
}

Accounts::AuthRequest::AuthRequest(PurpleAccount *account,
  const char *remote_user, const char *id, const char *alias,
  const char *message, bool /*on_list*/,
  PurpleAccountRequestAuthorizationCb auth_cb,
  PurpleAccountRequestAuthorizationCb deny_cb, void *data)
  : Request(account, remote_user, id, alias), auth_cb_(auth_cb),
    deny_cb_(deny_cb), data_(data)
{
  message_ = g_strdup(message);
}

Accounts::AuthRequest::~AuthRequest()
{
  g_free(message_);
}

Accounts::PendingRequestWindow::PendingRequestWindow(
  Accounts &accounts, const Requests &requests)
  : SplitDialog(0, 0, 80, 24, _("Pending requests")), accounts_(&accounts),
    dialog_(nullptr)
{
  setColorScheme(CenterIM::SCHEME_GENERALWINDOW);

  treeview_ = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  setContainer(*treeview_);

  for (const Request *request : requests)
    appendRequest(*request);

  buttons_->appendItem(
    _("Done"), sigc::hide(sigc::mem_fun(this, &PendingRequestWindow::close)));

  onScreenResized();
}

void Accounts::PendingRequestWindow::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::CHAT_AREA));
}

void Accounts::PendingRequestWindow::appendRequest(const Request &request)
{
  const char *req;
  if (typeid(request) == typeid(AddRequest))
    req = _("Add");
  else if (typeid(request) == typeid(AuthRequest))
    req = _("Authorize");
  else
    g_assert_not_reached();

  char *text;
  if (request.alias_ != nullptr)
    text = g_strdup_printf("[%s] %s: %s %s (%s)",
      purple_account_get_protocol_name(request.account_),
      purple_account_get_username(request.account_), req, request.remote_user_,
      request.alias_);
  else
    text = g_strdup_printf("[%s] %s: %s %s",
      purple_account_get_protocol_name(request.account_),
      purple_account_get_username(request.account_), req, request.remote_user_);

  auto b = new CppConsUI::Button(text);
  b->signal_activate.connect(
    sigc::bind(sigc::mem_fun(this, &PendingRequestWindow::onActivate),
      sigc::ref(request)));

  g_free(text);

  CppConsUI::TreeView::NodeReference node =
    treeview_->appendNode(treeview_->getRootNode(), *b);
  request_map_[&request] = node;
}

void Accounts::PendingRequestWindow::removeRequest(const Request &request)
{
  RequestMap::iterator i = request_map_.find(&request);
  g_assert(i != request_map_.end());

  // If a dialog is opened for this request then close it. This happens only
  // when the request is closed by some external event. This should never happen
  // when the dialog/request is closed by the user.
  if (dialog_ != nullptr && dialog_->getRequest() == &request) {
    // It has to be an auth request.
    g_assert(typeid(request) == typeid(AuthRequest));

    dialog_->close();

    // The dialog should become NULL because the close() method triggers the
    // OnAuthResponse() handler.
    g_assert(dialog_ == nullptr);
  }

  treeview_->deleteNode(i->second, false);
  request_map_.erase(i);
}

Accounts::PendingRequestWindow::RequestDialog::RequestDialog(
  const Request &request, const char *title, const char *text)
  : AbstractDialog(title), request_(&request)
{
  addButton(YES_BUTTON_TEXT, RESPONSE_YES);
  addSeparator();
  addButton(NO_BUTTON_TEXT, RESPONSE_NO);
  // Never give focus to the textview.
  buttons_->setFocusCycle(FOCUS_CYCLE_LOCAL);

  auto textview = new CppConsUI::TextView(AUTOSIZE, AUTOSIZE);
  textview->append(text);
  layout_->insertWidget(0, *textview);
}

void Accounts::PendingRequestWindow::RequestDialog::emitResponse(
  ResponseType response)
{
  signal_response(*this, response);
}

void Accounts::PendingRequestWindow::onActivate(
  CppConsUI::Button & /*activator*/, const Request &request)
{
  // We cannot have more than one request dialog opened.
  g_assert(dialog_ == nullptr);

  if (typeid(request) == typeid(AddRequest)) {
    const AddRequest *add_request = dynamic_cast<const AddRequest *>(&request);
    g_assert(add_request);

    char *text;
    if (request.alias_ != nullptr)
      text = g_strdup_printf(_("Add %s (%s) to your buddy list?"),
        request.remote_user_, request.alias_);
    else
      text =
        g_strdup_printf(_("Add %s to your buddy list?"), request.remote_user_);
    dialog_ = new RequestDialog(request, _("Add buddy"), text);
    g_free(text);

    dialog_->signal_response.connect(
      sigc::mem_fun(this, &PendingRequestWindow::onAddResponse));
    dialog_->show();
  }
  else if (typeid(request) == typeid(AuthRequest)) {
    const AuthRequest *auth_request =
      dynamic_cast<const AuthRequest *>(&request);
    g_assert(auth_request);

    char *text;
    if (request.alias_ != nullptr)
      text =
        g_strdup_printf(_("Allow %s (%s) to add you to his or her buddy list?\n"
                          "Message: %s"),
          request.remote_user_, request.alias_, auth_request->message_);
    else
      text = g_strdup_printf(_("Allow %s to add you to his or her buddy list?\n"
                               "Message: %s"),
        request.remote_user_, auth_request->message_);
    dialog_ = new RequestDialog(request, _("Authorize buddy"), text);
    g_free(text);

    dialog_->signal_response.connect(
      sigc::mem_fun(this, &PendingRequestWindow::onAuthResponse));
    dialog_->show();
  }
  else
    g_assert_not_reached();
}

void Accounts::PendingRequestWindow::onAddResponse(
  RequestDialog &activator, ResponseType response)
{
  // Stay sane.
  g_assert(dialog_ == &activator);

  const AddRequest *request =
    dynamic_cast<const AddRequest *>(activator.getRequest());
  g_assert(request);

  // Set early that there is no active dialog anymore. This is important because
  // otherwise the dialog is deleted twice. Once on the Accounts::closeRequest()
  // -> Accounts::PendingRequestWindow::removeRequest() path and once after
  // returning from this response handling method (because dialogs are closed
  // and deleted after a response is made by the user).
  dialog_ = nullptr;

  switch (response) {
  case CppConsUI::AbstractDialog::RESPONSE_YES:
    purple_blist_request_add_buddy(
      request->account_, request->remote_user_, nullptr, request->alias_);
    accounts_->closeRequest(*request);
    break;
  case CppConsUI::AbstractDialog::RESPONSE_NO:
    accounts_->closeRequest(*request);
    break;
  default:
    break;
  }
}

void Accounts::PendingRequestWindow::onAuthResponse(
  RequestDialog &activator, ResponseType response)
{
  // Stay sane.
  g_assert(dialog_ == &activator);

  const AuthRequest *request =
    dynamic_cast<const AuthRequest *>(activator.getRequest());
  g_assert(request != nullptr);

  // Set early that there is no active dialog anymore. This is important because
  // otherwise the dialog is deleted twice. Once on the Accounts::closeRequest()
  // -> Accounts::PendingRequestWindow::removeRequest() path and once after
  // returning from this response handling method (because dialogs are closed
  // and deleted after a response is made by the user).
  dialog_ = nullptr;

  switch (response) {
  case CppConsUI::AbstractDialog::RESPONSE_YES:
    request->auth_cb_(request->data_);
    if (!purple_find_buddy(request->account_, request->remote_user_))
      purple_blist_request_add_buddy(
        request->account_, request->remote_user_, nullptr, request->alias_);
    accounts_->closeRequest(*request);
    break;
  case CppConsUI::AbstractDialog::RESPONSE_NO:
    request->deny_cb_(request->data_);
    accounts_->closeRequest(*request);
    break;
  default:
    break;
  }
}

Accounts::Accounts() : request_window_(nullptr)
{
  // If the statuses are not known, set them all to the default.
  if (!purple_prefs_get_bool("/purple/savedstatus/startup_current_status"))
    purple_savedstatus_activate(purple_savedstatus_get_startup());

  // Set the purple account callbacks.
  memset(&centerim_account_ui_ops_, 0, sizeof(centerim_account_ui_ops_));
  centerim_account_ui_ops_.notify_added = notify_added_;
  centerim_account_ui_ops_.status_changed = status_changed_;
  centerim_account_ui_ops_.request_add = request_add_;
  centerim_account_ui_ops_.request_authorize = request_authorize_;
  centerim_account_ui_ops_.close_account_request = close_account_request_;
  purple_accounts_set_ui_ops(&centerim_account_ui_ops_);
}

Accounts::~Accounts()
{
  purple_accounts_set_ui_ops(nullptr);
}

void Accounts::init()
{
  g_assert(my_instance_ == nullptr);

  my_instance_ = new Accounts;
}

void Accounts::finalize()
{
  g_assert(my_instance_ != nullptr);

  delete my_instance_;
  my_instance_ = nullptr;
}

void Accounts::closeRequest(const Request &request)
{
  // It is not really nice to delete an object referenced by a const parameter,
  // but well..

  Requests::iterator i =
    std::find(requests_.begin(), requests_.end(), &request);
  // It is possible that the request has been already closed but it should be
  // very very rare (and it would most likely signalize a problem in libpurple).
  if (i == requests_.end())
    return;

  if (request_window_ != nullptr)
    request_window_->removeRequest(**i);
  delete *i;
  requests_.erase(i);
  signal_request_count_change(*this, requests_.size());
}

void Accounts::onPendingRequestWindowClose(CppConsUI::Window &activator)
{
  // The request window is dying.
  g_assert(request_window_ == &activator);
  request_window_ = nullptr;
}

void Accounts::notify_added(PurpleAccount *account, const char *remote_user,
  const char * /*id*/, const char *alias, const char *message)
{
  const char *proto = purple_account_get_protocol_name(account);
  const char *uname = purple_account_get_username(account);

  // The code below creates four translation strings in the pot file. It is
  // possible to reduce it only to one string but then the string contains many
  // quirks and it is impossible to translate it without looking into the code.
  if (message) {
    if (alias)
      LOG->message(_("+ [%s] %s: %s (%s) has made you his or her buddy: %s"),
        proto, uname, remote_user, alias, message);
    else
      LOG->message(_("+ [%s] %s: %s has made you his or her buddy: %s"), proto,
        uname, remote_user, message);
  }
  else {
    if (alias)
      LOG->message(_("+ [%s] %s: %s (%s) has made you his or her buddy"), proto,
        uname, remote_user, alias);
    else
      LOG->message(_("+ [%s] %s: %s has made you his or her buddy"), proto,
        uname, remote_user);
  }
}

void Accounts::status_changed(PurpleAccount *account, PurpleStatus *status)
{
  if (!purple_account_get_enabled(account, PACKAGE_NAME))
    return;

  LOG->message(_("+ [%s] %s: Status changed to: %s"),
    purple_account_get_protocol_name(account),
    purple_account_get_username(account), purple_status_get_name(status));
}

void Accounts::request_add(PurpleAccount *account, const char *remote_user,
  const char *id, const char *alias, const char * /*message*/)
{
  auto request = new AddRequest(account, remote_user, id, alias);
  requests_.push_back(request);
  if (request_window_ != nullptr)
    request_window_->appendRequest(*request);

  const char *proto = purple_account_get_protocol_name(account);
  const char *uname = purple_account_get_username(account);
  if (alias != nullptr)
    LOG->message(_("+ [%s] %s: New add request from %s (%s)"), proto, uname,
      remote_user, alias);
  else
    LOG->message(
      _("+ [%s] %s: New add request from %s"), proto, uname, remote_user);
}

void *Accounts::request_authorize(PurpleAccount *account,
  const char *remote_user, const char *id, const char *alias,
  const char *message, gboolean on_list,
  PurpleAccountRequestAuthorizationCb auth_cb,
  PurpleAccountRequestAuthorizationCb deny_cb, void *user_data)
{
  auto request = new AuthRequest(account, remote_user, id, alias, message,
    on_list, auth_cb, deny_cb, user_data);
  requests_.push_back(request);
  signal_request_count_change(*this, requests_.size());
  if (request_window_ != nullptr)
    request_window_->appendRequest(*request);

  const char *proto = purple_account_get_protocol_name(account);
  const char *uname = purple_account_get_username(account);
  if (alias != nullptr)
    LOG->message(_("+ [%s] %s: New authorization request from %s (%s)"), proto,
      uname, remote_user, alias);
  else
    LOG->message(_("+ [%s] %s: New authorization request from %s"), proto,
      uname, remote_user);

  return request;
}

void Accounts::close_account_request(void *ui_handle)
{
  Requests::iterator i =
    std::find(requests_.begin(), requests_.end(), ui_handle);
  g_return_if_fail(i == requests_.end());

  // This code path is only for auth requests.
  g_assert(typeid(*i) == typeid(AuthRequest *));

  if (request_window_ != nullptr)
    request_window_->removeRequest(**i);
  delete *i;
  requests_.erase(i);
  signal_request_count_change(*this, requests_.size());
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
