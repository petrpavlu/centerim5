/*
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

#include "Request.h"

#include "Log.h"

#include <cppconsui/InputDialog.h>
#include <cppconsui/Spacer.h>
#include <errno.h>
#include "gettext.h"

Request *Request::my_instance = NULL;

Request *Request::instance()
{
  return my_instance;
}

Request::RequestDialog::RequestDialog(const char *title, const char *primary,
  const char *secondary, const char *ok_text, GCallback ok_cb,
  const char *cancel_text, GCallback cancel_cb, void *user_data)
  : SplitDialog(title), ok_cb(ok_cb), cancel_cb(cancel_cb), user_data(user_data)
{
  setColorScheme("generalwindow");

  lbox = new CppConsUI::ListBox(AUTOSIZE, AUTOSIZE);
  if (primary)
    lbox->appendWidget(*(new CppConsUI::Label(AUTOSIZE, 1, primary)));
  if (primary && secondary)
    lbox->appendWidget(*(new CppConsUI::Spacer(AUTOSIZE, 1)));
  if (secondary)
    lbox->appendWidget(*(new CppConsUI::Label(AUTOSIZE, 1, secondary)));
  if (primary || secondary)
    lbox->appendWidget(*(new CppConsUI::HorizontalLine(AUTOSIZE)));
  setContainer(*lbox);

  if (ok_text)
    addButton(ok_text, RESPONSE_OK);
  if (ok_text && cancel_text)
    addSeparator();
  if (cancel_text)
    addButton(cancel_text, RESPONSE_CANCEL);
  signal_response.connect(sigc::mem_fun(this, &RequestDialog::responseHandler));

  onScreenResized();
}

void Request::RequestDialog::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::CHAT_AREA));
}

Request::InputTextDialog::InputTextDialog(const char *title,
  const char *primary, const char *secondary, const char *default_value,
  bool /*masked*/, const char *ok_text, GCallback ok_cb,
  const char *cancel_text, GCallback cancel_cb, void *user_data)
  : RequestDialog(title, primary, secondary, ok_text, ok_cb, cancel_text,
      cancel_cb, user_data)
{
  entry = new CppConsUI::TextEntry(AUTOSIZE, AUTOSIZE, default_value);
  lbox->appendWidget(*entry);
  entry->grabFocus();
}

PurpleRequestType Request::InputTextDialog::getRequestType()
{
  return PURPLE_REQUEST_INPUT;
}

