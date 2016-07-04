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

#ifndef REQUEST_H
#define REQUEST_H

#include <cppconsui/CheckBox.h>
#include <cppconsui/ComboBox.h>
#include <cppconsui/InputDialog.h>
#include <cppconsui/Label.h>
#include <cppconsui/SplitDialog.h>
#include <cppconsui/TextEntry.h>
#include <cppconsui/TreeView.h>
#include <libpurple/purple.h>

#define REQUEST (Request::instance())

class Request {
public:
  static Request *instance();

private:
  class RequestDialog : public CppConsUI::SplitDialog {
  public:
    RequestDialog(const char *title, const char *primary, const char *secondary,
      const char *ok_text, GCallback ok_cb, const char *cancel_text,
      GCallback cancel_cb, void *user_data);
    virtual ~RequestDialog() override {}

    // FreeWindow
    virtual void onScreenResized() override;

    virtual PurpleRequestType getRequestType() = 0;

  protected:
    GCallback ok_cb_;
    GCallback cancel_cb_;
    void *user_data_;

    // Convenience variable, same as dynamic_cast<ListBox *>(container).
    CppConsUI::ListBox *lbox_;

    virtual void responseHandler(
      CppConsUI::SplitDialog &activator, ResponseType response) = 0;

  private:
    CONSUI_DISABLE_COPY(RequestDialog);
  };

  class InputTextDialog : public RequestDialog {
  public:
    InputTextDialog(const char *title, const char *primary,
      const char *secondary, const char *default_value, bool masked,
      const char *ok_text, GCallback ok_cb, const char *cancel_text,
      GCallback cancel_cb, void *user_data);
    virtual ~InputTextDialog() override {}

    // RequestDialog
    virtual PurpleRequestType getRequestType() override;

  protected:
    CppConsUI::TextEntry *entry_;

    // RequestDialog
    virtual void responseHandler(
      SplitDialog &activator, ResponseType response) override;

  private:
    CONSUI_DISABLE_COPY(InputTextDialog);
  };

  class ChoiceDialog : public RequestDialog {
  public:
    ChoiceDialog(const char *title, const char *primary, const char *secondary,
      int default_value, const char *ok_text, GCallback ok_cb,
      const char *cancel_text, GCallback cancel_cb, void *user_data,
      va_list choices);
    virtual ~ChoiceDialog() override {}

    // RequestDialog
    virtual PurpleRequestType getRequestType() override;

  protected:
    CppConsUI::ComboBox *combo_;

    // RequestDialog
    virtual void responseHandler(
      SplitDialog &activator, ResponseType response) override;

  private:
    CONSUI_DISABLE_COPY(ChoiceDialog);
  };

  class ActionDialog : public RequestDialog {
  public:
    ActionDialog(const char *title, const char *primary, const char *secondary,
      int default_value, void *user_data, size_t action_count, va_list actions);
    virtual ~ActionDialog() override {}

    // RequestDialog
    virtual PurpleRequestType getRequestType() override;

  protected:
    // RequestDialog
    virtual void responseHandler(
      SplitDialog &activator, ResponseType response) override;

  private:
    CONSUI_DISABLE_COPY(ActionDialog);

    void onActionChoice(CppConsUI::Button &activator, size_t i, GCallback cb);
  };

  class FieldsDialog : public RequestDialog {
  public:
    FieldsDialog(const char *title, const char *primary, const char *secondary,
      PurpleRequestFields *request_fields, const char *ok_text, GCallback ok_cb,
      const char *cancel_text, GCallback cancel_cb, void *user_data);
    virtual ~FieldsDialog() override {}

    // RequestDialog
    virtual PurpleRequestType getRequestType() override;

  protected:
    PurpleRequestFields *fields_;
    CppConsUI::TreeView *treeview_;

    class StringField : public CppConsUI::Button {
    public:
      StringField(PurpleRequestField *field);
      virtual ~StringField() override {}

    protected:
      PurpleRequestField *field_;

