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

#include "Request.h"

#include "Log.h"

#include <cppconsui/InputDialog.h>
#include <cppconsui/Spacer.h>
#include <cstring>
#include <errno.h>
#include "gettext.h"

Request *Request::my_instance_ = NULL;

Request *Request::instance()
{
  return my_instance_;
}

Request::RequestDialog::RequestDialog(const char *title, const char *primary,
  const char *secondary, const char *ok_text, GCallback ok_cb,
  const char *cancel_text, GCallback cancel_cb, void *user_data)
  : SplitDialog(title), ok_cb_(ok_cb), cancel_cb_(cancel_cb),
    user_data_(user_data)
{
  setColorScheme(CenterIM::SCHEME_GENERALWINDOW);

  lbox_ = new CppConsUI::ListBox(AUTOSIZE, AUTOSIZE);
  if (primary)
    lbox_->appendWidget(*(new CppConsUI::Label(AUTOSIZE, 1, primary)));
  if (primary && secondary)
    lbox_->appendWidget(*(new CppConsUI::Spacer(AUTOSIZE, 1)));
  if (secondary)
    lbox_->appendWidget(*(new CppConsUI::Label(AUTOSIZE, 1, secondary)));
  if (primary || secondary)
    lbox_->appendWidget(*(new CppConsUI::HorizontalLine(AUTOSIZE)));
  setContainer(*lbox_);

  if (ok_text != NULL)
    addButton(ok_text, RESPONSE_OK);
  if (ok_text != NULL && cancel_text != NULL)
    addSeparator();
  if (cancel_text != NULL)
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
  entry_ = new CppConsUI::TextEntry(AUTOSIZE, AUTOSIZE, default_value);
  lbox_->appendWidget(*entry_);
  entry_->grabFocus();
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
    if (ok_cb_ != NULL)
      reinterpret_cast<PurpleRequestInputCb>(ok_cb_)(
        user_data_, entry_->getText());
    break;
  case AbstractDialog::RESPONSE_CANCEL:
    if (cancel_cb_ != NULL)
      reinterpret_cast<PurpleRequestInputCb>(cancel_cb_)(
        user_data_, entry_->getText());
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
  combo_ = new CppConsUI::ComboBox(AUTOSIZE, 1, _("Selected value"));
  lbox_->appendWidget(*combo_);
  combo_->grabFocus();

  const char *text;
  while ((text = va_arg(choices, const char *)) != NULL) {
    int resp = va_arg(choices, int);
    combo_->addOption(text, resp);
  }
  combo_->setSelectedByData(default_value);
}

PurpleRequestType Request::ChoiceDialog::getRequestType()
{
  return PURPLE_REQUEST_CHOICE;
}

