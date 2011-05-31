/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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

#include "Conversation.h"

#include "CenterIM.h"
#include <Footer.h>

#include <cppconsui/Keys.h>

#include <sys/stat.h>
#include "gettext.h"

#define LOGS_DIR "clogs"
#define TIME_FORMAT _("%d.%m.%y %H:%M")

/// @todo Increase to higher value later.
//#define CONVERSATION_DESTROY_TIMEOUT 1800000
#define CONVERSATION_DESTROY_TIMEOUT 6000

Conversation::Conversation(PurpleConversation *conv_)
: Window(0, 0, 80, 24), conv(conv_), filename(NULL), logfile(NULL)
, status(STATUS_ACTIVE)
{
  g_assert(conv);

  SetColorScheme("conversation");

  view = new CppConsUI::TextView(width - 2, height, true, true);
  input = new CppConsUI::TextEdit(width - 2, height);
  line = new CppConsUI::HorizontalLine(width);
  AddWidget(*view, 1, 0);
  AddWidget(*input, 1, 1);
  AddWidget(*line, 0, height);
  input->GrabFocus();

  // open logfile
  BuildLogFilename();

  GError *err = NULL;
  if (!(logfile = g_io_channel_new_file(filename, "a", &err))) {
    if (err) {
      LOG->Error(_("Error opening conversation logfile '%s' (%s)."),
          filename, err->message);

      g_error_free(err);
      err = NULL;
    }
    else
      LOG->Error(_("Error opening conversation logfile '%s'."), filename);
  }

  LoadHistory();

  DeclareBindables();
  LOG->Debug("Conversation::Conversation(): this=%p, title=%s",
      static_cast<void*>(this), purple_conversation_get_title(conv));
}

Conversation::~Conversation()
{
  g_free(filename);
  if (logfile)
    g_io_channel_unref(logfile);
  LOG->Debug("Conversation::~Conversation(): this=%p",
      static_cast<void*>(this));
}

bool Conversation::ProcessInput(const TermKeyKey& key)
{
  if (Window::ProcessInput(key))
    return true;
  return view->ProcessInput(key);
}

void Conversation::MoveResize(int newx, int newy, int neww, int newh)
{
  Window::MoveResize(newx, newy, neww, newh);

  int percentage = purple_prefs_get_int(CONF_PREFIX "/chat/partitioning");
  percentage = CLAMP(percentage, 0, 100);

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

bool Conversation::RestoreFocus()
{
  FOOTER->SetText(_(
        "<centerim|buddylist> buddy list, "
        "<centerim|conversation-next>/"
        "<centerim|conversation-prev>/<centerim|conversation-active> next/prev/act conv, "
        "<centerim|accountstatusmenu> status menu, "
        "<conversation|send> send"));

  return Window::RestoreFocus();
}

void Conversation::UngrabFocus()
{
  FOOTER->SetText(NULL);
  Window::UngrabFocus();
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
  CppConsUI::Rect r = CENTERIM->GetScreenAreaSize(CenterIM::CHAT_AREA);
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

void Conversation::Receive(const char *name, const char *alias,
    const char *message, PurpleMessageFlags flags, time_t mtime)
{
  PurpleConversationType type = purple_conversation_get_type(conv);

  int color = 0;
  char mtype = 'O'; // other
  if (flags & PURPLE_MESSAGE_SEND) {
    color = 1;
    mtype = 'S'; // sent
  }
  else if (flags & PURPLE_MESSAGE_RECV) {
    color = 2;
    mtype = 'R'; // recv
  }

  // write text into logfile
  long t = static_cast<long>(mtime);
  char *msg;
  if (type == PURPLE_CONV_TYPE_CHAT)
    msg = g_strdup_printf("%c %ld %s: %s\n", mtype, t, name, message);
  else
    msg = g_strdup_printf("%c %ld %s\n", mtype, t, message);
  if (logfile) {
    GError *err = NULL;
    if (g_io_channel_write_chars(logfile, msg, -1, NULL, &err)
        != G_IO_STATUS_NORMAL) {
      if (err) {
        LOG->Error(_("Error writing to conversation logfile (%s)."),
            err->message);
        g_error_free(err);
        err = NULL;
      }
      else
        LOG->Error(_("Error writing to conversation logfile."));
    }
    if (g_io_channel_flush(logfile, &err) != G_IO_STATUS_NORMAL) {
      if (err) {
        LOG->Error(_("Error flushing conversation logfile (%s)."),
            err->message);
        g_error_free(err);
        err = NULL;
      }
      else
        LOG->Error(_("Error flushing conversation logfile."));
    }
  }
  g_free(msg);

  // we currently don't support displaying HTML in any way
  char *nohtml = purple_markup_strip_html(message);

  // write text to the window
  const char *timestr = purple_utf8_strftime(TIME_FORMAT, localtime(&mtime));
  if (type == PURPLE_CONV_TYPE_CHAT)
    msg = g_strdup_printf("%s %s: %s", timestr, name, nohtml);
  else
    msg = g_strdup_printf("%s %s", timestr, nohtml);
  view->Append(msg, color);
  g_free(msg);

  g_free(nohtml);
}

void Conversation::ActionSend()
{
  PurpleConversationType type = purple_conversation_get_type(conv);
  char *str = input->AsString();
  if (str) {
    char *escaped = purple_markup_escape_text(str, strlen(str));
    char *html = purple_strdup_withhtml(escaped);
    if (type == PURPLE_CONV_TYPE_CHAT)
      purple_conv_chat_send(PURPLE_CONV_CHAT(conv), html);
    else if (type == PURPLE_CONV_TYPE_IM)
      purple_conv_im_send(PURPLE_CONV_IM(conv), html);
    g_free(html);
    g_free(escaped);
    g_free(str);
    input->Clear();
  }
}

void Conversation::DestroyPurpleConversation(PurpleConversation *conv)
{
  purple_conversation_destroy(conv);
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
    LOG->Error(_("Error creating directory '%s'."), dir);
  g_free(dir);

  g_free(acct_name);
}

void Conversation::LoadHistory()
{
  // open logfile
  GError *err = NULL;
  GIOChannel *chan;

  if ((chan = g_io_channel_new_file(filename, "r", &err)) == NULL) {
    if (err) {
      LOG->Error(_("Error opening conversation logfile '%s' (%s)."),
          filename, err->message);
      g_error_free(err);
      err = NULL;
    }
    else
      LOG->Error(_("Error opening conversation logfile '%s'."), filename);
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
    char *msg = g_strdup_printf("%s %s", purple_utf8_strftime(TIME_FORMAT,
          localtime(&time)), nohtml);
    view->Append(msg, color);
    g_free(nohtml);
    g_free(msg);

    g_free(line);
  }
  if (st != G_IO_STATUS_EOF) {
    if (err) {
      LOG->Error(_("Error reading from conversation logfile '%s' (%s)."),
          filename, err->message);
      g_error_free(err);
      err = NULL;
    }
    else
      LOG->Error(_("Error reading from conversation logfile '%s'."),
          filename);
  }
  g_io_channel_unref(chan);
}

void Conversation::DeclareBindables()
{
  DeclareBindable("conversation", "send",
      sigc::mem_fun(this, &Conversation::ActionSend),
      InputProcessor::BINDABLE_OVERRIDE);
}
