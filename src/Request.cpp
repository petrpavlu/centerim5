/*
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

#include "Request.h"

#include "CenterIM.h"
#include "Log.h"

#include <cppconsui/InputDialog.h>
#include <cppconsui/Spacer.h>
#include <string.h>
#include <errno.h>
#include "gettext.h"

Request *Request::instance = NULL;

Request *Request::Instance()
{
  return instance;
}

Request::RequestDialog::RequestDialog(const char *title,
    const char *primary, const char *secondary, const char *ok_text,
    GCallback ok_cb, const char *cancel_text, GCallback cancel_cb,
    void *user_data)
: SplitDialog(title), ok_cb(ok_cb), cancel_cb(cancel_cb), user_data(user_data)
{
  SetColorScheme("generalwindow");

  lbox = new CppConsUI::ListBox(AUTOSIZE, AUTOSIZE);
  if (primary)
    lbox->AppendWidget(*(new CppConsUI::Label(primary)));
  if (primary && secondary)
    lbox->AppendWidget(*(new CppConsUI::Spacer(AUTOSIZE, 1)));
  if (secondary)
    lbox->AppendWidget(*(new CppConsUI::Label(secondary)));
  if (primary || secondary)
    lbox->AppendWidget(*(new CppConsUI::HorizontalLine(AUTOSIZE)));
  SetContainer(*lbox);

  if (ok_text)
    AddButton(ok_text, RESPONSE_OK);
  if (ok_text && cancel_text)
    AddSeparator();
  if (cancel_text)
    AddButton(cancel_text, RESPONSE_CANCEL);
  signal_response.connect(sigc::mem_fun(this,
        &RequestDialog::ResponseHandler));
}

void Request::RequestDialog::ScreenResized()
{
  MoveResizeRect(CENTERIM->GetScreenAreaSize(CenterIM::CHAT_AREA));
}

Request::InputTextDialog::InputTextDialog(const char *title,
    const char *primary, const char *secondary, const char *default_value,
    bool masked, const char *ok_text, GCallback ok_cb,
    const char *cancel_text, GCallback cancel_cb, void *user_data)
: RequestDialog(title, primary, secondary, ok_text, ok_cb, cancel_text,
    cancel_cb, user_data)
{
  entry = new CppConsUI::TextEntry(AUTOSIZE, AUTOSIZE, default_value);
  lbox->AppendWidget(*entry);
  entry->GrabFocus();
}

PurpleRequestType Request::InputTextDialog::GetRequestType()
{
  return PURPLE_REQUEST_INPUT;
}

void Request::InputTextDialog::ResponseHandler(SplitDialog& activator,
    ResponseType response)
{
  switch (response) {
    case AbstractDialog::RESPONSE_OK:
      if (ok_cb)
        reinterpret_cast<PurpleRequestInputCb>(ok_cb)(user_data,
            entry->GetText());
      break;
    case AbstractDialog::RESPONSE_CANCEL:
      if (cancel_cb)
        reinterpret_cast<PurpleRequestInputCb>(cancel_cb)(user_data,
            entry->GetText());
      break;
    default:
      g_assert_not_reached();
      break;
  }
}

Request::ChoiceDialog::ChoiceDialog(const char *title, const char *primary,
    const char *secondary, int default_value, const char *ok_text,
    GCallback ok_cb, const char *cancel_text, GCallback cancel_cb,
    void *user_data, va_list choices)
: RequestDialog(title, primary, secondary, ok_text, ok_cb, cancel_text,
    cancel_cb, user_data)
{
  combo = new CppConsUI::ComboBox(_("Selected value"));
  lbox->AppendWidget(*combo);
  combo->GrabFocus();

  char *text;
  while ((text = va_arg(choices, char*))) {
    int resp = va_arg(choices, int);
    combo->AddOption(text, resp);
  }
  combo->SetSelectedByData(default_value);
}

PurpleRequestType Request::ChoiceDialog::GetRequestType()
{
  return PURPLE_REQUEST_CHOICE;
}

void Request::ChoiceDialog::ResponseHandler(SplitDialog& activator,
    ResponseType response)
{
  size_t selected = combo->GetSelected();
  int data = combo->GetData(selected);

  switch (response) {
    case AbstractDialog::RESPONSE_OK:
      if (ok_cb)
        reinterpret_cast<PurpleRequestChoiceCb>(ok_cb)(user_data,
            data);
      break;
    case AbstractDialog::RESPONSE_CANCEL:
      if (cancel_cb)
        reinterpret_cast<PurpleRequestChoiceCb>(cancel_cb)(user_data,
            data);
      break;
    default:
      g_assert_not_reached();
      break;
  }
}

Request::ActionDialog::ActionDialog(const char *title, const char *primary,
    const char *secondary, int default_value, void *user_data,
    size_t action_count, va_list actions)
: RequestDialog(title, primary, secondary, NULL, NULL, NULL, NULL, user_data)
{
  for (size_t i = 0; i < action_count; i++) {
    char *title = va_arg(actions, char*);
    GCallback cb = va_arg(actions, GCallback);

    CppConsUI::Button *b = buttons->AppendItem(title,
        sigc::bind(sigc::mem_fun(this,
            &Request::ActionDialog::OnActionChoice), i, cb));
    if (static_cast<int>(i) == default_value)
      b->GrabFocus();

    if (i != action_count - 1)
      buttons->AppendSeparator();
  }
}

PurpleRequestType Request::ActionDialog::GetRequestType()
{
  return PURPLE_REQUEST_ACTION;
}

void Request::ActionDialog::ResponseHandler(SplitDialog& activator,
    ResponseType response)
{
}

void Request::ActionDialog::OnActionChoice(CppConsUI::Button& activator,
    size_t i, GCallback cb)
{
  if (cb)
    reinterpret_cast<PurpleRequestActionCb>(cb)(user_data, i);

  Close();
}

Request::FieldsDialog::FieldsDialog(const char *title, const char *primary,
    const char *secondary, PurpleRequestFields *request_fields,
    const char *ok_text, GCallback ok_cb, const char *cancel_text,
    GCallback cancel_cb, void *user_data)
: RequestDialog(title, primary, secondary, ok_text, ok_cb, cancel_text,
    cancel_cb, user_data), fields(request_fields)
{
  tree = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  lbox->AppendWidget(*tree);

  bool grouping = true;
  GList *groups = purple_request_fields_get_groups(fields);
  if (!groups)
    return;
  if (!purple_request_field_group_get_title(
        reinterpret_cast<PurpleRequestFieldGroup*>(groups->data))
      && !groups->next)
    grouping = false;
  for (; groups; groups = groups->next) {
    PurpleRequestFieldGroup *group
      = reinterpret_cast<PurpleRequestFieldGroup*>(groups->data);

    CppConsUI::TreeView::NodeReference parent = tree->GetRootNode();
    if (grouping) {
      const char *title = purple_request_field_group_get_title(group);
      if (!title)
        title = _("Settings group");

      CppConsUI::TreeView::ToggleCollapseButton *button
        = new CppConsUI::TreeView::ToggleCollapseButton(title);
      parent = tree->AppendNode(tree->GetRootNode(), *button);
    }

    for (GList *gfields = purple_request_field_group_get_fields(group);
        gfields; gfields = gfields->next) {
      PurpleRequestField *field
        = reinterpret_cast<PurpleRequestField*>(gfields->data);

      if (!purple_request_field_is_visible(field))
        continue;

      PurpleRequestFieldType type = purple_request_field_get_type(field);

      switch (type) {
        case PURPLE_REQUEST_FIELD_STRING:
          tree->AppendNode(parent, *(new StringField(field)));
          break;
        case PURPLE_REQUEST_FIELD_INTEGER:
          tree->AppendNode(parent, *(new IntegerField(field)));
          break;
        case PURPLE_REQUEST_FIELD_BOOLEAN:
          tree->AppendNode(parent, *(new BooleanField(field)));
          break;
        case PURPLE_REQUEST_FIELD_CHOICE:
          tree->AppendNode(parent, *(new ChoiceField(field)));
          break;
        case PURPLE_REQUEST_FIELD_LIST:
          if (purple_request_field_list_get_multi_select(field))
            tree->AppendNode(parent, *(new ListFieldMultiple(field)));
          else
            tree->AppendNode(parent, *(new ListFieldSingle(field)));
          break;
        case PURPLE_REQUEST_FIELD_LABEL:
          tree->AppendNode(parent, *(new LabelField(field)));
          break;
        case PURPLE_REQUEST_FIELD_IMAGE:
          tree->AppendNode(parent, *(new ImageField(field)));
          break;
        case PURPLE_REQUEST_FIELD_ACCOUNT:
          tree->AppendNode(parent, *(new AccountField(field)));
          break;
        default:
          LOG->Error(_("Unimplemented Request field type."));
          break;
      }
    }
  }
}

PurpleRequestType Request::FieldsDialog::GetRequestType()
{
  return PURPLE_REQUEST_FIELDS;
}

Request::FieldsDialog::StringField::StringField(PurpleRequestField *field)
: Button(FLAG_VALUE), field(field)
{
  g_assert(field);

  if (purple_request_field_string_is_masked(field))
    ; // TODO

  char *text = g_strdup_printf("%s%s",
      purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  SetText(text);
  g_free(text);

  SetValue(purple_request_field_string_get_value(field));
  signal_activate.connect(sigc::mem_fun(this, &StringField::OnActivate));
}

void Request::FieldsDialog::StringField::OnActivate(CppConsUI::Button& activator)
{
  CppConsUI::InputDialog *dialog
    = new CppConsUI::InputDialog(purple_request_field_get_label(field),
      purple_request_field_string_get_value(field));
  dialog->signal_response.connect(sigc::mem_fun(this,
        &StringField::ResponseHandler));
  dialog->Show();
}

void Request::FieldsDialog::StringField::ResponseHandler(
    CppConsUI::InputDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_OK:
      purple_request_field_string_set_value(field, activator.GetText());
      SetValue(purple_request_field_string_get_value(field));
      break;
    default:
      break;
  }
}

Request::FieldsDialog::IntegerField::IntegerField(PurpleRequestField *field)
: Button(FLAG_VALUE), field(field)
{
  g_assert(field);

  char *text = g_strdup_printf("%s%s",
      purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  SetText(text);
  g_free(text);

  SetValue(purple_request_field_int_get_value(field));
  signal_activate.connect(sigc::mem_fun(this, &IntegerField::OnActivate));
}

void Request::FieldsDialog::IntegerField::OnActivate(CppConsUI::Button& activator)
{
  char *value = g_strdup_printf("%d", purple_request_field_int_get_value(field));
  CppConsUI::InputDialog *dialog
    = new CppConsUI::InputDialog(purple_request_field_get_label(field),
        value);
  g_free(value);
  dialog->SetFlags(CppConsUI::TextEntry::FLAG_NUMERIC);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &IntegerField::ResponseHandler));
  dialog->Show();
}

void Request::FieldsDialog::IntegerField::ResponseHandler(
    CppConsUI::InputDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response)
{
  const char *text;
  long int i;

  switch (response) {
    case CppConsUI::AbstractDialog::RESPONSE_OK:
      text = activator.GetText();
      errno = 0;
      i = strtol(text, NULL, 10);
      if (errno == ERANGE || i > INT_MAX || i < INT_MIN)
        LOG->Warning(_("Value is out of range."));
      purple_request_field_int_set_value(field, i);
      SetValue(purple_request_field_int_get_value(field));
      break;
    default:
      break;
  }
}

Request::FieldsDialog::BooleanField::BooleanField(PurpleRequestField *field)
: field(field)
{
  g_assert(field);

  char *text = g_strdup_printf("%s%s",
      purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  SetText(text);
  g_free(text);

  SetState(purple_request_field_bool_get_value(field));
  signal_toggle.connect(sigc::mem_fun(this, &BooleanField::OnToggle));
}

void Request::FieldsDialog::BooleanField::OnToggle(CheckBox& activator,
    bool new_state)
{
  purple_request_field_bool_set_value(field, new_state);
}

Request::FieldsDialog::ChoiceField::ChoiceField(PurpleRequestField *field)
: field(field)
{
  g_assert(field);

  char *text = g_strdup_printf("%s%s",
      purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  SetText(text);
  g_free(text);

  for (GList *list = purple_request_field_choice_get_labels(field); list;
      list = list->next)
    AddOption(reinterpret_cast<const char*>(list->data));
  SetSelected(purple_request_field_choice_get_default_value(field));

  signal_selection_changed.connect(sigc::mem_fun(this,
        &ChoiceField::OnSelectionChanged));
}

void Request::FieldsDialog::ChoiceField::OnSelectionChanged(
    ComboBox& activator, int new_entry, const char *title, intptr_t data)
{
  purple_request_field_choice_set_value(field, new_entry);
}

Request::FieldsDialog::ListFieldMultiple::ListFieldMultiple(
    PurpleRequestField *field)
: ListBox(AUTOSIZE, 1), field(field)
{
  g_assert(field);

  // TODO display label of the field somewhere

  int height = 0;
  for (GList *list = purple_request_field_list_get_items(field); list;
      list = list->next, height++)
    AppendWidget(*(new ListFieldItem(field,
            reinterpret_cast<const char*>(list->data))));
  SetHeight(height);
}

Request::FieldsDialog::ListFieldMultiple::ListFieldItem::ListFieldItem(
    PurpleRequestField *field, const char *text)
: field(field)
{
  g_assert(field);

  SetText(text);
  SetState(purple_request_field_list_is_selected(field, text));
  signal_toggle.connect(sigc::mem_fun(this, &ListFieldItem::OnToggle));
}

void Request::FieldsDialog::ListFieldMultiple::ListFieldItem::OnToggle(
    CheckBox& activator, bool new_state)
{
  if (new_state)
    purple_request_field_list_add_selected(field, GetText());
  else {
    /* XXX This chunk is super-slow, libpurple should provide
     * purple_request_field_list_remove_selected() function. */
    GList *new_selected = NULL;
    for (GList *selected = purple_request_field_list_get_selected(field);
        selected; selected = selected->next) {
      const char *data = reinterpret_cast<const char*>(selected->data);
      if (strcmp(GetText(), data))
        new_selected = g_list_append(new_selected, g_strdup(data));
    }

    if (new_selected) {
      purple_request_field_list_set_selected(field, new_selected);
      g_list_foreach(new_selected, reinterpret_cast<GFunc>(g_free), NULL);
    }
    else
      purple_request_field_list_clear_selected(field);
  }
}