void Request::ChoiceDialog::responseHandler(
  SplitDialog & /*activator*/, ResponseType response)
{
  switch (response) {
  case AbstractDialog::RESPONSE_OK:
    if (ok_cb_ != NULL)
      reinterpret_cast<PurpleRequestChoiceCb>(ok_cb_)(
        user_data_, combo_->getSelectedData());
    break;
  case AbstractDialog::RESPONSE_CANCEL:
    if (cancel_cb_ != NULL)
      reinterpret_cast<PurpleRequestChoiceCb>(cancel_cb_)(
        user_data_, combo_->getSelectedData());
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
  for (size_t i = 0; i < action_count; ++i) {
    const char *title = va_arg(actions, const char *);
    GCallback cb = va_arg(actions, GCallback);

    CppConsUI::Button *b = buttons_->appendItem(title,
      sigc::bind(sigc::mem_fun(this, &ActionDialog::onActionChoice), i, cb));
    if (static_cast<int>(i) == default_value)
      b->grabFocus();

    if (i < action_count - 1)
      buttons_->appendSeparator();
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
  if (cb != NULL)
    reinterpret_cast<PurpleRequestActionCb>(cb)(user_data_, i);

  // It is possible that the callback action already called
  // purple_request_destroy() in which case 'this' object is already deleted and
  // calling close() in such a case leads to an error.
  Requests *requests = &REQUEST->requests_;
  if (requests->find(this) != requests->end())
    close();
}

Request::FieldsDialog::FieldsDialog(const char *title, const char *primary,
  const char *secondary, PurpleRequestFields *request_fields,
  const char *ok_text, GCallback ok_cb, const char *cancel_text,
  GCallback cancel_cb, void *user_data)
  : RequestDialog(title, primary, secondary, ok_text, ok_cb, cancel_text,
      cancel_cb, user_data),
    fields_(request_fields)
{
  treeview_ = new CppConsUI::TreeView(AUTOSIZE, AUTOSIZE);
  lbox_->appendWidget(*treeview_);

  bool grouping = true;
  GList *groups = purple_request_fields_get_groups(fields_);
  if (groups == NULL)
    return;
  if (purple_request_field_group_get_title(
        static_cast<PurpleRequestFieldGroup *>(groups->data)) == NULL &&
    groups->next == NULL)
    grouping = false;
  for (; groups != NULL; groups = groups->next) {
    PurpleRequestFieldGroup *group =
      static_cast<PurpleRequestFieldGroup *>(groups->data);

    CppConsUI::TreeView::NodeReference parent = treeview_->getRootNode();
    if (grouping) {
      const char *title = purple_request_field_group_get_title(group);
      if (title == NULL)
        title = _("Settings group");

      CppConsUI::TreeView::ToggleCollapseButton *button =
        new CppConsUI::TreeView::ToggleCollapseButton(title);
      parent = treeview_->appendNode(treeview_->getRootNode(), *button);
    }

    for (GList *gfields = purple_request_field_group_get_fields(group);
         gfields != NULL; gfields = gfields->next) {
      PurpleRequestField *field =
        static_cast<PurpleRequestField *>(gfields->data);

      if (!purple_request_field_is_visible(field))
        continue;

      PurpleRequestFieldType type = purple_request_field_get_type(field);

      switch (type) {
      case PURPLE_REQUEST_FIELD_STRING:
        treeview_->appendNode(parent, *(new StringField(field)));
        break;
      case PURPLE_REQUEST_FIELD_INTEGER:
        treeview_->appendNode(parent, *(new IntegerField(field)));
        break;
      case PURPLE_REQUEST_FIELD_BOOLEAN:
        treeview_->appendNode(parent, *(new BooleanField(field)));
        break;
      case PURPLE_REQUEST_FIELD_CHOICE:
        treeview_->appendNode(parent, *(new ChoiceField(field)));
        break;
      case PURPLE_REQUEST_FIELD_LIST:
        if (purple_request_field_list_get_multi_select(field))
          treeview_->appendNode(parent, *(new ListFieldMultiple(field)));
        else
          treeview_->appendNode(parent, *(new ListFieldSingle(field)));
        break;
      case PURPLE_REQUEST_FIELD_LABEL:
        treeview_->appendNode(parent, *(new LabelField(field)));
        break;
      case PURPLE_REQUEST_FIELD_IMAGE:
        treeview_->appendNode(parent, *(new ImageField(field)));
        break;
      case PURPLE_REQUEST_FIELD_ACCOUNT:
        treeview_->appendNode(parent, *(new AccountField(field)));
        break;
      default:
        LOG->error(_("Unhandled request field type '%d'."), type);
        break;
      }
    }
  }

  treeview_->grabFocus();
}

PurpleRequestType Request::FieldsDialog::getRequestType()
{
  return PURPLE_REQUEST_FIELDS;
}

Request::FieldsDialog::StringField::StringField(PurpleRequestField *field)
  : Button(FLAG_VALUE), field_(field)
{
  g_assert(field_ != NULL);

  if (purple_request_field_string_is_masked(field_))
    setMasked(true);

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field_) ? "*" : "",
      purple_request_field_get_label(field_));
  setText(text);
  g_free(text);

  setValue(purple_request_field_string_get_value(field_));
  signal_activate.connect(sigc::mem_fun(this, &StringField::onActivate));
}

void Request::FieldsDialog::StringField::onActivate(
  CppConsUI::Button & /*activator*/)
{
  CppConsUI::InputDialog *dialog =
    new CppConsUI::InputDialog(purple_request_field_get_label(field_),
      purple_request_field_string_get_value(field_));
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

  purple_request_field_string_set_value(field_, activator.getText());
  setValue(purple_request_field_string_get_value(field_));
}

Request::FieldsDialog::IntegerField::IntegerField(PurpleRequestField *field)
  : Button(FLAG_VALUE), field_(field)
{
  g_assert(field_ != NULL);

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field_) ? "*" : "",
      purple_request_field_get_label(field_));
  setText(text);
  g_free(text);

  setValue(purple_request_field_int_get_value(field_));
  signal_activate.connect(sigc::mem_fun(this, &IntegerField::onActivate));
}

void Request::FieldsDialog::IntegerField::onActivate(
  CppConsUI::Button & /*activator*/)
{
  char *value =
    g_strdup_printf("%d", purple_request_field_int_get_value(field_));
  CppConsUI::InputDialog *dialog =
    new CppConsUI::InputDialog(purple_request_field_get_label(field_), value);
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
  purple_request_field_int_set_value(field_, CLAMP(i, INT_MIN, INT_MAX));
  setValue(purple_request_field_int_get_value(field_));
}