void Request::InputTextDialog::responseHandler(
  SplitDialog & /*activator*/, ResponseType response)
{
  switch (response) {
  case AbstractDialog::RESPONSE_OK:
    if (ok_cb)
      reinterpret_cast<PurpleRequestInputCb>(ok_cb)(
        user_data, entry->getText());
    break;
  case AbstractDialog::RESPONSE_CANCEL:
    if (cancel_cb)
      reinterpret_cast<PurpleRequestInputCb>(cancel_cb)(
        user_data, entry->getText());
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
  combo = new CppConsUI::ComboBox(AUTOSIZE, 1, _("Selected value"));
  lbox->appendWidget(*combo);
  combo->grabFocus();

  const char *text;
  while ((text = va_arg(choices, const char *))) {
    int resp = va_arg(choices, int);
    combo->addOption(text, resp);
  }
  combo->setSelectedByData(default_value);
}

PurpleRequestType Request::ChoiceDialog::getRequestType()
{
  return PURPLE_REQUEST_CHOICE;
}

void Request::ChoiceDialog::responseHandler(
  SplitDialog & /*activator*/, ResponseType response)
{
  int data = combo->getSelectedData();

  switch (response) {
  case AbstractDialog::RESPONSE_OK:
    if (ok_cb)
      reinterpret_cast<PurpleRequestChoiceCb>(ok_cb)(user_data, data);
    break;
  case AbstractDialog::RESPONSE_CANCEL:
    if (cancel_cb)
      reinterpret_cast<PurpleRequestChoiceCb>(cancel_cb)(user_data, data);
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
    const char *title = va_arg(actions, const char *);
    GCallback cb = va_arg(actions, GCallback);

    CppConsUI::Button *b = buttons->appendItem(title,
      sigc::bind(sigc::mem_fun(this, &ActionDialog::onActionChoice), i, cb));
    if (static_cast<int>(i) == default_value)
      b->grabFocus();

    if (i < action_count - 1)
      buttons->appendSeparator();
  }
}

PurpleRequestType Request::ActionDialog::getRequestType()
{
  return PURPLE_REQUEST_ACTION;
}

void Request::ActionDialog::responseHandler(
  SplitDialog & /*activator*/, ResponseType /*response*/)
{
}

void Request::ActionDialog::onActionChoice(
  CppConsUI::Button & /*activator*/, size_t i, GCallback cb)
{
  if (cb)
    reinterpret_cast<PurpleRequestActionCb>(cb)(user_data, i);

  /* It's possible that the callback action already called
   * purple_request_destroy() in which case 'this' object is already deleted
   * and calling close() in such a case leads to an error. */
  Requests *requests = &REQUEST->requests;
  if (requests->find(this) != requests->end())
    close();
}

Request::FieldsDialog::FieldsDialog(const char *title, const char *primary,
  const char *secondary, PurpleRequestFields *request_fields,
  const char *ok_text, GCallback ok_cb, const char *cancel_text,
  GCallback cancel_cb, void *user_data)
  : RequestDialog(title, primary, secondary, ok_text, ok_cb, cancel_text,
      cancel_cb, user_data),
    fields(request_fields)
{
  treeview = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  lbox->appendWidget(*treeview);

  bool grouping = true;
  GList *groups = purple_request_fields_get_groups(fields);
  if (!groups)
    return;
  if (!purple_request_field_group_get_title(
        static_cast<PurpleRequestFieldGroup *>(groups->data)) &&
    !groups->next)
    grouping = false;
  for (; groups; groups = groups->next) {
    PurpleRequestFieldGroup *group =
      static_cast<PurpleRequestFieldGroup *>(groups->data);

    CppConsUI::TreeView::NodeReference parent = treeview->getRootNode();
    if (grouping) {
      const char *title = purple_request_field_group_get_title(group);
      if (!title)
        title = _("Settings group");

      CppConsUI::TreeView::ToggleCollapseButton *button =
        new CppConsUI::TreeView::ToggleCollapseButton(title);
      parent = treeview->appendNode(treeview->getRootNode(), *button);
    }

    for (GList *gfields = purple_request_field_group_get_fields(group); gfields;
         gfields = gfields->next) {
      PurpleRequestField *field =
        static_cast<PurpleRequestField *>(gfields->data);

      if (!purple_request_field_is_visible(field))
        continue;

      PurpleRequestFieldType type = purple_request_field_get_type(field);

      switch (type) {
      case PURPLE_REQUEST_FIELD_STRING:
        treeview->appendNode(parent, *(new StringField(field)));
        break;
      case PURPLE_REQUEST_FIELD_INTEGER:
        treeview->appendNode(parent, *(new IntegerField(field)));
        break;
      case PURPLE_REQUEST_FIELD_BOOLEAN:
        treeview->appendNode(parent, *(new BooleanField(field)));
        break;
      case PURPLE_REQUEST_FIELD_CHOICE:
        treeview->appendNode(parent, *(new ChoiceField(field)));
        break;
      case PURPLE_REQUEST_FIELD_LIST:
        if (purple_request_field_list_get_multi_select(field))
          treeview->appendNode(parent, *(new ListFieldMultiple(field)));
        else
          treeview->appendNode(parent, *(new ListFieldSingle(field)));
        break;
      case PURPLE_REQUEST_FIELD_LABEL:
        treeview->appendNode(parent, *(new LabelField(field)));
        break;
      case PURPLE_REQUEST_FIELD_IMAGE:
        treeview->appendNode(parent, *(new ImageField(field)));
        break;
      case PURPLE_REQUEST_FIELD_ACCOUNT:
        treeview->appendNode(parent, *(new AccountField(field)));
        break;
      default:
        LOG->error(_("Unhandled request field type '%d'."), type);
        break;
      }
    }
  }

  treeview->grabFocus();
}

PurpleRequestType Request::FieldsDialog::getRequestType()
{
  return PURPLE_REQUEST_FIELDS;
}

Request::FieldsDialog::StringField::StringField(PurpleRequestField *field)
  : Button(FLAG_VALUE), field(field)
{
  g_assert(field);

  if (purple_request_field_string_is_masked(field))
    setMasked(true);

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  setText(text);
  g_free(text);

  setValue(purple_request_field_string_get_value(field));
  signal_activate.connect(sigc::mem_fun(this, &StringField::onActivate));
}

void Request::FieldsDialog::StringField::onActivate(
  CppConsUI::Button & /*activator*/)
{
  CppConsUI::InputDialog *dialog =
    new CppConsUI::InputDialog(purple_request_field_get_label(field),
      purple_request_field_string_get_value(field));
  dialog->setMasked(isMasked());
  dialog->signal_response.connect(
    sigc::mem_fun(this, &StringField::responseHandler));
  dialog->show();
}

void Request::FieldsDialog::StringField::responseHandler(
  CppConsUI::InputDialog &activator,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  purple_request_field_string_set_value(field, activator.getText());
  setValue(purple_request_field_string_get_value(field));
}

Request::FieldsDialog::IntegerField::IntegerField(PurpleRequestField *field)
  : Button(FLAG_VALUE), field(field)
{
  g_assert(field);

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  setText(text);
  g_free(text);

  setValue(purple_request_field_int_get_value(field));
  signal_activate.connect(sigc::mem_fun(this, &IntegerField::onActivate));
}

void Request::FieldsDialog::IntegerField::onActivate(
  CppConsUI::Button & /*activator*/)
{
  char *value =
    g_strdup_printf("%d", purple_request_field_int_get_value(field));
  CppConsUI::InputDialog *dialog =
    new CppConsUI::InputDialog(purple_request_field_get_label(field), value);
  g_free(value);
  dialog->setFlags(CppConsUI::TextEntry::FLAG_NUMERIC);
  dialog->signal_response.connect(
    sigc::mem_fun(this, &IntegerField::responseHandler));
  dialog->show();
}

void Request::FieldsDialog::IntegerField::responseHandler(
  CppConsUI::InputDialog &activator,
  CppConsUI::AbstractDialog::ResponseType response)
{
  if (response != AbstractDialog::RESPONSE_OK)
    return;

  const char *text = activator.getText();
  errno = 0;
  long i = strtol(text, NULL, 10);
  if (errno == ERANGE || i > INT_MAX || i < INT_MIN)
    LOG->warning(_("Value is out of range."));
  purple_request_field_int_set_value(field, CLAMP(i, INT_MIN, INT_MAX));
  setValue(purple_request_field_int_get_value(field));
}

Request::FieldsDialog::BooleanField::BooleanField(PurpleRequestField *field)
  : field(field)
{
  g_assert(field);

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  setText(text);
  g_free(text);

  setChecked(purple_request_field_bool_get_value(field));
  signal_toggle.connect(sigc::mem_fun(this, &BooleanField::onToggle));
}

void Request::FieldsDialog::BooleanField::onToggle(
  CheckBox & /*activator*/, bool new_state)
{
  purple_request_field_bool_set_value(field, new_state);
}

Request::FieldsDialog::ChoiceField::ChoiceField(PurpleRequestField *field)
  : field(field)
{
  g_assert(field);

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  setText(text);
  g_free(text);

  for (GList *list = purple_request_field_choice_get_labels(field); list;
       list = list->next)
    addOption(static_cast<const char *>(list->data));
  setSelected(purple_request_field_choice_get_default_value(field));

  signal_selection_changed.connect(
    sigc::mem_fun(this, &ChoiceField::onSelectionChanged));
}

void Request::FieldsDialog::ChoiceField::onSelectionChanged(
  ComboBox & /*activator*/, int new_entry, const char * /*title*/,
  intptr_t /*data*/)
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
    appendWidget(
      *(new ListFieldItem(field, static_cast<const char *>(list->data))));
  setHeight(height);
}