      void onActivate(CppConsUI::Button &activator);
      void responseHandler(CppConsUI::InputDialog &activator,
        CppConsUI::AbstractDialog::ResponseType response);

    private:
      CONSUI_DISABLE_COPY(StringField);
    };

    class IntegerField : public CppConsUI::Button {
    public:
      IntegerField(PurpleRequestField *field);
      virtual ~IntegerField() override {}

    protected:
      PurpleRequestField *field_;

      void onActivate(CppConsUI::Button &activator);
      void responseHandler(CppConsUI::InputDialog &activator,
        CppConsUI::AbstractDialog::ResponseType response);

    private:
      CONSUI_DISABLE_COPY(IntegerField);
    };

    class BooleanField : public CppConsUI::CheckBox {
    public:
      BooleanField(PurpleRequestField *field);
      virtual ~BooleanField() override {}

    protected:
      PurpleRequestField *field_;

      void onToggle(CppConsUI::CheckBox &activator, bool new_state);

    private:
      CONSUI_DISABLE_COPY(BooleanField);
    };

    class ChoiceField : public CppConsUI::ComboBox {
    public:
      ChoiceField(PurpleRequestField *field);
      virtual ~ChoiceField() override {}

    protected:
      PurpleRequestField *field_;

      void onSelectionChanged(CppConsUI::ComboBox &activator, int new_entry,
        const char *title, intptr_t data);

    private:
      CONSUI_DISABLE_COPY(ChoiceField);
    };

    class ListFieldMultiple : public CppConsUI::ListBox {
    public:
      ListFieldMultiple(PurpleRequestField *field);
      virtual ~ListFieldMultiple() override {}

    protected:
      PurpleRequestField *field_;

      class ListFieldItem : public CppConsUI::CheckBox {
      public:
        ListFieldItem(PurpleRequestField *field, const char *text);
        virtual ~ListFieldItem() override {}

      protected:
        PurpleRequestField *field_;

        void onToggle(CppConsUI::CheckBox &activator, bool new_state);

      private:
        CONSUI_DISABLE_COPY(ListFieldItem);
      };

    private:
      CONSUI_DISABLE_COPY(ListFieldMultiple);
    };

    class ListFieldSingle : public CppConsUI::ComboBox {
    public:
      ListFieldSingle(PurpleRequestField *field);
      virtual ~ListFieldSingle() override {}

    protected:
      PurpleRequestField *field_;

      void onSelectionChanged(CppConsUI::ComboBox &activator, int new_entry,
        const char *title, intptr_t data);

    private:
      CONSUI_DISABLE_COPY(ListFieldSingle);
    };

    class LabelField : public CppConsUI::Label {
    public:
      LabelField(PurpleRequestField *field);
      virtual ~LabelField() override {}

    protected:
      PurpleRequestField *field_;

    private:
      CONSUI_DISABLE_COPY(LabelField);
    };

    class ImageField : public CppConsUI::Button {
    public:
      ImageField(PurpleRequestField *field);
      virtual ~ImageField() override {}

    protected:
      PurpleRequestField *field_;

      void onActivate(CppConsUI::Button &activator);

    private:
      CONSUI_DISABLE_COPY(ImageField);
    };

    class AccountField : public CppConsUI::ComboBox {
    public:
      AccountField(PurpleRequestField *field);
      virtual ~AccountField() override {}

    protected:
      PurpleRequestField *field_;

      void onAccountChanged(CppConsUI::Button &activator, size_t new_entry,
        const char *title, intptr_t data);

    private:
      CONSUI_DISABLE_COPY(AccountField);
    };

    // RequestDialog
    virtual void responseHandler(
      CppConsUI::SplitDialog &activator, ResponseType response) override;

  private:
    CONSUI_DISABLE_COPY(FieldsDialog);
  };

  typedef std::set<RequestDialog *> Requests;

  Requests requests_;

  PurpleRequestUiOps centerim_request_ui_ops_;

  static Request *my_instance_;

  Request();
  ~Request();
  CONSUI_DISABLE_COPY(Request);