Request::FieldsDialog::BooleanField::BooleanField(PurpleRequestField *field)
  : field_(field)
{
  g_assert(field_ != NULL);

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field_) ? "*" : "",
      purple_request_field_get_label(field_));
  setText(text);
  g_free(text);

  setChecked(purple_request_field_bool_get_value(field_));
  signal_toggle.connect(sigc::mem_fun(this, &BooleanField::onToggle));
}

void Request::FieldsDialog::BooleanField::onToggle(
  CheckBox & /*activator*/, bool new_state)
{
  purple_request_field_bool_set_value(field_, new_state);
}

Request::FieldsDialog::ChoiceField::ChoiceField(PurpleRequestField *field)
  : field_(field)
{
  g_assert(field_ != NULL);

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field_) ? "*" : "",
      purple_request_field_get_label(field_));
  setText(text);
  g_free(text);

  for (GList *list = purple_request_field_choice_get_labels(field_);
       list != NULL; list = list->next)
    addOption(static_cast<const char *>(list->data));
  setSelected(purple_request_field_choice_get_default_value(field_));

  signal_selection_changed.connect(
    sigc::mem_fun(this, &ChoiceField::onSelectionChanged));
}

void Request::FieldsDialog::ChoiceField::onSelectionChanged(
  ComboBox & /*activator*/, int new_entry, const char * /*title*/,
  intptr_t /*data*/)
{
  purple_request_field_choice_set_value(field_, new_entry);
}

Request::FieldsDialog::ListFieldMultiple::ListFieldMultiple(
  PurpleRequestField *field)
  : ListBox(AUTOSIZE, 1), field_(field)
{
  g_assert(field_ != NULL);

  // TODO Display label of the field somewhere.

  int height = 0;
  for (GList *list = purple_request_field_list_get_items(field_); list != NULL;
       list = list->next, height++)
    appendWidget(
      *(new ListFieldItem(field_, static_cast<const char *>(list->data))));
  setHeight(height);
}

Request::FieldsDialog::ListFieldMultiple::ListFieldItem::ListFieldItem(
  PurpleRequestField *field, const char *text)
  : field_(field)
{
  g_assert(field_ != NULL);

  setText(text);
  setChecked(purple_request_field_list_is_selected(field_, text));
  signal_toggle.connect(sigc::mem_fun(this, &ListFieldItem::onToggle));
}

void Request::FieldsDialog::ListFieldMultiple::ListFieldItem::onToggle(
  CheckBox & /*activator*/, bool new_state)
{
  if (new_state)
    purple_request_field_list_add_selected(field_, getText());
  else {
    // XXX This chunk is very slow, libpurple should provide
    // purple_request_field_list_remove_selected() function.
    GList *new_selected = NULL;
    for (GList *selected = purple_request_field_list_get_selected(field_);
         selected != NULL; selected = selected->next) {
      const char *data = static_cast<const char *>(selected->data);
      if (std::strcmp(getText(), data) != 0)
        new_selected = g_list_append(new_selected, g_strdup(data));
    }

    if (new_selected != NULL) {
      purple_request_field_list_set_selected(field_, new_selected);
      g_list_foreach(new_selected, reinterpret_cast<GFunc>(g_free), NULL);
      g_list_free(new_selected);
    }
    else
      purple_request_field_list_clear_selected(field_);
  }
}

Request::FieldsDialog::ListFieldSingle::ListFieldSingle(
  PurpleRequestField *field)
  : field_(field)
{
  g_assert(field_ != NULL);

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field_) ? "*" : "",
      purple_request_field_get_label(field_));
  setText(text);
  g_free(text);

  GList *list = purple_request_field_list_get_items(field_);
  for (int i = 0; list; ++i, list = list->next) {
    const char *text = static_cast<const char *>(list->data);
    addOption(text);
    if (purple_request_field_list_is_selected(field_, text))
      setSelected(i);
  }

  signal_selection_changed.connect(
    sigc::mem_fun(this, &ListFieldSingle::onSelectionChanged));
}

void Request::FieldsDialog::ListFieldSingle::onSelectionChanged(
  ComboBox & /*activator*/, int /*new_entry*/, const char *title,
  intptr_t /*data*/)
{
  purple_request_field_list_clear_selected(field_);
  purple_request_field_list_add_selected(field_, title);
}

Request::FieldsDialog::LabelField::LabelField(PurpleRequestField *field)
  : field_(field)
{
  g_assert(field_ != NULL);

  setText(purple_request_field_get_label(field_));
}

