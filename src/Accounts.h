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

#ifndef __ACCOUNTS_H__
#define __ACCOUNTS_H__

#include <cppconsui/SplitDialog.h>
#include <cppconsui/TreeView.h>
#include <libpurple/purple.h>

#define ACCOUNTS (Accounts::Instance())

class Accounts
{
public:
  static Accounts *Instance();

  void OpenPendingRequests();

  // interface for Header
  sigc::signal<void, Accounts&, size_t> signal_request_count_change;

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
    Request(const Request&);
    Request& operator=(const Request&);
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

  typedef std::vector<Request*> Requests;

  class PendingRequestWindow
  : public CppConsUI::SplitDialog
  {
  public:
    explicit PendingRequestWindow(Accounts& accounts);
    virtual ~PendingRequestWindow() {}

    // FreeWindow
    virtual void OnScreenResized();

  protected:
    class RequestDialog
    : public CppConsUI::AbstractDialog
    {
    public:
      RequestDialog(const char *title, const char *text);
      virtual ~RequestDialog() {}

      sigc::signal<void, RequestDialog&, ResponseType> signal_response;

    protected:
      // AbstractDialog
      virtual void EmitResponse(ResponseType response);

    private:
      RequestDialog(const RequestDialog&);
      RequestDialog& operator=(const RequestDialog&);
    };

    typedef std::map<const Request*, CppConsUI::TreeView::NodeReference>
      RequestMap;

    CppConsUI::TreeView *requests;
    RequestMap request_map;
    Accounts *accounts;

    void Populate();
    void AppendRequest(const Request& request);
    void OnRequestAdd(Accounts& accounts, const Request& request);
    void OnRequestRemove(Accounts& accounts, const Request& request);

    void OnActivate(CppConsUI::Button& activator, const Request& request);
    void OnAddResponse(RequestDialog& activator, ResponseType response,
        const AddRequest& request);
    void OnAuthResponse(RequestDialog& activator, ResponseType response,
        const AuthRequest& request);

  private:
    PendingRequestWindow(const PendingRequestWindow&);
    PendingRequestWindow& operator=(const PendingRequestWindow&);
  };

  PurpleAccountUiOps centerim_account_ui_ops;

  Requests requests;

  static Accounts *instance;

  Accounts();
  Accounts(const Accounts&);
  Accounts& operator=(const Accounts&);
  ~Accounts();

  static void Init();
  static void Finalize();
  friend class CenterIM;

  // interface for PendingRequestWindow
  const Requests *GetRequests() const { return &requests; };
  void RemoveRequest(const Request& request);
  sigc::signal<void, Accounts&, const Request&> signal_request_add;
  sigc::signal<void, Accounts&, const Request&> signal_request_remove;

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
