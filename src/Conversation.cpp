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

#include "Conversation.h"

#include "BuddyList.h"
#include "Conversations.h"
#include "Footer.h"

#include <cppconsui/ColorScheme.h>
#include <cstring>
#include <sys/stat.h>
#include "gettext.h"

Conversation::Conversation(PurpleConversation *conv)
  : Window(0, 0, 80, 24), conv_(conv), filename_(NULL), logfile_(NULL),
    input_text_length_(0), room_list_(NULL), room_list_line_(NULL)
{
  g_assert(conv_ != NULL);

  setColorScheme(CenterIM::SCHEME_CONVERSATION);

  view_ = new CppConsUI::TextView(width_ - 2, height_, true, true);
  input_ = new CppConsUI::TextEdit(width_ - 2, height_);
  input_->signal_text_change.connect(
    sigc::mem_fun(this, &Conversation::onInputTextChange));
  char *name = g_strdup_printf("[%s] %s",
    purple_account_get_protocol_name(purple_conversation_get_account(conv_)),
    purple_conversation_get_name(conv_));
  line_ = new ConversationLine(name);
  g_free(name);
  addWidget(*view_, 1, 0);
  addWidget(*input_, 1, 1);
  addWidget(*line_, 0, height_);

  PurpleConversationType type = purple_conversation_get_type(conv_);
  if (type == PURPLE_CONV_TYPE_CHAT) {
    room_list_ = new ConversationRoomList(1, 1, conv_);
    room_list_line_ = new CppConsUI::VerticalLine(1);

    addWidget(*room_list_, 1, 0);
    addWidget(*room_list_line_, 1, 0);
  }

  input_->grabFocus();

  // Open logfile.
  buildLogFilename();

  GError *err = NULL;
  logfile_ = g_io_channel_new_file(filename_, "a", &err);
  if (logfile_ == NULL) {
    LOG->error(_("Error opening conversation logfile '%s' (%s)."), filename_,
      err->message);
    g_clear_error(&err);
  }

  loadHistory();

  onScreenResized();
  declareBindables();
}

Conversation::~Conversation()
{
  g_free(filename_);
  if (logfile_ != NULL)
    g_io_channel_unref(logfile_);
}

bool Conversation::processInput(const TermKeyKey &key)
{
  if (view_->processInput(key))
    return true;

  return Window::processInput(key);
}

void Conversation::moveResize(int newx, int newy, int neww, int newh)
{
  Window::moveResize(newx, newy, neww, newh);

  int view_percentage = purple_prefs_get_int(CONF_PREFIX "/chat/partitioning");
  view_percentage = CLAMP(view_percentage, 0, 100);

  int view_height = (newh * view_percentage) / 100;
  if (view_height < 1)
    view_height = 1;

  int input_height = newh - view_height - 1;
  if (input_height < 1)
    input_height = 1;

  int roomlist_percentage =
    purple_prefs_get_int(CONF_PREFIX "/chat/roomlist_partitioning");
  roomlist_percentage = CLAMP(roomlist_percentage, 0, 100);

  int view_width = neww - 2;
  if (room_list_ != NULL)
    view_width = (view_width * roomlist_percentage) / 100;

  view_->moveResize(1, 0, view_width, view_height);

  input_->moveResize(1, view_height + 1, neww - 2, input_height);
  line_->moveResize(0, view_height, neww, 1);

  // Place the room list if it exists.
  if (room_list_ != NULL) {
    // +2 accounts for borders.
    room_list_line_->moveResize(view_width + 1, 0, 1, view_height);
    // Give it some padding to make it line up.
    room_list_->moveResize(
      view_width + 3, 0, neww - view_width - 3, view_height);
  }
}

bool Conversation::restoreFocus()
{
  FOOTER->setText(_("%s buddy list, %s main menu, "
                    "%s/%s/%s next/prev/act conv, %s send, %s expand"),
    "centerim|buddylist", "centerim|generalmenu", "centerim|conversation-next",
    "centerim|conversation-prev", "centerim|conversation-active",
    "conversation|send", "centerim|conversation-expand");

  return Window::restoreFocus();
}

void Conversation::ungrabFocus()
{
  FOOTER->setText(NULL);
  Window::ungrabFocus();
}