Request::FieldsDialog::ListFieldMultiple::ListFieldItem::ListFieldItem(
  PurpleRequestField *field, const char *text)
  : field(field)
{
  g_assert(field);

  setText(text);
  setChecked(purple_request_field_list_is_selected(field, text));
  signal_toggle.connect(sigc::mem_fun(this, &ListFieldItem::onToggle));
}

void Request::FieldsDialog::ListFieldMultiple::ListFieldItem::onToggle(
  CheckBox & /*activator*/, bool new_state)
{
  if (new_state)
    purple_request_field_list_add_selected(field, getText());
  else {
    /* XXX This chunk is super-slow, libpurple should provide
     * purple_request_field_list_remove_selected() function. */
    GList *new_selected = NULL;
    for (GList *selected = purple_request_field_list_get_selected(field);
         selected; selected = selected->next) {
      const char *data = static_cast<const char *>(selected->data);
      if (strcmp(getText(), data))
        new_selected = g_list_append(new_selected, g_strdup(data));
    }

    if (new_selected) {
      purple_request_field_list_set_selected(field, new_selected);
      g_list_foreach(new_selected, reinterpret_cast<GFunc>(g_free), NULL);
      g_list_free(new_selected);
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

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  setText(text);
  g_free(text);

  GList *list = purple_request_field_list_get_items(field);
  for (int i = 0; list; i++, list = list->next) {
    const char *text = static_cast<const char *>(list->data);
    addOption(text);
    if (purple_request_field_list_is_selected(field, text))
      setSelected(i);
  }

  signal_selection_changed.connect(
    sigc::mem_fun(this, &ListFieldSingle::onSelectionChanged));
}

void Request::FieldsDialog::ListFieldSingle::onSelectionChanged(
  ComboBox & /*activator*/, int /*new_entry*/, const char *title,
  intptr_t /*data*/)
{
  purple_request_field_list_clear_selected(field);
  purple_request_field_list_add_selected(field, title);
}

Request::FieldsDialog::LabelField::LabelField(PurpleRequestField *field)
  : field(field)
{
  g_assert(field);

  setText(purple_request_field_get_label(field));
}

Request::FieldsDialog::ImageField::ImageField(PurpleRequestField *field)
  : field(field)
{
  g_assert(field);

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  setText(text);
  g_free(text);

  // TODO

  signal_activate.connect(sigc::mem_fun(this, &ImageField::onActivate));
}

void Request::FieldsDialog::ImageField::onActivate(Button & /*activator*/)
{
}

Request::FieldsDialog::AccountField::AccountField(PurpleRequestField *field)
  : field(field)
{
  g_assert(field);

  // TODO filter (purple_request_field_account_get_filter())
  // TODO signals (signed-on, signed-off, account-added, account-removed)

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field) ? "*" : "",
      purple_request_field_get_label(field));
  setText(text);
  g_free(text);

  gboolean show_all = purple_request_field_account_get_show_all(field);
  for (GList *list = purple_accounts_get_all(); list; list = list->next) {
    PurpleAccount *account = static_cast<PurpleAccount *>(list->data);
    if (!show_all && !purple_account_is_connected(account))
      continue;

    char *label =
      g_strdup_printf("[%s] %s", purple_account_get_protocol_name(account),
        purple_account_get_username(account));
    addOptionPtr(label, account);
    g_free(label);
  }
  setSelectedByDataPtr(purple_request_field_account_get_default_value(field));

  signal_selection_changed.connect(
    sigc::mem_fun(this, &AccountField::onAccountChanged));
}

