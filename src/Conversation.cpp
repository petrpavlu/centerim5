/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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

#include "Conversation.h"

#include "CenterIM.h"

#include <cppconsui/Keys.h>

#include <sys/stat.h>
#include "gettext.h"

#define CONTEXT_CONVERSATION "conversation"

#define LOGS_DIR "clogs"
#define TIME_FORMAT _("%d.%m.%y %H:%M")

/// @todo Increase to higher value later.
//#define CONVERSATION_DESTROY_TIMEOUT 1800000
#define CONVERSATION_DESTROY_TIMEOUT 6000

#define CONF_CHAT_PARTITIONING_DEFAULT 80 /* 20% for the input window */

Conversation::Conversation(PurpleConversation *conv_)
: Window(0, 0, 80, 24)
, conv(conv_)
, filename(NULL)
, logfile(NULL)
, status(STATUS_ACTIVE)
{
  g_assert(conv);

  SetColorScheme("conversation");

  view = new TextView(width - 2, height, true);
  input = new TextEdit(width - 2, height);
  line = new HorizontalLine(width);
  AddWidget(*view, 1, 0);
  AddWidget(*input, 1, 1);
  AddWidget(*line, 0, height);

  // open logfile
  BuildLogFilename();

  GError *err = NULL;
  if ((logfile = g_io_channel_new_file(filename, "a", &err)) == NULL) {
    if (err) {
      LOG->Error(_("Error opening conversation logfile `%s' (%s).\n"),
          filename, err->message);

      g_error_free(err);
      err = NULL;
    }
    else
      LOG->Error(_("Error opening conversation logfile `%s'.\n"), filename);
  }

  DeclareBindables();
}

Conversation::~Conversation()
{
  g_free(filename);
  if (logfile)
    g_io_channel_unref(logfile);
}

void Conversation::DestroyPurpleConversation(PurpleConversation *conv)
{
  purple_conversation_destroy(conv);
}

void Conversation::DeclareBindables()
{
  DeclareBindable(CONTEXT_CONVERSATION, "send",
      sigc::mem_fun(this, &Conversation::Send),
      InputProcessor::BINDABLE_OVERRIDE);
}

DEFINE_SIG_REGISTERKEYS(Conversation, RegisterKeys);
bool Conversation::RegisterKeys()
{
  RegisterKeyDef(CONTEXT_CONVERSATION, "send", _("Send the message."),
      Keys::UnicodeTermKey("x", TERMKEY_KEYMOD_CTRL));
  // XXX move to default key bindings config
  RegisterKeyDef(CONTEXT_CONVERSATION, "send", _("Send the message."),
      Keys::SymbolTermKey(TERMKEY_SYM_ENTER));
  return true;
}