  static void init();
  static void finalize();
  friend class CenterIM;

  void onDialogResponse(CppConsUI::SplitDialog &dialog,
    CppConsUI::AbstractDialog::ResponseType response);

  static void *request_input_(const char *title, const char *primary,
    const char *secondary, const char *default_value, gboolean multiline,
    gboolean masked, char *hint, const char *ok_text, GCallback ok_cb,
    const char *cancel_text, GCallback cancel_cb, PurpleAccount *account,
    const char *who, PurpleConversation *conv, void *user_data)
  {
    return REQUEST->request_input(title, primary, secondary, default_value,
      multiline, masked, hint, ok_text, ok_cb, cancel_text, cancel_cb, account,
      who, conv, user_data);
  }
  static void *request_choice_(const char *title, const char *primary,
    const char *secondary, int default_value, const char *ok_text,
    GCallback ok_cb, const char *cancel_text, GCallback cancel_cb,
    PurpleAccount *account, const char *who, PurpleConversation *conv,
    void *user_data, va_list choices)
  {
    return REQUEST->request_choice(title, primary, secondary, default_value,
      ok_text, ok_cb, cancel_text, cancel_cb, account, who, conv, user_data,
      choices);
  }
  static void *request_action_(const char *title, const char *primary,
    const char *secondary, int default_action, PurpleAccount *account,
    const char *who, PurpleConversation *conv, void *user_data,
    size_t action_count, va_list actions)
  {
    return REQUEST->request_action(title, primary, secondary, default_action,
      account, who, conv, user_data, action_count, actions);
  }
  static void *request_fields_(const char *title, const char *primary,
    const char *secondary, PurpleRequestFields *fields, const char *ok_text,
    GCallback ok_cb, const char *cancel_text, GCallback cancel_cb,
    PurpleAccount *account, const char *who, PurpleConversation *conv,
    void *user_data)
  {
    return REQUEST->request_fields(title, primary, secondary, fields, ok_text,
      ok_cb, cancel_text, cancel_cb, account, who, conv, user_data);
  }
  static void *request_file_(const char *title, const char *filename,
    gboolean savedialog, GCallback ok_cb, GCallback cancel_cb,
    PurpleAccount *account, const char *who, PurpleConversation *conv,
    void *user_data)
  {
    return REQUEST->request_file(title, filename, savedialog, ok_cb, cancel_cb,
      account, who, conv, user_data);
  }
  static void close_request_(PurpleRequestType type, void *ui_handle)
  {
    REQUEST->close_request(type, ui_handle);
  }
  static void *request_folder_(const char *title, const char *dirname,
    GCallback ok_cb, GCallback cancel_cb, PurpleAccount *account,
    const char *who, PurpleConversation *conv, void *user_data)
  {
    return REQUEST->request_folder(
      title, dirname, ok_cb, cancel_cb, account, who, conv, user_data);
  }
  static void *request_action_with_icon_(const char *title, const char *primary,
    const char *secondary, int default_action, PurpleAccount *account,
    const char *who, PurpleConversation *conv, gconstpointer icon_data,
    gsize icon_size, void *user_data, size_t action_count, va_list actions)
  {
    return REQUEST->request_action_with_icon(title, primary, secondary,
      default_action, account, who, conv, icon_data, icon_size, user_data,
      action_count, actions);
  }

  void *request_input(const char *title, const char *primary,
    const char *secondary, const char *default_value, gboolean multiline,
    gboolean masked, char *hint, const char *ok_text, GCallback ok_cb,
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
  void *request_folder(const char *title, const char *dirname, GCallback ok_cb,
    GCallback cancel_cb, PurpleAccount *account, const char *who,
    PurpleConversation *conv, void *user_data);
  void *request_action_with_icon(const char *title, const char *primary,
    const char *secondary, int default_action, PurpleAccount *account,
    const char *who, PurpleConversation *conv, gconstpointer icon_data,
    gsize icon_size, void *user_data, size_t action_count, va_list actions);
};

#endif // REQUEST_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