Request::FieldsDialog::ImageField::ImageField(PurpleRequestField *field)
  : field_(field)
{
  g_assert(field_ != NULL);

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field_) ? "*" : "",
      purple_request_field_get_label(field_));
  setText(text);
  g_free(text);

  // TODO

  signal_activate.connect(sigc::mem_fun(this, &ImageField::onActivate));
}

void Request::FieldsDialog::ImageField::onActivate(Button & /*activator*/)
{
}

Request::FieldsDialog::AccountField::AccountField(PurpleRequestField *field)
  : field_(field)
{
  g_assert(field_ != NULL);

  // TODO filter (purple_request_field_account_get_filter())
  // TODO signals (signed-on, signed-off, account-added, account-removed)

  char *text =
    g_strdup_printf("%s%s", purple_request_field_is_required(field_) ? "*" : "",
      purple_request_field_get_label(field_));
  setText(text);
  g_free(text);

  gboolean show_all = purple_request_field_account_get_show_all(field_);
  for (GList *list = purple_accounts_get_all(); list != NULL;
       list = list->next) {
    PurpleAccount *account = static_cast<PurpleAccount *>(list->data);
    if (!show_all && !purple_account_is_connected(account))
      continue;

    char *label =
      g_strdup_printf("[%s] %s", purple_account_get_protocol_name(account),
        purple_account_get_username(account));
    addOptionPtr(label, account);
    g_free(label);
  }
  setSelectedByDataPtr(purple_request_field_account_get_default_value(field_));

  signal_selection_changed.connect(
    sigc::mem_fun(this, &AccountField::onAccountChanged));
}

void Request::FieldsDialog::AccountField::onAccountChanged(
  Button & /*activator*/, size_t /*new_entry*/, const char * /*title*/,
  intptr_t data)
{
  purple_request_field_account_set_value(
    field_, reinterpret_cast<PurpleAccount *>(data));
}

void Request::FieldsDialog::responseHandler(
  SplitDialog & /*activator*/, ResponseType response)
{
  switch (response) {
  case AbstractDialog::RESPONSE_OK:
    if (ok_cb_ != NULL)
      reinterpret_cast<PurpleRequestFieldsCb>(ok_cb_)(user_data_, fields_);
    break;
  case AbstractDialog::RESPONSE_CANCEL:
    if (cancel_cb_ != NULL)
      reinterpret_cast<PurpleRequestFieldsCb>(cancel_cb_)(user_data_, fields_);
    break;
  default:
    g_assert_not_reached();
    break;
  }

  purple_request_fields_destroy(fields_);
}

Request::Request()
{
  memset(&centerim_request_ui_ops_, 0, sizeof(centerim_request_ui_ops_));

  // Set the purple request callbacks.
  centerim_request_ui_ops_.request_input = request_input_;
  centerim_request_ui_ops_.request_choice = request_choice_;
  centerim_request_ui_ops_.request_action = request_action_;
  centerim_request_ui_ops_.request_fields = request_fields_;
  centerim_request_ui_ops_.request_file = request_file_;
  centerim_request_ui_ops_.close_request = close_request_;
  centerim_request_ui_ops_.request_folder = request_folder_;
  centerim_request_ui_ops_.request_action_with_icon = request_action_with_icon_;
  purple_request_set_ui_ops(&centerim_request_ui_ops_);
}

Request::~Request()
{
  // Close all opened requests.
  while (!requests_.empty()) {
    RequestDialog *dialog = *(requests_.begin());
    purple_request_close(dialog->getRequestType(), dialog);
  }

  purple_request_set_ui_ops(NULL);
}

void Request::init()
{
  g_assert(my_instance_ == NULL);

  my_instance_ = new Request;
}

void Request::finalize()
{
  g_assert(my_instance_ != NULL);

  delete my_instance_;
  my_instance_ = NULL;
}

void Request::onDialogResponse(CppConsUI::SplitDialog &dialog,
  CppConsUI::AbstractDialog::ResponseType /*response*/)
{
  RequestDialog *rdialog = dynamic_cast<RequestDialog *>(&dialog);
  g_assert(rdialog != NULL);

  if (requests_.find(rdialog) == requests_.end())
    return;

  requests_.erase(rdialog);
  purple_request_close(rdialog->getRequestType(), rdialog);
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

  requests_.insert(dialog);
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

  requests_.insert(dialog);
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

  requests_.insert(dialog);
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

  requests_.insert(dialog);
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

  g_assert(ui_handle != NULL);

  RequestDialog *dialog = static_cast<RequestDialog *>(ui_handle);
  if (requests_.find(dialog) == requests_.end())
    return;

  requests_.erase(dialog);
  dialog->close();
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

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
