/*
 * Copyright (C) 2010 by CenterIM developers
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

#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <cppconsui/CheckBox.h>
#include <cppconsui/ComboBox.h>
#include <cppconsui/InputDialog.h>
#include <cppconsui/TextEntry.h>
#include <cppconsui/TreeView.h>
#include <cppconsui/SplitDialog.h>
#include <libpurple/purple.h>

#define REQUEST (Request::Instance())

class Request
{
public:
  static Request *Instance();

protected:

private:
  class RequestDialog
  : public SplitDialog
  {
  public:
    RequestDialog(const gchar *title, const gchar *primary,
        const gchar *secondary, const gchar *ok_text, GCallback ok_cb,
        const gchar *cancel_text, GCallback cancel_cb, void *user_data);
    virtual ~RequestDialog() {}

    // FreeWindow
    virtual void ScreenResized();

    virtual PurpleRequestType GetRequestType() = 0;

  protected:
    GCallback ok_cb;
    GCallback cancel_cb;
    void *user_data;

    // convenient var, same as dynamic_cast<ListBox *>(container)
    ListBox *lbox;

    virtual void ResponseHandler(SplitDialog& activator,
        ResponseType response) = 0;

  private:
    RequestDialog(const RequestDialog&);
    RequestDialog& operator=(const RequestDialog&);
  };

  class InputTextDialog
  : public RequestDialog
  {
  public:
    InputTextDialog(const gchar *title, const gchar *primary,
        const gchar *secondary, const gchar *default_value, bool masked,
        const gchar *ok_text, GCallback ok_cb, const gchar *cancel_text,
        GCallback cancel_cb, void *user_data);
    virtual ~InputTextDialog() {}

    virtual PurpleRequestType GetRequestType();

  protected:
    TextEntry *entry;

    virtual void ResponseHandler(SplitDialog& activator,
        ResponseType response);

  private:
    InputTextDialog(const InputTextDialog&);
    InputTextDialog& operator=(const InputTextDialog&);
  };

  class ChoiceDialog
  : public RequestDialog
  {
  public:
    ChoiceDialog(const gchar *title, const gchar *primary,
        const gchar *secondary, int default_value, const gchar *ok_text,
        GCallback ok_cb, const gchar *cancel_text, GCallback cancel_cb,
        void *user_data, va_list choices);
    virtual ~ChoiceDialog() {}

    virtual PurpleRequestType GetRequestType();

  protected:
    ComboBox *combo;

    virtual void ResponseHandler(SplitDialog& activator,
        ResponseType response);

  private:
    ChoiceDialog(const ChoiceDialog&);
    ChoiceDialog& operator=(const ChoiceDialog&);
  };

  class ActionDialog
  : public RequestDialog
  {
  public:
    ActionDialog(const gchar *title, const gchar *primary,
        const gchar *secondary, int default_value, void *user_data,
        size_t action_count, va_list actions);
    virtual ~ActionDialog() {}

    virtual PurpleRequestType GetRequestType();

  protected:
    virtual void ResponseHandler(SplitDialog& activator,
        ResponseType response);

  private:
    ActionDialog(const ActionDialog&);
    ActionDialog& operator=(const ActionDialog&);

    void OnActionChoice(Button& activator, size_t i, GCallback cb);
  };

  class FieldsDialog
  : public RequestDialog
  {
  public:
    FieldsDialog(const gchar *title, const gchar *primary,
        const gchar *secondary, PurpleRequestFields *request_fields,
        const gchar *ok_text, GCallback ok_cb, const gchar *cancel_text,
        GCallback cancel_cb, void *user_data);
    virtual ~FieldsDialog() {}

    virtual PurpleRequestType GetRequestType();

  protected:
    PurpleRequestFields *fields;
    TreeView *tree;

    class StringField
    : public Button
    {
    public:
      StringField(PurpleRequestField *field);
      virtual ~StringField() {}

    protected:
      PurpleRequestField *field;

      void OnActivate(Button& activator);
      void ResponseHandler(InputDialog& activator,
          AbstractDialog::ResponseType response);

    private:
      StringField(const StringField&);
      StringField& operator=(const StringField&);
    };

    class IntegerField
    : public Button
    {
    public:
      IntegerField(PurpleRequestField *field);
      virtual ~IntegerField() {}

    protected:
      PurpleRequestField *field;

      void OnActivate(Button& activator);
      void ResponseHandler(InputDialog& activator,
          AbstractDialog::ResponseType response);

    private:
      IntegerField(const IntegerField&);
      IntegerField& operator=(const IntegerField&);
    };

    class BooleanField
    : public CheckBox
    {
    public:
      BooleanField(PurpleRequestField *field);
      virtual ~BooleanField() {}

    protected:
      PurpleRequestField *field;

      void OnToggle(CheckBox& activator, bool new_state);

    private:
      BooleanField(const BooleanField&);
      BooleanField& operator=(const BooleanField&);
    };

    class ChoiceField
    : public ComboBox
    {
    public:
      ChoiceField(PurpleRequestField *field);
      virtual ~ChoiceField() {}

    protected:
      PurpleRequestField *field;

      void OnSelectionChanged(ComboBox& activator, int new_entry,
          const gchar *title, intptr_t data);

    private:
      ChoiceField(const ChoiceField&);
      ChoiceField& operator=(const ChoiceField&);
    };

    class ListFieldMultiple
    : public ListBox
    {
    public:
      ListFieldMultiple(PurpleRequestField *field);
      virtual ~ListFieldMultiple() {}

    protected:
      PurpleRequestField *field;

      class ListFieldItem
      : public CheckBox
      {
      public:
        ListFieldItem(PurpleRequestField *field, const gchar *text);
        virtual ~ListFieldItem() {}

      protected:
        PurpleRequestField *field;

        void OnToggle(CheckBox& activator, bool new_state);

      private:
        ListFieldItem(const ListFieldItem&);
        ListFieldItem& operator=(const ListFieldItem&);
      };

    private:
      ListFieldMultiple(const ListFieldMultiple&);
      ListFieldMultiple& operator=(const ListFieldMultiple&);
    };

    class ListFieldSingle
    : public ComboBox
    {
    public:
      ListFieldSingle(PurpleRequestField *field);
      virtual ~ListFieldSingle() {}

    protected:
      PurpleRequestField *field;

      void OnSelectionChanged(ComboBox& activator, int new_entry,
          const gchar *title, intptr_t data);

    private:
      ListFieldSingle(const ListFieldSingle&);
      ListFieldSingle& operator=(const ListFieldSingle&);
    };

    class LabelField
    : public Label
    {
    public:
      LabelField(PurpleRequestField *field);
      virtual ~LabelField() {}

    protected:
      PurpleRequestField *field;

    private:
      LabelField(const LabelField&);
      LabelField& operator=(const LabelField&);
    };

    class ImageField
    : public Button
    {
    public:
      ImageField(PurpleRequestField *field);
      virtual ~ImageField() {}

    protected:
      PurpleRequestField *field;

      void OnActivate(Button& activator);

    private:
      ImageField(const ImageField&);
      ImageField& operator=(const ImageField&);
    };

    class AccountField
    : public ComboBox
    {
    public:
      AccountField(PurpleRequestField *field);
      virtual ~AccountField() {}

    protected:
      PurpleRequestField *field;

      void OnAccountChanged(Button& activator, size_t new_entry,
          const gchar *title, intptr_t data);

    private:
      AccountField(const AccountField&);
      AccountField& operator=(const AccountField&);
    };

    virtual void ResponseHandler(SplitDialog& activator,
        ResponseType response);

  private:
    FieldsDialog(const FieldsDialog&);
    FieldsDialog& operator=(const FieldsDialog&);
  };

  typedef std::set<RequestDialog *> Requests;

  Requests requests;

  PurpleRequestUiOps centerim_request_ui_ops;

  static Request *instance;

  Request();
  Request(const Request&);
  Request& operator=(const Request&);
  ~Request();

  static void Init();
  static void Finalize();
  friend class CenterIM;

  void OnDialogResponse(SplitDialog& dialog,
      AbstractDialog::ResponseType response);

  static void *request_input_(const char *title, const char *primary,
      const char *secondary, const char *default_value, gboolean multiline,
      gboolean masked, gchar *hint, const char *ok_text, GCallback ok_cb,
      const char *cancel_text, GCallback cancel_cb, PurpleAccount *account,
      const char *who, PurpleConversation *conv, void *user_data)
    { return REQUEST->request_input(title, primary, secondary, default_value,
        multiline, masked, hint, ok_text, ok_cb, cancel_text, cancel_cb,
        account, who, conv, user_data); }
  static void *request_choice_(const char *title, const char *primary,
      const char *secondary, int default_value, const char *ok_text,
      GCallback ok_cb, const char *cancel_text, GCallback cancel_cb,
      PurpleAccount *account, const char *who, PurpleConversation *conv,
      void *user_data, va_list choices)
    { return REQUEST->request_choice(title, primary, secondary, default_value,
        ok_text, ok_cb, cancel_text, cancel_cb, account, who, conv, user_data,
        choices); }
  static void *request_action_(const char *title, const char *primary,
      const char *secondary, int default_action, PurpleAccount *account,
      const char *who, PurpleConversation *conv, void *user_data,
      size_t action_count, va_list actions)
    { return REQUEST->request_action(title, primary, secondary,
        default_action, account, who, conv, user_data, action_count,
        actions); }
  static void *request_fields_(const char *title, const char *primary,
      const char *secondary, PurpleRequestFields *fields, const char *ok_text,
      GCallback ok_cb, const char *cancel_text, GCallback cancel_cb,
      PurpleAccount *account, const char *who, PurpleConversation *conv,
      void *user_data)
    { return REQUEST->request_fields(title, primary, secondary, fields,
        ok_text, ok_cb, cancel_text, cancel_cb, account, who, conv,
        user_data); }
  static void *request_file_(const char *title, const char *filename,
      gboolean savedialog, GCallback ok_cb, GCallback cancel_cb,
      PurpleAccount *account, const char *who, PurpleConversation *conv,
      void *user_data)
    { return REQUEST->request_file(title, filename, savedialog, ok_cb,
        cancel_cb, account, who, conv, user_data); }
  static void close_request_(PurpleRequestType type, void *ui_handle)
    { REQUEST->close_request(type, ui_handle); }
  static void *request_folder_(const char *title, const char *dirname,
      GCallback ok_cb, GCallback cancel_cb, PurpleAccount *account,
      const char *who, PurpleConversation *conv, void *user_data)
    { return REQUEST->request_folder(title, dirname, ok_cb, cancel_cb,
        account, who, conv, user_data); }
  static void *request_action_with_icon_(const char *title,
      const char *primary, const char *secondary, int default_action,
      PurpleAccount *account, const char *who, PurpleConversation *conv,
      gconstpointer icon_data, gsize icon_size, void *user_data,
      size_t action_count, va_list actions)
    { return REQUEST->request_action_with_icon(title, primary, secondary,
        default_action, account, who, conv, icon_data, icon_size, user_data,
        action_count, actions); }

  void *request_input(const char *title, const char *primary,
      const char *secondary, const char *default_value, gboolean multiline,
      gboolean masked, gchar *hint, const char *ok_text, GCallback ok_cb,
      const char *cancel_text, GCallback cancel_cb, PurpleAccount *account,
      const char *who, PurpleConversation *conv, void *user_data);
  void *request_choice(const char *title, const char *primary,
      const char *secondary, int default_value, const char *ok_text,
      GCallback ok_cb, const char *cancel_text, GCallback cancel_cb,
      PurpleAccount *account, const char *who, PurpleConversation *conv,
      void *user_data, va_list choices);
  void *request_action(const char *title, const char *primary,
      const char *secondary, int default_action, PurpleAccount *account,
      const char *who, PurpleConversation *conv, void *user_data,
      size_t action_count, va_list actions);
  void *request_fields(const char *title, const char *primary,
      const char *secondary, PurpleRequestFields *fields, const char *ok_text,
      GCallback ok_cb, const char *cancel_text, GCallback cancel_cb,
      PurpleAccount *account, const char *who, PurpleConversation *conv,
      void *user_data);
  void *request_file(const char *title, const char *filename,
      gboolean savedialog, GCallback ok_cb, GCallback cancel_cb,
      PurpleAccount *account, const char *who, PurpleConversation *conv,
      void *user_data);
  void close_request(PurpleRequestType type, void *ui_handle);
  void *request_folder(const char *title, const char *dirname,
      GCallback ok_cb, GCallback cancel_cb, PurpleAccount *account,
      const char *who, PurpleConversation *conv, void *user_data);
  void *request_action_with_icon(const char *title, const char *primary,
      const char *secondary, int default_action, PurpleAccount *account,
      const char *who, PurpleConversation *conv, gconstpointer icon_data,
      gsize icon_size, void *user_data, size_t action_count, va_list actions);
};

#endif // __REQUEST_H__
