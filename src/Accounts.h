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

#ifndef __ACCOUNTS_H__
#define __ACCOUNTS_H__

#include <cppconsui/SplitDialog.h>
#include <cppconsui/TreeView.h>
#include <libpurple/purple.h>

#define ACCOUNTS (Accounts::instance())

class Accounts
{
public:
  static Accounts *instance();

  void restoreStatuses(bool offline);

  void openPendingRequests();

  // interface for Header
  sigc::signal<void, Accounts &, size_t> signal_request_count_change;

protected:

private:
  struct Request
  {
    PurpleAccount *account;
    char *remote_user;
    char *id;
    char *alias;

    Request(PurpleAccount *account_, const char *remote_user_,
        const char *id_, const char *alias_);
    virtual ~Request();

  private:
    CONSUI_DISABLE_COPY(Request);
  };

  struct AddRequest
  : public Request
  {
    AddRequest(PurpleAccount *account_, const char *remote_user_,
        const char *id_, const char *alias_);
    virtual ~AddRequest() {}
  };

  struct AuthRequest
  : public Request
  {
    char *message;
    PurpleAccountRequestAuthorizationCb auth_cb;
    PurpleAccountRequestAuthorizationCb deny_cb;
    void *data;

    AuthRequest(PurpleAccount *account_, const char *remote_user_,
        const char *id_, const char *alias_, const char *message_,
        bool on_list_, PurpleAccountRequestAuthorizationCb auth_cb_,
        PurpleAccountRequestAuthorizationCb deny_cb_, void *data_);
    virtual ~AuthRequest();
  };

  typedef std::vector<Request *> Requests;

  class PendingRequestWindow
  : public CppConsUI::SplitDialog
  {
  public:
    PendingRequestWindow(Accounts &accounts_, const Requests &requests);
    virtual ~PendingRequestWindow() {}

    // Window
    virtual void onScreenResized();

    /* Provide a way for the Accounts singleton to add or delete requests
     * dynamically when these events occur. */
    void appendRequest(const Request &request);
    void removeRequest(const Request &request);

  protected:
    class RequestDialog
    : public CppConsUI::AbstractDialog
    {
    public:
      RequestDialog(const Request &request_, const char *title,
          const char *text);
      virtual ~RequestDialog() {}

      const Request *getRequest() const { return request; }

      sigc::signal<void, RequestDialog &, ResponseType> signal_response;

    protected:
      // the request represented by the dialog
      const Request *request;

      // AbstractDialog
      virtual void emitResponse(ResponseType response);

    private:
      CONSUI_DISABLE_COPY(RequestDialog);
    };

    typedef std::map<const Request *, CppConsUI::TreeView::NodeReference>
      RequestMap;

    CppConsUI::TreeView *treeview;
    RequestMap request_map;
    Accounts *accounts;
    RequestDialog *dialog;

    void onActivate(CppConsUI::Button &activator, const Request &request);
    void onAddResponse(RequestDialog &activator, ResponseType response);
    void onAuthResponse(RequestDialog &activator, ResponseType response);

  private:
    CONSUI_DISABLE_COPY(PendingRequestWindow);
  };

  PurpleAccountUiOps centerim_account_ui_ops;
  Requests requests;
  PendingRequestWindow *request_window;

  static Accounts *my_instance;

  Accounts();
  ~Accounts();
  CONSUI_DISABLE_COPY(Accounts);

  static void init();
  static void finalize();
  friend class CenterIM;

  /* This method is called by PendingRequestWindow when an add or auth request
   * should be closed. */
  void closeRequest(const Request &request);
  void onPendingRequestWindowClose(CppConsUI::Window &activator);

  static void notify_added_(PurpleAccount *account, const char *remote_user,
      const char *id, const char *alias, const char *message)
    { ACCOUNTS->notify_added(account, remote_user, id, alias, message); }
  static void status_changed_(PurpleAccount *account, PurpleStatus *status)
    { ACCOUNTS->status_changed(account, status); }
  static void request_add_(PurpleAccount *account, const char *remote_user,
      const char *id, const char *alias, const char *message)
    { ACCOUNTS->request_add(account, remote_user, id, alias, message); }
  static void *request_authorize_(PurpleAccount *account,
      const char *remote_user, const char *id, const char *alias,
      const char *message, gboolean on_list,
      PurpleAccountRequestAuthorizationCb authorize_cb,
      PurpleAccountRequestAuthorizationCb deny_cb, void *user_data)
    { return ACCOUNTS->request_authorize(account, remote_user, id, alias,
        message, on_list, authorize_cb, deny_cb, user_data); }
  static void close_account_request_(void *ui_handle)
    { ACCOUNTS->close_account_request(ui_handle); }

  void notify_added(PurpleAccount *account, const char *remote_user,
      const char *id, const char *alias, const char *message);
  void status_changed(PurpleAccount *account, PurpleStatus *status);
  void request_add(PurpleAccount *account, const char *remote_user,
      const char *id, const char *alias, const char *message);
  void *request_authorize(PurpleAccount *account, const char *remote_user,
      const char *id, const char *alias, const char *message,
      gboolean on_list, PurpleAccountRequestAuthorizationCb authorize_cb,
      PurpleAccountRequestAuthorizationCb deny_cb, void *user_data);
  void close_account_request(void *ui_handle);
};

#endif // __ACCOUNTS_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