Request::FieldsDialog::ListFieldSingle::ListFieldSingle(
    PurpleRequestField *field)
: field(field)
{
  g_assert(field);

  char *text = g_strdup_printf("%s%s",
      purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  SetText(text);
  g_free(text);

  GList *list = purple_request_field_list_get_items(field);
  for (int i = 0; list; i++, list = list->next) {
    const char *text = reinterpret_cast<const char*>(list->data);
    AddOption(text);
    if (purple_request_field_list_is_selected(field, text))
      SetSelected(i);
  }

  signal_selection_changed.connect(sigc::mem_fun(this,
        &ListFieldSingle::OnSelectionChanged));
}

void Request::FieldsDialog::ListFieldSingle::OnSelectionChanged(
    ComboBox& activator, int new_entry, const char *title, intptr_t data)
{
  purple_request_field_list_clear_selected(field);
  purple_request_field_list_add_selected(field, title);
}

Request::FieldsDialog::LabelField::LabelField(PurpleRequestField *field)
: field(field)
{
  g_assert(field);

  SetText(purple_request_field_get_label(field));
}

Request::FieldsDialog::ImageField::ImageField(PurpleRequestField *field)
: field(field)
{
  g_assert(field);

  char *text = g_strdup_printf("%s%s",
      purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  SetText(text);
  g_free(text);

  // TODO

  signal_activate.connect(sigc::mem_fun(this, &ImageField::OnActivate));
}

void Request::FieldsDialog::ImageField::OnActivate(Button& activator)
{
}

Request::FieldsDialog::AccountField::AccountField(PurpleRequestField *field)
: field(field)
{
  g_assert(field);

  // TODO filter (purple_request_field_account_get_filter())
  // TODO signals (signed-on, signed-off, account-added, account-removed)

  char *text = g_strdup_printf("%s%s",
      purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  SetText(text);
  g_free(text);

  gboolean show_all = purple_request_field_account_get_show_all(field);
  for (GList *list = purple_accounts_get_all(); list; list = list->next) {
    PurpleAccount *account = reinterpret_cast<PurpleAccount*>(list->data);
    if (!show_all && !purple_account_is_connected(account))
      continue;

    char *label = g_strdup_printf("[%s] %s",
        purple_account_get_protocol_name(account),
        purple_account_get_username(account));
    AddOptionPtr(label, account);
    g_free(label);
  }
  SetSelectedByDataPtr(purple_request_field_account_get_default_value(field));

  signal_selection_changed.connect(sigc::mem_fun(this,
        &AccountField::OnAccountChanged));
}

void Request::FieldsDialog::AccountField::OnAccountChanged(Button& activator,
    size_t new_entry, const char *title, intptr_t data)
{
  purple_request_field_account_set_value(field,
      reinterpret_cast<PurpleAccount*>(data));
}

void Request::FieldsDialog::ResponseHandler(SplitDialog& activator,
    ResponseType response)
{
  switch (response) {
    case AbstractDialog::RESPONSE_OK:
      if (ok_cb)
        reinterpret_cast<PurpleRequestFieldsCb>(ok_cb)(user_data, fields);
      break;
    case AbstractDialog::RESPONSE_CANCEL:
      if (cancel_cb)
        reinterpret_cast<PurpleRequestFieldsCb>(cancel_cb)(user_data, fields);
      break;
    default:
      g_assert_not_reached();
      break;
  }

  purple_request_fields_destroy(fields);
}

Request::Request()
{
  memset(&centerim_request_ui_ops, 0, sizeof(centerim_request_ui_ops));

  // set the purple request callbacks
  centerim_request_ui_ops.request_input = request_input_;
  centerim_request_ui_ops.request_choice = request_choice_;
  centerim_request_ui_ops.request_action = request_action_;
  centerim_request_ui_ops.request_fields = request_fields_;
  centerim_request_ui_ops.request_file = request_file_;
  centerim_request_ui_ops.close_request = close_request_;
  centerim_request_ui_ops.request_folder = request_folder_;
  centerim_request_ui_ops.request_action_with_icon
    = request_action_with_icon_;
  purple_request_set_ui_ops(&centerim_request_ui_ops);
}

Request::~Request()
{
  // close all opened requests
  while (requests.size()) {
    RequestDialog *rdialog = *(requests.begin());
    purple_request_close(rdialog->GetRequestType(), rdialog);
  }

  purple_request_set_ui_ops(NULL);
}

void Request::Init()
{
  g_assert(!instance);

  instance = new Request;
}

void Request::Finalize()
{
  g_assert(instance);

  delete instance;
  instance = NULL;
}

void Request::OnDialogResponse(CppConsUI::SplitDialog& dialog,
    CppConsUI::AbstractDialog::ResponseType response)
{
  RequestDialog *rdialog = dynamic_cast<RequestDialog*>(&dialog);
  g_assert(rdialog);

  requests.erase(rdialog);
  purple_request_close(rdialog->GetRequestType(), rdialog);
}

void *Request::request_input(const char *title, const char *primary,
    const char *secondary, const char *default_value, gboolean multiline,
    gboolean masked, char *hint, const char *ok_text, GCallback ok_cb,
    const char *cancel_text, GCallback cancel_cb, PurpleAccount *account,
    const char *who, PurpleConversation *conv, void *user_data)
{
  LOG->Debug("request_input");

  InputTextDialog *dialog = new InputTextDialog(title, primary, secondary,
      default_value, masked, ok_text, ok_cb, cancel_text, cancel_cb,
      user_data);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &Request::OnDialogResponse));
  dialog->Show();

  requests.insert(dialog);
  return dialog;
}

void *Request::request_choice(const char *title, const char *primary,
    const char *secondary, int default_value, const char *ok_text,
    GCallback ok_cb, const char *cancel_text, GCallback cancel_cb,
    PurpleAccount *account, const char *who, PurpleConversation *conv,
    void *user_data, va_list choices)
{
  LOG->Debug("request_choice");

  ChoiceDialog *dialog = new ChoiceDialog(title, primary, secondary,
      default_value, ok_text, ok_cb, cancel_text, cancel_cb, user_data,
      choices);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &Request::OnDialogResponse));
  dialog->Show();

  requests.insert(dialog);
  return dialog;
}