void Conversation::BuildLogFilename()
{
  PurpleAccount *account;
  PurplePlugin *prpl;
  const char *proto_name;
  char *acct_name;
  char *dir;
  const char *name;

  account = purple_conversation_get_account(conv);
  prpl = purple_find_prpl(purple_account_get_protocol_id(account));
  g_assert(prpl);

  proto_name = purple_account_get_protocol_name(account);

  acct_name = g_strdup(purple_escape_filename(purple_normalize(account,
          purple_account_get_username(account))));

  name = purple_conversation_get_name(conv);

  filename = g_build_filename(purple_user_dir(), LOGS_DIR, proto_name,
      acct_name, purple_escape_filename(
        purple_normalize(account, name)), NULL);

  dir = g_path_get_dirname(filename);
  if (g_mkdir_with_parents(dir, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
    LOG->Error(_("Error creating directory `%s'.\n"), dir);
  g_free(dir);

  g_free(acct_name);
}

void Conversation::Close()
{
  /* Let libpurple and Conversations know that this conversation should be
   * destroyed after some time. */
  destroy_conn = COREMANAGER->TimeoutOnceConnect(sigc::bind(sigc::mem_fun(
          this, &Conversation::DestroyPurpleConversation), conv),
      CONVERSATION_DESTROY_TIMEOUT);
  status = STATUS_TRASH;

  signal_close(*this);
}

void Conversation::ScreenResized()
{
  Rect r = CENTERIM->GetScreenAreaSize(CenterIM::CHAT_AREA);
  // make room for conversations list
  r.height--;

  MoveResizeRect(r);
}

void Conversation::Show()
{
  if (destroy_conn.connected()) {
    destroy_conn.disconnect();
    status = STATUS_ACTIVE;
  }
  Window::Show();
}

void Conversation::MoveResize(int newx, int newy, int neww, int newh)
{
  Window::MoveResize(newx, newy, neww, newh);

  char *pref = g_strconcat(CONF_PREFIX, "chat/partitioning", NULL);
  int percentage = CONF->GetInt(pref, CONF_CHAT_PARTITIONING_DEFAULT, 0, 100);
  g_free(pref);

  int view_height = (height * percentage) / 100;
  if (view_height < 1)
    view_height = 1;

  int input_height = height - view_height - 1;
  if (input_height < 1)
    input_height = 1;

  view->MoveResize(1, 0, width - 2, view_height);
  input->MoveResize(1, view_height + 1, width - 2, input_height);
  line->MoveResize(0, view_height, width, 1);
}

void Conversation::Receive(const char *name, const char *alias, const char *message,
  PurpleMessageFlags flags, time_t mtime)
{
  // we currently don't support displaying HTML in any way
  char *nohtml = purple_markup_strip_html(message);

  int color = 0;
  char type = 'O'; // other
  if (flags & PURPLE_MESSAGE_SEND) {
    color = 1;
    type = 'S'; // sent
  }
  else if (flags & PURPLE_MESSAGE_RECV) {
    color = 2;
    type = 'R'; // recv
  }

  // write text into logfile
  // encode all newline characters as <br>
  char *html = purple_strdup_withhtml(nohtml);
  char *msg = g_strdup_printf("%c %d %s\n", type, static_cast<int>(mtime),
      html);
  g_free(html);
  if (logfile) {
    GError *err = NULL;
    if (g_io_channel_write_chars(logfile, msg, -1, NULL, &err)
        != G_IO_STATUS_NORMAL) {
      if (err) {
        LOG->Error(_("Error writing to conversation logfile (%s).\n"),
            err->message);
        g_error_free(err);
        err = NULL;
      }
      else
        LOG->Error(_("Error writing to conversation logfile.\n"));
    }
    if (g_io_channel_flush(logfile, &err) != G_IO_STATUS_NORMAL) {
      if (err) {
        LOG->Error(_("Error flushing conversation logfile (%s).\n"),
            err->message);
        g_error_free(err);
        err = NULL;
      }
      else
        LOG->Error(_("Error flushing conversation logfile.\n"));
    }
  }
  g_free(msg);

  // write text to the window
  msg = g_strdup_printf("%s %s", purple_utf8_strftime(TIME_FORMAT, localtime(&mtime)), nohtml);
  view->Append(msg, color);
  g_free(msg);

  g_free(nohtml);
}

ConversationChat::ConversationChat(PurpleConversation *conv)
: Conversation(conv)
{
  convchat = PURPLE_CONV_CHAT(conv);
  LoadHistory();
}

void ConversationChat::LoadHistory()
{
  //g_return_if_fail(CONF->GetLogChats());

  PurpleAccount *account = purple_conversation_get_account(conv);
  const char *name = purple_conversation_get_name(conv);
  GList *logs = NULL;
  const char *alias = name;
  PurpleLogReadFlags flags;
  char *history;
  char *header;
  PurpleMessageFlags mflag;

  logs = purple_log_get_logs(PURPLE_LOG_CHAT, name, account);

  if (logs == NULL)
    return;

  mflag = (PurpleMessageFlags)(PURPLE_MESSAGE_NO_LOG | PURPLE_MESSAGE_SYSTEM | PURPLE_MESSAGE_DELAYED);
  history = purple_log_read((PurpleLog*)logs->data, &flags);

  header = g_strdup_printf("<b>Conversation with %s on %s:</b><br>", alias,
               purple_date_format_full(localtime(&((PurpleLog *)logs->data)->time)));
  view->Append(header);
  g_free(header);

  if (flags & PURPLE_LOG_READ_NO_NEWLINE)
    purple_str_strip_char(history, '\n');
  char *nohtml = purple_markup_strip_html(history);
  g_free(history);
  view->Append(nohtml);
  g_free(nohtml);

  g_list_foreach(logs, (GFunc)purple_log_free, NULL);
  g_list_free(logs);
}

void ConversationChat::Send()
{
}

ConversationIm::ConversationIm(PurpleConversation *conv)
: Conversation(conv)
{
  convim = PURPLE_CONV_IM(conv);
  LoadHistory();
  LOG->Debug("%p constructor()\n", this);
}

ConversationIm::~ConversationIm()
{
  LOG->Debug("%p destructor()\n", this);
}

void ConversationIm::LoadHistory()
{
  // open logfile
  GError *err = NULL;
  GIOChannel *chan;

  if ((chan = g_io_channel_new_file(filename, "r", &err)) == NULL) {
    if (err) {
      LOG->Error(_("Error opening conversation logfile `%s' (%s).\n"),
          filename, err->message);
      g_error_free(err);
      err = NULL;
    }
    else
      LOG->Error(_("Error opening conversation logfile `%s'.\n"), filename);
    return;
  }

  GIOStatus st;
  char *line;
  // read conversation logfile line by line
  while ((st = g_io_channel_read_line(chan, &line, NULL, NULL, &err))
      == G_IO_STATUS_NORMAL && line != NULL) {
    // parse type
    const char *cur = line;
    int color = 0;
    switch (*cur) {
      case 'O': // other
        break;
      case 'S': // sent
        color = 1;
        break;
      case 'R': // recv
        color = 2;
        break;
      default: // wrong format
        continue;
    }

    // skip to time mark
    cur++;
    if (*cur != ' ')
      continue;
    cur++;

    // parse time
    time_t time = 0;
    while (*cur >= '0' && *cur <= '9') {
      time = 10 * time + *cur - '0';
      cur++;
    }

    // sanity check
    if (*cur != ' ')
      continue;
    cur++;

    // write text to the window
    char *nohtml = purple_markup_strip_html(cur);
    char *msg = g_strdup_printf("%s %s", purple_utf8_strftime(TIME_FORMAT, localtime(&time)), nohtml);
    view->Append(msg, color);
    g_free(nohtml);
    g_free(msg);

    g_free(line);
  }
  if (st != G_IO_STATUS_EOF) {
    if (err) {
      LOG->Error(_("Error reading from conversation logfile `%s' (%s).\n"),
          filename, err->message);
      g_error_free(err);
      err = NULL;
    }
    else
      LOG->Error(_("Error reading from conversation logfile `%s'.\n"),
          filename);
  }
  g_io_channel_unref(chan);
}

void ConversationIm::Send()
{
  char *str = input->AsString("<br/>");
  if (str) {
    purple_conv_im_send(convim, str);
    g_free(str);
    input->Clear();
  }
}