void Request::FieldsDialog::AccountField::onAccountChanged(
  Button & /*activator*/, size_t /*new_entry*/, const char * /*title*/,
  intptr_t data)
{
  purple_request_field_account_set_value(
    field, reinterpret_cast<PurpleAccount *>(data));
}

void Request::FieldsDialog::responseHandler(
  SplitDialog & /*activator*/, ResponseType response)
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
  centerim_request_ui_ops.request_action_with_icon = request_action_with_icon_;
  purple_request_set_ui_ops(&centerim_request_ui_ops);
}

Request::~Request()
{
  // close all opened requests
  while (requests.size()) {
    RequestDialog *dialog = *(requests.begin());
    purple_request_close(dialog->getRequestType(), dialog);
  }

  purple_request_set_ui_ops(NULL);
}

void Request::init()
{
  g_assert(!my_instance);

  my_instance = new Request;
}

void Request::finalize()
{
  g_assert(my_instance);

  delete my_instance;
  my_instance = NULL;
}

void Request::onDialogResponse(CppConsUI::SplitDialog &dialog,
  CppConsUI::AbstractDialog::ResponseType /*response*/)
{
  RequestDialog *rdialog = dynamic_cast<RequestDialog *>(&dialog);
  g_assert(rdialog);

  if (requests.find(rdialog) != requests.end()) {
    requests.erase(rdialog);
    purple_request_close(rdialog->getRequestType(), rdialog);
  }
}