void *Request::request_action(const char *title, const char *primary,
    const char *secondary, int default_action, PurpleAccount *account,
    const char *who, PurpleConversation *conv, void *user_data,
    size_t action_count, va_list actions)
{
  LOG->Debug("request_action");

  ActionDialog *dialog = new ActionDialog(title, primary, secondary,
      default_action, user_data, action_count, actions);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &Request::OnDialogResponse));
  dialog->Show();

  requests.insert(dialog);
  return dialog;
}

void *Request::request_fields(const char *title, const char *primary,
    const char *secondary, PurpleRequestFields *fields,
    const char *ok_text, GCallback ok_cb, const char *cancel_text,
    GCallback cancel_cb, PurpleAccount *account, const char *who,
    PurpleConversation *conv, void *user_data)
{
  LOG->Debug("request_fields");

  FieldsDialog *dialog = new FieldsDialog(title, primary, secondary,
      fields, ok_text, ok_cb, cancel_text, cancel_cb, user_data);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &Request::OnDialogResponse));
  dialog->Show();

  requests.insert(dialog);
  return dialog;
}

void *Request::request_file(const char *title, const char *filename,
    gboolean savedialog, GCallback ok_cb, GCallback cancel_cb,
    PurpleAccount *account, const char *who, PurpleConversation *conv,
    void *user_data)
{
  return NULL;
}

void Request::close_request(PurpleRequestType type, void *ui_handle)
{
  LOG->Debug("close_request");

  g_assert(ui_handle);

  RequestDialog *dialog = reinterpret_cast<RequestDialog*>(ui_handle);
  if (requests.find(dialog) != requests.end()) {
    requests.erase(dialog);
    dialog->Close();
  }
}

void *Request::request_folder(const char *title, const char *dirname,
    GCallback ok_cb, GCallback cancel_cb, PurpleAccount *account,
    const char *who, PurpleConversation *conv, void *user_data)
{
  return NULL;
}

void *Request::request_action_with_icon(const char *title,
    const char *primary, const char *secondary, int default_action,
    PurpleAccount *account, const char *who, PurpleConversation *conv,
    gconstpointer icon_data, gsize icon_size, void *user_data,
    size_t action_count, va_list actions)
{
  return NULL;
}