void Conversation::show()
{
  // Update the scrollbar setting. It is delayed until the conversation window
  // is actually displayed, so screen lines recalculations in TextView (caused
  // by changing the scrollbar setting) are not triggered if it is not really
  // necessary.
  view_->setScrollBar(!CENTERIM->isEnabledExpandedConversationMode());

  Window::show();
}

void Conversation::close()
{
  signal_close(*this);

  // Next line deletes this object. Do not touch any member variable after this
  // line.
  purple_conversation_destroy(conv_);
}

void Conversation::onScreenResized()
{
  CppConsUI::Rect r = CENTERIM->getScreenArea(CenterIM::CHAT_AREA);
  // Make room for conversation list.
  r.height--;

  moveResizeRect(r);
}

void Conversation::write(const char *name, const char * /*alias*/,
  const char *message, PurpleMessageFlags flags, time_t mtime)
{
  // Beep on message.
  if (!(flags & PURPLE_MESSAGE_SEND) &&
    purple_prefs_get_bool(CONF_PREFIX "/chat/beep_on_msg")) {
    // TODO Implement correct error handling.
    CppConsUI::Error error;
    CppConsUI::Curses::beep(error);
  }

  // Update the last_activity property.
  PurpleConversationType type = purple_conversation_get_type(conv_);
  time_t cur_time = time(NULL);

  if (type == PURPLE_CONV_TYPE_IM) {
    PurpleBlistNode *bnode = PURPLE_BLIST_NODE(
      purple_find_buddy(purple_conversation_get_account(conv_),
        purple_conversation_get_name(conv_)));
    if (bnode) {
      purple_blist_node_set_int(bnode, "last_activity", cur_time);

      // Inform the buddy list node that it should update its state.
      BUDDYLIST->updateNode(bnode);
    }
  }

  // Write the message.
  int color;
  const char *dir;
  const char *mtype;
  if (flags & PURPLE_MESSAGE_SEND) {
    dir = "OUT";
    mtype = "MSG2"; // cim5 message.
    color = 1;
  }
  else if (flags & PURPLE_MESSAGE_RECV) {
    dir = "IN";
    mtype = "MSG2"; // cim5 message.
    color = 2;
  }
  else {
    dir = "IN";
    mtype = "OTHER";
    color = 0;
  }

  // Write text into logfile.
  if (!(flags & PURPLE_MESSAGE_NO_LOG)) {
    char *log_msg;
    if (type == PURPLE_CONV_TYPE_CHAT)
      log_msg = g_strdup_printf("\f\n%s\n%s\n%lu\n%lu\n%s: %s\n", dir, mtype,
        mtime, cur_time, name, message);
    else
      log_msg = g_strdup_printf(
        "\f\n%s\n%s\n%lu\n%lu\n%s\n", dir, mtype, mtime, cur_time, message);
    if (logfile_ != NULL) {
      GError *err = NULL;
      if (g_io_channel_write_chars(logfile_, log_msg, -1, NULL, &err) !=
        G_IO_STATUS_NORMAL) {
        LOG->error(
          _("Error writing to conversation logfile (%s)."), err->message);
        g_clear_error(&err);
      }
      if (g_io_channel_flush(logfile_, &err) != G_IO_STATUS_NORMAL) {
        LOG->error(
          _("Error flushing conversation logfile (%s)."), err->message);
        g_clear_error(&err);
      }
    }
    g_free(log_msg);
  }

  // We currently do not support displaying HTML in any way.
  char *nohtml = stripHTML(message);

  // Write text to the window.
  char *time = extractTime(mtime, cur_time);
  char *msg;
  if (type == PURPLE_CONV_TYPE_CHAT)
    msg = g_strdup_printf("%s %s: %s", time, name, nohtml);
  else
    msg = g_strdup_printf("%s %s", time, nohtml);
  view_->append(msg, color);
  g_free(nohtml);
  g_free(time);
  g_free(msg);
}

Conversation::ConversationLine::ConversationLine(const char *text)
  : AbstractLine(AUTOSIZE, 1)
{
  g_assert(text != NULL);

  text_ = g_strdup(text);
  text_width_ = CppConsUI::Curses::onScreenWidth(text_);
}

Conversation::ConversationLine::~ConversationLine()
{
  g_free(text_);
}