void *Request::request_input(const char *title, const char *primary,
  const char *secondary, const char *default_value, gboolean /*multiline*/,
  gboolean masked, char * /*hint*/, const char *ok_text, GCallback ok_cb,
  const char *cancel_text, GCallback cancel_cb, PurpleAccount * /*account*/,
  const char * /*who*/, PurpleConversation * /*conv*/, void *user_data)
{
  LOG->debug("request_input");

  InputTextDialog *dialog = new InputTextDialog(title, primary, secondary,
    default_value, masked, ok_text, ok_cb, cancel_text, cancel_cb, user_data);
  dialog->signal_response.connect(
    sigc::mem_fun(this, &Request::onDialogResponse));
  dialog->show();

  requests.insert(dialog);
  return dialog;
}

void *Request::request_choice(const char *title, const char *primary,
  const char *secondary, int default_value, const char *ok_text,
  GCallback ok_cb, const char *cancel_text, GCallback cancel_cb,
  PurpleAccount * /*account*/, const char * /*who*/,
  PurpleConversation * /*conv*/, void *user_data, va_list choices)
{
  LOG->debug("request_choice");

  ChoiceDialog *dialog = new ChoiceDialog(title, primary, secondary,
    default_value, ok_text, ok_cb, cancel_text, cancel_cb, user_data, choices);
  dialog->signal_response.connect(
    sigc::mem_fun(this, &Request::onDialogResponse));
  dialog->show();

  requests.insert(dialog);
  return dialog;
}

void *Request::request_action(const char *title, const char *primary,
  const char *secondary, int default_action, PurpleAccount * /*account*/,
  const char * /*who*/, PurpleConversation * /*conv*/, void *user_data,
  size_t action_count, va_list actions)
{
  LOG->debug("request_action");

  ActionDialog *dialog = new ActionDialog(title, primary, secondary,
    default_action, user_data, action_count, actions);
  dialog->signal_response.connect(
    sigc::mem_fun(this, &Request::onDialogResponse));
  dialog->show();

  requests.insert(dialog);
  return dialog;
}

void *Request::request_fields(const char *title, const char *primary,
  const char *secondary, PurpleRequestFields *fields, const char *ok_text,
  GCallback ok_cb, const char *cancel_text, GCallback cancel_cb,
  PurpleAccount * /*account*/, const char * /*who*/,
  PurpleConversation * /*conv*/, void *user_data)
{
  LOG->debug("request_fields");

  FieldsDialog *dialog = new FieldsDialog(title, primary, secondary, fields,
    ok_text, ok_cb, cancel_text, cancel_cb, user_data);
  dialog->signal_response.connect(
    sigc::mem_fun(this, &Request::onDialogResponse));
  dialog->show();

  requests.insert(dialog);
  return dialog;
}

void *Request::request_file(const char * /*title*/, const char * /*filename*/,
  gboolean /*savedialog*/, GCallback /*ok_cb*/, GCallback /*cancel_cb*/,
  PurpleAccount * /*account*/, const char * /*who*/,
  PurpleConversation * /*conv*/, void * /*user_data*/)
{
  return NULL;
}

void Request::close_request(PurpleRequestType /*type*/, void *ui_handle)
{
  LOG->debug("close_request");

  g_assert(ui_handle);

  RequestDialog *dialog = static_cast<RequestDialog *>(ui_handle);
  if (requests.find(dialog) != requests.end()) {
    requests.erase(dialog);
    dialog->close();
  }
}

void *Request::request_folder(const char * /*title*/, const char * /*dirname*/,
  GCallback /*ok_cb*/, GCallback /*cancel_cb*/, PurpleAccount * /*account*/,
  const char * /*who*/, PurpleConversation * /*conv*/, void * /*user_data*/)
{
  return NULL;
}

void *Request::request_action_with_icon(const char * /*title*/,
  const char * /*primary*/, const char * /*secondary*/, int /*default_action*/,
  PurpleAccount * /*account*/, const char * /*who*/,
  PurpleConversation * /*conv*/, gconstpointer /*icon_data*/,
  gsize /*icon_size*/, void * /*user_data*/, size_t /*action_count*/,
  va_list /*actions*/)
{
  return NULL;
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