int Conversation::ConversationLine::draw(
  CppConsUI::Curses::ViewPort area, CppConsUI::Error &error)
{
  if (real_width_ == 0 || real_height_ != 1)
    return 0;

  int l;
  if (text_width_ + 5 >= static_cast<unsigned>(real_width_))
    l = 0;
  else
    l = real_width_ - text_width_ - 5;

  // Use HorizontalLine colors.
  int attrs;
  DRAW(getAttributes(
    CppConsUI::ColorScheme::PROPERTY_HORIZONTALLINE_LINE, &attrs, error));
  DRAW(area.attrOn(attrs, error));

  int i;
  for (i = 0; i < l; i++)
    DRAW(area.addLineChar(i, 0, CppConsUI::Curses::LINE_HLINE, error));
  int printed;
  DRAW(area.addString(i, 0, text_, error, &printed));
  i += printed;
  for (; i < real_width_; i++)
    DRAW(area.addLineChar(i, 0, CppConsUI::Curses::LINE_HLINE, error));

  DRAW(area.attrOff(attrs, error));

  return 0;
}

char *Conversation::stripHTML(const char *str) const
{
  // Almost copy&paste from libpurple/util.c:purple_markup_strip_html(), but
  // this version does not convert tab character to a space.

  if (str == NULL)
    return NULL;

  int i, j, k, entlen;
  bool visible = true;
  bool closing_td_p = false;
  gchar *str2;
  const gchar *cdata_close_tag = NULL, *ent;
  gchar *href = NULL;
  int href_st = 0;

  str2 = g_strdup(str);

  for (i = 0, j = 0; str2[i] != '\0'; ++i) {
    if (str2[i] == '<') {
      if (cdata_close_tag) {
        // Note: Do not even assume any other tag is a tag in CDATA.
        if (g_ascii_strncasecmp(
              str2 + i, cdata_close_tag, !strlen(cdata_close_tag))) {
          i += strlen(cdata_close_tag) - 1;
          cdata_close_tag = NULL;
        }
        continue;
      }
      else if (!g_ascii_strncasecmp(str2 + i, "<td", 3) && closing_td_p) {
        str2[j++] = '\t';
        visible = true;
      }
      else if (!g_ascii_strncasecmp(str2 + i, "</td>", 5)) {
        closing_td_p = true;
        visible = false;
      }
      else {
        closing_td_p = false;
        visible = true;
      }

      k = i + 1;

      if (g_ascii_isspace(str2[k]))
        visible = true;
      else if (str2[k]) {
        // Scan until we end the tag either implicitly (closed start tag) or
        // explicitly, using a sloppy method (i.e., < or > inside quoted
        // attributes will screw us up).
        while (str2[k] != '\0' && str2[k] != '<' && str2[k] != '>')
          ++k;

        // If we have got an <a> tag with an href, save the address to print
        // later.
        if (g_ascii_strncasecmp(str2 + i, "<a", 2) == 0 &&
          g_ascii_isspace(str2[i + 2])) {
          int st;  // Start of href, inclusive [.
          int end; // End of href, exclusive ).
          char delim = ' ';
          // Find start of href.
          for (st = i + 3; st < k; ++st) {
            if (g_ascii_strncasecmp(str2 + st, "href=", 5) == 0) {
              st += 5;
              if (str2[st] == '"' || str2[st] == '\'') {
                delim = str2[st];
                ++st;
              }
              break;
            }
          }
          // Find end of address.
          for (end = st; end < k && str2[end] != delim; ++end) {
            // All the work is done in the loop construct above.
          }

          // If there is an address, save it. If there was already one saved,
          // kill it.
          if (st < k) {
            char *tmp;
            g_free(href);
            tmp = g_strndup(str2 + st, end - st);
            href = purple_unescape_html(tmp);
            g_free(tmp);
            href_st = j;
          }
        }

        // Replace </a> with an ascii representation of the address the link was
        // pointing to.
        else if (href != NULL &&
          g_ascii_strncasecmp(str2 + i, "</a>", 4) == 0) {
          size_t hrlen = std::strlen(href);

          // Only insert the href if it is different from the CDATA.
          // 7 == strlen("http://").
          if ((hrlen != (unsigned)(j - href_st) ||
                std::strncmp(str2 + href_st, href, hrlen)) != 0 &&
            (hrlen != (unsigned)(j - href_st + 7) ||
                std::strncmp(str2 + href_st, href + 7, hrlen - 7) != 0)) {
            str2[j++] = ' ';
            str2[j++] = '(';
            g_memmove(str2 + j, href, hrlen);
            j += hrlen;
            str2[j++] = ')';
            g_free(href);
            href = NULL;
          }
        }

        // Check for tags which should be mapped to newline (but ignore some of
        // the tags at the beginning of the text).
        else if ((j != 0 && (g_ascii_strncasecmp(str2 + i, "<p>", 3) == 0 ||
                              g_ascii_strncasecmp(str2 + i, "<tr", 3) == 0 ||
                              g_ascii_strncasecmp(str2 + i, "<hr", 3) == 0 ||
                              g_ascii_strncasecmp(str2 + i, "<li", 3) == 0 ||
                              g_ascii_strncasecmp(str2 + i, "<div", 4) == 0)) ||
          g_ascii_strncasecmp(str2 + i, "<br", 3) == 0 ||
          g_ascii_strncasecmp(str2 + i, "</table>", 8) == 0)
          str2[j++] = '\n';
        else if (g_ascii_strncasecmp(str2 + i, "<script", 7) == 0)
          cdata_close_tag = "</script>";
        else if (g_ascii_strncasecmp(str2 + i, "<style", 6) == 0)
          cdata_close_tag = "</style>";
        // Update the index and continue checking after the tag.
        i = (str2[k] == '<' || str2[k] == '\0') ? k - 1 : k;
        continue;
      }
    }
    else if (cdata_close_tag)
      continue;
    else if (!g_ascii_isspace(str2[i]))
      visible = true;

    if (str2[i] == '&' &&
      (ent = purple_markup_unescape_entity(str2 + i, &entlen))) {
      while (*ent != '\0')
        str2[j++] = *ent++;
      i += entlen - 1;
      continue;
    }

    if (visible)
      str2[j++] = g_ascii_isspace(str2[i]) && str[i] != '\t' ? ' ' : str2[i];
  }

  g_free(href);

  str2[j] = '\0';

  return str2;
}

void Conversation::buildLogFilename()
{
  PurpleAccount *account = purple_conversation_get_account(conv_);
  PurplePlugin *prpl =
    purple_find_prpl(purple_account_get_protocol_id(account));
  g_assert(prpl != NULL);

  const char *proto_name = purple_account_get_protocol_name(account);

  char *acct_name = g_strdup(purple_escape_filename(
    purple_normalize(account, purple_account_get_username(account))));

  const char *name = purple_conversation_get_name(conv_);

  filename_ = g_build_filename(purple_user_dir(), "clogs", proto_name,
    acct_name, purple_escape_filename(purple_normalize(account, name)), NULL);

  char *dir = g_path_get_dirname(filename_);
  if (g_mkdir_with_parents(dir, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
    LOG->error(_("Error creating directory '%s'."), dir);
  g_free(dir);

  g_free(acct_name);
}

char *Conversation::extractTime(time_t sent_time, time_t show_time) const
{
  // Based on the extracttime() function from cim4.

  // Convert to local time, note that localtime_r() should not really fail.
  struct tm show_time_local;
  struct tm sent_time_local;
  if (localtime_r(&show_time, &show_time_local) == NULL)
    memset(&show_time_local, 0, sizeof(show_time_local));
  if (localtime_r(&sent_time, &sent_time_local) == NULL)
    memset(&sent_time_local, 0, sizeof(sent_time_local));

  // Format the times.
  char *t1 = g_strdup(purple_date_format_long(&show_time_local));
  char *t2 = g_strdup(purple_date_format_long(&sent_time_local));

  int tdiff = abs(sent_time - show_time);

  if (tdiff > 5 && std::strcmp(t1, t2) != 0) {
    char *res = g_strdup_printf("%s [%s]", t1, t2);
    g_free(t1);
    g_free(t2);
    return res;
  }

  g_free(t2);
  return t1;
}

void Conversation::loadHistory()
{
  // Open logfile.
  GError *err = NULL;
  GIOChannel *chan = g_io_channel_new_file(filename_, "r", &err);
  if (chan == NULL) {
    LOG->error(_("Error opening conversation logfile '%s' (%s)."), filename_,
      err->message);
    g_clear_error(&err);
    return;
  }
  // This should never fail.
  g_io_channel_set_encoding(chan, NULL, NULL);

  GIOStatus st;
  char *line;
  bool new_msg = false;
  // Read conversation logfile line by line.
  while (new_msg ||
    (st = g_io_channel_read_line(chan, &line, NULL, NULL, &err)) ==
      G_IO_STATUS_NORMAL) {
    new_msg = false;

    // Start flag.
    if (std::strcmp(line, "\f\n") != 0) {
      g_free(line);
      continue;
    }
    g_free(line);

    // Parse direction (in/out).
    if ((st = g_io_channel_read_line(chan, &line, NULL, NULL, &err)) !=
      G_IO_STATUS_NORMAL)
      break;
    int color = 0;
    if (std::strcmp(line, "OUT\n") == 0)
      color = 1;
    else if (std::strcmp(line, "IN\n") == 0)
      color = 2;
    g_free(line);

    // Handle type.
    if ((st = g_io_channel_read_line(chan, &line, NULL, NULL, &err)) !=
      G_IO_STATUS_NORMAL)
      break;
    bool cim4 = true;
    if (std::strcmp(line, "MSG2\n") == 0)
      cim4 = false;
    else if (std::strcmp(line, "OTHER\n") == 0) {
      cim4 = false;
      color = 0;
    }
    g_free(line);

    // Sent time.
    if ((st = g_io_channel_read_line(chan, &line, NULL, NULL, &err)) !=
      G_IO_STATUS_NORMAL)
      break;
    time_t sent_time = atol(line);
    g_free(line);

    // Show time.
    if ((st = g_io_channel_read_line(chan, &line, NULL, NULL, &err)) !=
      G_IO_STATUS_NORMAL)
      break;
    time_t show_time = atol(line);
    g_free(line);

    if (!cim4) {
      // cim5, read only one line and strip it off HTML.
      if ((st = g_io_channel_read_line(chan, &line, NULL, NULL, &err)) !=
        G_IO_STATUS_NORMAL)
        break;

      // Validate UTF-8.
      if (!g_utf8_validate(line, -1, NULL)) {
        g_free(line);
        LOG->error(_("Invalid message detected in conversation logfile"
                     " '%s'. The message was skipped."),
          filename_);
        continue;
      }

      // Write text to the window.
      char *nohtml = stripHTML(line);
      char *time = extractTime(sent_time, show_time);
      char *msg = g_strdup_printf("%s %s", time, nohtml);
      view_->append(msg, color);
      g_free(nohtml);
      g_free(time);
      g_free(msg);
      g_free(line);
    }
    else {
      // cim4, read multiple raw lines.
      gsize length;
      std::string msg;
      while ((st = g_io_channel_read_line(chan, &line, &length, NULL, &err)) ==
          G_IO_STATUS_NORMAL &&
        line != NULL) {
        if (std::strcmp(line, "\f\n") == 0) {
          new_msg = true;
          break;
        }

        // Strip '\r' if necessary.
        if (length > 1 && line[length - 2] == '\r') {
          line[length - 2] = '\n';
          line[length - 1] = '\0';
        }
        msg.append(line);
        g_free(line);
      }

      if (!new_msg) {
        // EOL or I/O error.
        break;
      }

      // Validate UTF-8.
      if (!g_utf8_validate(msg.c_str(), -1, NULL)) {
        LOG->error(_("Invalid message detected in conversation logfile"
                     " '%s'. The message was skipped."),
          filename_);
        continue;
      }

      // Add the message to the window.
      char *time = extractTime(sent_time, show_time);
      char *final_msg = g_strdup_printf("%s %s", time, msg.c_str());
      view_->append(final_msg, color);
      g_free(time);
      g_free(final_msg);
    }
  }

  if (st != G_IO_STATUS_EOF) {
    LOG->error(_("Error reading from conversation logfile '%s' (%s)."),
      filename_, err->message);
    g_clear_error(&err);
  }
  g_io_channel_unref(chan);
}

bool Conversation::processCommand(const char *raw, const char *html)
{
  // Check that it is a command.
  if (std::strncmp(raw, "/", 1) != 0)
    return false;

  purple_conversation_write(conv_, "", html, PURPLE_MESSAGE_NO_LOG, time(NULL));

  char *error = NULL;
  // Strip the prefix and execute the command.
  PurpleCmdStatus status =
    purple_cmd_do_command(conv_, raw + 1, html + 1, &error);

  bool result = true;
  switch (status) {
  case PURPLE_CMD_STATUS_OK:
    break;
  case PURPLE_CMD_STATUS_NOT_FOUND:
    // It is not a valid command, process it as a message.
    result = false;
    break;
  case PURPLE_CMD_STATUS_WRONG_ARGS:
    purple_conversation_write(conv_, "",
      _("Wrong number of arguments passed to the command."),
      PURPLE_MESSAGE_NO_LOG, time(NULL));
    break;
  case PURPLE_CMD_STATUS_FAILED:
    purple_conversation_write(conv_, "",
      error ? error : _("The command failed for an unknown reason."),
      PURPLE_MESSAGE_NO_LOG, time(NULL));
    break;
  case PURPLE_CMD_STATUS_WRONG_TYPE:
    if (purple_conversation_get_type(conv_) == PURPLE_CONV_TYPE_IM)
      purple_conversation_write(conv_, "",
        _("The command only works in chats, not IMs."), PURPLE_MESSAGE_NO_LOG,
        time(NULL));
    else
      purple_conversation_write(conv_, "",
        _("The command only works in IMs, not chats."), PURPLE_MESSAGE_NO_LOG,
        time(NULL));
    break;
  case PURPLE_CMD_STATUS_WRONG_PRPL:
    purple_conversation_write(conv_, "",
      _("The command does not work on this protocol."), PURPLE_MESSAGE_NO_LOG,
      time(NULL));
    break;
  }

  g_free(error);

  return result;
}

void Conversation::onInputTextChange(CppConsUI::TextEdit &activator)
{
  PurpleConvIm *im = PURPLE_CONV_IM(conv_);
  if (im == NULL)
    return;

  if (!CONVERSATIONS->getSendTypingPref()) {
    input_text_length_ = 0;
    return;
  }

  size_t old_text_length = input_text_length_;
  size_t new_text_length = activator.getTextLength();
  input_text_length_ = new_text_length;

  if (new_text_length == 0) {
    // All text is deleted, turn off typing.
    purple_conv_im_stop_send_typed_timeout(im);

    serv_send_typing(purple_conversation_get_gc(conv_),
      purple_conversation_get_name(conv_), PURPLE_NOT_TYPING);
    return;
  }

  purple_conv_im_stop_send_typed_timeout(im);
  purple_conv_im_start_send_typed_timeout(im);

  time_t again = purple_conv_im_get_type_again(im);
  if ((old_text_length == 0 && new_text_length != 0) ||
    (again != 0 && time(NULL) > again)) {
    // The first letter is inserted or update is required for typing status.
    unsigned int timeout = serv_send_typing(purple_conversation_get_gc(conv_),
      purple_conversation_get_name(conv_), PURPLE_TYPING);
    purple_conv_im_set_type_again(im, timeout);
  }
}

void Conversation::actionSend()
{
  const char *str = input_->getText();
  if (str == NULL || str[0] == '\0')
    return;

  purple_idle_touch();

  char *escaped = purple_markup_escape_text(str, strlen(str));
  char *html = purple_strdup_withhtml(escaped);
  if (processCommand(str, html)) {
    // The command was processed.
  }
  else {
    PurpleConversationType type = purple_conversation_get_type(conv_);
    if (type == PURPLE_CONV_TYPE_CHAT)
      purple_conv_chat_send(PURPLE_CONV_CHAT(conv_), html);
    else if (type == PURPLE_CONV_TYPE_IM)
      purple_conv_im_send(PURPLE_CONV_IM(conv_), html);
  }
  g_free(html);
  g_free(escaped);
  input_->clear();
}

void Conversation::declareBindables()
{
  declareBindable("conversation", "send",
    sigc::mem_fun(this, &Conversation::actionSend),
    InputProcessor::BINDABLE_OVERRIDE);
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
