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

#include "Conversation.h"

#include "BuddyList.h"
#include "Conversations.h"
#include "Footer.h"

#include <sys/stat.h>
#include "gettext.h"

Conversation::Conversation(PurpleConversation *conv_)
: Window(0, 0, 80, 24), conv(conv_), filename(NULL), logfile(NULL)
, input_text_length(0)
{
  g_assert(conv);

  setColorScheme("conversation");

  view = new CppConsUI::TextView(width - 2, height, true, true);
  input = new CppConsUI::TextEdit(width - 2, height);
  input->signal_text_change.connect(sigc::mem_fun(this,
        &Conversation::onInputTextChange));
  char *name = g_strdup_printf("[%s] %s",
      purple_account_get_protocol_name(purple_conversation_get_account(conv)),
      purple_conversation_get_name(conv));
  line = new ConversationLine(name);
  g_free(name);
  addWidget(*view, 1, 0);
  addWidget(*input, 1, 1);
  addWidget(*line, 0, height);
  input->grabFocus();

  // open logfile
  buildLogFilename();

  GError *err = NULL;
  if (!(logfile = g_io_channel_new_file(filename, "a", &err))) {
    LOG->error(_("Error opening conversation logfile '%s' (%s)."), filename,
        err->message);
    g_clear_error(&err);
  }

  loadHistory();

  onScreenResized();
  declareBindables();
}

Conversation::~Conversation()
{
  g_free(filename);
  if (logfile)
    g_io_channel_unref(logfile);
}

bool Conversation::processInput(const TermKeyKey &key)
{
  if (view->processInput(key))
    return true;

  return Window::processInput(key);
}

void Conversation::moveResize(int newx, int newy, int neww, int newh)
{
  Window::moveResize(newx, newy, neww, newh);

  int percentage = purple_prefs_get_int(CONF_PREFIX "/chat/partitioning");
  percentage = CLAMP(percentage, 0, 100);

  int view_height = (newh * percentage) / 100;
  if (view_height < 1)
    view_height = 1;

  int input_height = newh - view_height - 1;
  if (input_height < 1)
    input_height = 1;

  view->moveResize(2, 0, neww - 4, view_height);
  input->moveResize(2, view_height + 1, neww - 4, input_height);
  line->moveResize(1, view_height, neww - 2, 1);
}

bool Conversation::restoreFocus()
{
  FOOTER->setText(_("%s buddy list, %s main menu, "
        "%s/%s/%s next/prev/act conv, %s send, %s expand"),
      "centerim|buddylist", "centerim|generalmenu",
      "centerim|conversation-next", "centerim|conversation-prev",
      "centerim|conversation-active", "conversation|send",
      "centerim|conversation-expand");

  return Window::restoreFocus();
}

void Conversation::ungrabFocus()
{
  FOOTER->setText(NULL);
  Window::ungrabFocus();
}

void Conversation::show()
{
  /* Update the scrollbar setting. It is delayed until the conversation window
   * is actually displayed, so screen lines recalculations in TextView (caused
   * by changing the scrollbar setting) aren't triggered if it isn't really
   * necessary. */
  view->setScrollBar(!CENTERIM->isEnabledExpandedConversationMode());

  Window::show();
}

void Conversation::close()
{
  signal_close(*this);

  /* Next line deletes this object. Don't touch any member variable after this
   * line. */
  purple_conversation_destroy(conv);
}

void Conversation::onScreenResized()
{
  CppConsUI::Rect r = CENTERIM->getScreenArea(CenterIM::CHAT_AREA);
  // make room for conversations list
  r.height--;

  moveResizeRect(r);
}

void Conversation::write(const char *name, const char * /*alias*/,
    const char *message, PurpleMessageFlags flags, time_t mtime)
{
  // beep on message
  if (!(flags & PURPLE_MESSAGE_SEND) &&
      purple_prefs_get_bool(CONF_PREFIX "/chat/beep_on_msg"))
    CppConsUI::Curses::beep();

  // update the last_activity property
  PurpleConversationType type = purple_conversation_get_type(conv);
  time_t cur_time = time(NULL);

  if (type == PURPLE_CONV_TYPE_IM) {
    PurpleBlistNode *bnode = PURPLE_BLIST_NODE(purple_find_buddy(
          purple_conversation_get_account(conv),
          purple_conversation_get_name(conv)));
    if (bnode) {
      purple_blist_node_set_int(bnode, "last_activity", cur_time);

      // inform the buddy list node that it should update its state
      BUDDYLIST->updateNode(bnode);
    }
  }

  // write the message
  int color;
  const char *dir;
  const char *mtype;
  if (flags & PURPLE_MESSAGE_SEND) {
    dir = "OUT";
    mtype = "MSG2"; // cim5 message
    color = 1;
  }
  else if (flags & PURPLE_MESSAGE_RECV) {
    dir = "IN";
    mtype = "MSG2"; // cim5 message
    color = 2;
  }
  else {
    dir = "IN";
    mtype = "OTHER";
    color = 0;

  }

  // write text into logfile
  if (!(flags & PURPLE_MESSAGE_NO_LOG)) {
    char *log_msg;
    if (type == PURPLE_CONV_TYPE_CHAT)
      log_msg = g_strdup_printf("\f\n%s\n%s\n%lu\n%lu\n%s: %s\n", dir, mtype,
          mtime, cur_time, name, message);
    else
      log_msg = g_strdup_printf("\f\n%s\n%s\n%lu\n%lu\n%s\n", dir, mtype,
          mtime, cur_time, message);
    if (logfile) {
      GError *err = NULL;
      if (g_io_channel_write_chars(logfile, log_msg, -1, NULL, &err)
          != G_IO_STATUS_NORMAL) {
        LOG->error(_("Error writing to conversation logfile (%s)."),
            err->message);
        g_clear_error(&err);
      }
      if (g_io_channel_flush(logfile, &err) != G_IO_STATUS_NORMAL) {
        LOG->error(_("Error flushing conversation logfile (%s)."),
            err->message);
        g_clear_error(&err);
      }
    }
    g_free(log_msg);
  }

  // we currently don't support displaying HTML in any way
  char *nohtml = stripHTML(message);

  // write text to the window
  char *time = extractTime(mtime, cur_time);
  char *msg;
  if (type == PURPLE_CONV_TYPE_CHAT)
    msg = g_strdup_printf("%s %s: %s", time, name, nohtml);
  else
    msg = g_strdup_printf("%s %s", time, nohtml);
  view->append(msg, color);
  g_free(nohtml);
  g_free(time);
  g_free(msg);
}

Conversation::ConversationLine::ConversationLine(const char *text_)
: AbstractLine(AUTOSIZE, 1)
{
  g_assert(text_);

  text = g_strdup(text_);
  text_width = CppConsUI::Curses::onScreenWidth(text);
}

Conversation::ConversationLine::~ConversationLine()
{
  g_free(text);
}

void Conversation::ConversationLine::draw(CppConsUI::Curses::ViewPort area)
{
  if (real_width == 0 || real_height != 1)
    return;

  int l;
  if (text_width + 5 >= static_cast<unsigned>(real_width))
    l = 0;
  else
    l = real_width - text_width - 5;

  // use HorizontalLine colors
  int attrs = getColorPair("horizontalline", "line");
  area.attrOn(attrs);

  int i;
  for (i = 0; i < l; i++)
    area.addLineChar(i, 0, CppConsUI::Curses::LINE_HLINE);
  i += area.addString(i, 0, text);
  for (; i < real_width; i++)
    area.addLineChar(i, 0, CppConsUI::Curses::LINE_HLINE);

  area.attrOff(attrs);
}

char *Conversation::stripHTML(const char *str) const
{
  /* Almost copy&paste from libpurple/util.c:purple_markup_strip_html(), but
   * this version doesn't convert tab character to a space. */

  int i, j, k, entlen;
  bool visible = true;
  bool closing_td_p = false;
  gchar *str2;
  const gchar *cdata_close_tag = NULL, *ent;
  gchar *href = NULL;
  int href_st = 0;

  if (!str)
    return NULL;

  str2 = g_strdup(str);

  for (i = 0, j = 0; str2[i]; i++) {
    if (str2[i] == '<') {
      if (cdata_close_tag) {
        // note: don't even assume any other tag is a tag in CDATA
        if (g_ascii_strncasecmp(str2 + i, cdata_close_tag,
              !strlen(cdata_close_tag))) {
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
        /* Scan until we end the tag either implicitly (closed start tag) or
         * explicitly, using a sloppy method (i.e., < or > inside quoted
         * attributes will screw us up). */
        while (str2[k] && str2[k] != '<' && str2[k] != '>')
          k++;

        /* If we've got an <a> tag with an href, save the address to print
         * later. */
        if (!g_ascii_strncasecmp(str2 + i, "<a", 2) &&
            g_ascii_isspace(str2[i + 2])) {
          int st; // start of href, inclusive [
          int end; // end of href, exclusive )
          char delim = ' ';
          // find start of href
          for (st = i + 3; st < k; st++) {
            if (!g_ascii_strncasecmp(str2 + st, "href=", 5)) {
              st += 5;
              if (str2[st] == '"' || str2[st] == '\'') {
                delim = str2[st];
                st++;
              }
              break;
            }
          }
          // find end of address
          for (end = st; end < k && str2[end] != delim; end++) {
            // all the work is done in the loop construct above
          }

          /* If there's an address, save it. If there was already one saved,
           * kill it. */
          if (st < k) {
            char *tmp;
            g_free(href);
            tmp = g_strndup(str2 + st, end - st);
            href = purple_unescape_html(tmp);
            g_free(tmp);
            href_st = j;
          }
        }

        /* Replace </a> with an ascii representation of the address the link
         * was pointing to. */
        else if (href && !g_ascii_strncasecmp(str2 + i, "</a>", 4)) {
          size_t hrlen = strlen(href);

          /* Only insert the href if it's different from the CDATA.
           *  7 == strlen("http://") */
          if ((hrlen != (unsigned)(j - href_st) ||
                strncmp(str2 + href_st, href, hrlen)) &&
              (hrlen != (unsigned)(j - href_st + 7) ||
               strncmp(str2 + href_st, href + 7, hrlen - 7)))
          {
            str2[j++] = ' ';
            str2[j++] = '(';
            g_memmove(str2 + j, href, hrlen);
            j += hrlen;
            str2[j++] = ')';
            g_free(href);
            href = NULL;
          }
        }

        /* Check for tags which should be mapped to newline (but ignore some
         * of the tags at the beginning of the text) */
        else if ((j && (!g_ascii_strncasecmp(str2 + i, "<p>", 3) ||
                !g_ascii_strncasecmp(str2 + i, "<tr", 3) ||
                !g_ascii_strncasecmp(str2 + i, "<hr", 3) ||
                !g_ascii_strncasecmp(str2 + i, "<li", 3) ||
                !g_ascii_strncasecmp(str2 + i, "<div", 4))) ||
            !g_ascii_strncasecmp(str2 + i, "<br", 3) ||
            !g_ascii_strncasecmp(str2 + i, "</table>", 8))
          str2[j++] = '\n';
        // check for tags which begin CDATA and need to be closed
#if 0 // FIXME.. option is end tag optional, we can't handle this right now
        else if (!g_ascii_strncasecmp(str2 + i, "<option", 7))
        {
          // FIXME we should not do this if the OPTION is SELECT'd
          cdata_close_tag = "</option>";
        }
#endif
        else if (!g_ascii_strncasecmp(str2 + i, "<script", 7))
          cdata_close_tag = "</script>";
        else if (!g_ascii_strncasecmp(str2 + i, "<style", 6))
          cdata_close_tag = "</style>";
        // update the index and continue checking after the tag
        i = (str2[k] == '<' || str2[k] == '\0') ? k - 1: k;
        continue;
      }
    }
    else if (cdata_close_tag)
      continue;
    else if (!g_ascii_isspace(str2[i]))
      visible = true;

    if (str2[i] == '&' && (ent = purple_markup_unescape_entity(str2 + i,
            &entlen))) {
      while (*ent)
        str2[j++] = *ent++;
      i += entlen - 1;
      continue;
    }

    if (visible)
      str2[j++] = g_ascii_isspace(str2[i]) && str[i] != '\t' ? ' ': str2[i];
  }

  g_free(href);

  str2[j] = '\0';

  return str2;
}

void Conversation::buildLogFilename()
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

  filename = g_build_filename(purple_user_dir(), "clogs", proto_name,
      acct_name, purple_escape_filename(purple_normalize(account, name)),
      NULL);

  dir = g_path_get_dirname(filename);
  if (g_mkdir_with_parents(dir, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
    LOG->error(_("Error creating directory '%s'."), dir);
  g_free(dir);

  g_free(acct_name);
}

char *Conversation::extractTime(time_t sent_time, time_t show_time) const
{
  // based on the extracttime() function from cim4

  // convert to local time, note that localtime_r() shouldn't really fail
  struct tm show_time_local;
  struct tm sent_time_local;
  if (!localtime_r(&show_time, &show_time_local))
    memset(&show_time_local, 0, sizeof(show_time_local));
  if (!localtime_r(&sent_time, &sent_time_local))
    memset(&sent_time_local, 0, sizeof(sent_time_local));

  // format the times
  char *t1 = g_strdup(purple_date_format_long(&show_time_local));
  char *t2 = g_strdup(purple_date_format_long(&sent_time_local));

  int tdiff = abs(sent_time - show_time);

  if (tdiff > 5 && strcmp(t1, t2)) {
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
  // open logfile
  GError *err = NULL;
  GIOChannel *chan;

  if ((chan = g_io_channel_new_file(filename, "r", &err)) == NULL) {
    LOG->error(_("Error opening conversation logfile '%s' (%s)."), filename,
        err->message);
    g_clear_error(&err);
    return;
  }
  // this should never fail
  g_io_channel_set_encoding(chan, NULL, NULL);

  GIOStatus st;
  char *line;
  bool new_msg = false;
  // read conversation logfile line by line
  while (new_msg || (st = g_io_channel_read_line(chan, &line, NULL, NULL,
          &err)) == G_IO_STATUS_NORMAL) {
    new_msg = false;

    // start flag
    if (strcmp(line, "\f\n")) {
      g_free(line);
      continue;
    }
    g_free(line);

    // parse direction (in/out)
    if ((st = g_io_channel_read_line(chan, &line, NULL, NULL, &err))
        != G_IO_STATUS_NORMAL)
      break;
    int color = 0;
    if (!strcmp(line, "OUT\n"))
      color = 1;
    else if (!strcmp(line, "IN\n"))
      color = 2;
    g_free(line);

    // type
    if ((st = g_io_channel_read_line(chan, &line, NULL, NULL, &err))
        != G_IO_STATUS_NORMAL)
      break;
    bool cim4 = true;
    if (!strcmp(line, "MSG2\n"))
      cim4 = false;
    else if (!strcmp(line, "OTHER\n")) {
      cim4 = false;
      color = 0;
    }
    g_free(line);

    // sent time
    if ((st = g_io_channel_read_line(chan, &line, NULL, NULL, &err))
        != G_IO_STATUS_NORMAL)
      break;
    time_t sent_time = atol(line);
    g_free(line);

    // show time
    if ((st = g_io_channel_read_line(chan, &line, NULL, NULL, &err))
        != G_IO_STATUS_NORMAL)
      break;
    time_t show_time = atol(line);
    g_free(line);

    if (!cim4) {
      // cim5, read only one line and strip it off HTML
      if ((st = g_io_channel_read_line(chan, &line, NULL, NULL, &err))
          != G_IO_STATUS_NORMAL)
        break;

      // validate UTF-8
      if (!g_utf8_validate(line, -1, NULL)) {
        g_free(line);
        LOG->error(_("Invalid message detected in conversation logfile"
              " '%s'. The message was skipped."), filename);
        continue;
      }

      // write text to the window
      char *nohtml = stripHTML(line);
      char *time = extractTime(sent_time, show_time);
      char *msg = g_strdup_printf("%s %s", time, nohtml);
      view->append(msg, color);
      g_free(nohtml);
      g_free(time);
      g_free(msg);
      g_free(line);
    }
    else {
      // cim4, read multiple raw lines
      gsize length;
      std::string msg;
      while ((st = g_io_channel_read_line(chan, &line, &length, NULL, &err))
          == G_IO_STATUS_NORMAL && line != NULL) {
        if (!strcmp(line, "\f\n")) {
          new_msg = true;
          break;
        }

        // strip '\r' if necessary
        if (length > 1 && line[length - 2] == '\r') {
          line[length - 2] = '\n';
          line[length - 1] = '\0';
        }
        msg.append(line);
        g_free(line);
      }

      if (!new_msg) {
        // EOL or I/O error
        break;
      }

      // validate UTF-8
      if (!g_utf8_validate(msg.c_str(), -1, NULL)) {
        LOG->error(_("Invalid message detected in conversation logfile"
              " '%s'. The message was skipped."), filename);
        continue;
      }

      // add the message to the window
      char *time = extractTime(sent_time, show_time);
      char *final_msg = g_strdup_printf("%s %s", time, msg.c_str());
      view->append(final_msg, color);
      g_free(time);
      g_free(final_msg);
    }
  }

  if (st != G_IO_STATUS_EOF) {
    LOG->error(_("Error reading from conversation logfile '%s' (%s)."),
        filename, err->message);
    g_clear_error(&err);
  }
  g_io_channel_unref(chan);
}

bool Conversation::processCommand(const char *raw, const char *html)
{
  // check that it is a command
  if (strncmp(raw, "/", 1))
    return false;

  purple_conversation_write(conv, "", html, PURPLE_MESSAGE_NO_LOG,
      time(NULL));

  char *error = NULL;
  // strip the prefix and execute the command
  PurpleCmdStatus status = purple_cmd_do_command(conv, raw + 1, html + 1,
      &error);

  bool result = true;
  switch (status) {
    case PURPLE_CMD_STATUS_OK:
      break;
    case PURPLE_CMD_STATUS_NOT_FOUND:
      // it isn't a valid command, send it as a message
      result = false;
      break;
    case PURPLE_CMD_STATUS_WRONG_ARGS:
      purple_conversation_write(conv, "",
          _("Wrong number of arguments passed to the command."),
          PURPLE_MESSAGE_NO_LOG, time(NULL));
      break;
    case PURPLE_CMD_STATUS_FAILED:
      purple_conversation_write(conv, "",
          error ? error : _("The command failed for an unknown reason."),
          PURPLE_MESSAGE_NO_LOG, time(NULL));
      break;
    case PURPLE_CMD_STATUS_WRONG_TYPE:
      if (purple_conversation_get_type(conv) == PURPLE_CONV_TYPE_IM)
        purple_conversation_write(conv, "",
            _("The command only works in chats, not IMs."),
            PURPLE_MESSAGE_NO_LOG, time(NULL));
      else
        purple_conversation_write(conv, "",
            _("The command only works in IMs, not chats."),
            PURPLE_MESSAGE_NO_LOG, time(NULL));
      break;
    case PURPLE_CMD_STATUS_WRONG_PRPL:
      purple_conversation_write(conv, "",
          _("The command does not work on this protocol."),
          PURPLE_MESSAGE_NO_LOG, time(NULL));
      break;
  }

  g_free(error);

  return result;
}

void Conversation::onInputTextChange(CppConsUI::TextEdit &activator)
{
  PurpleConvIm *im = PURPLE_CONV_IM(conv);
  if (!im)
    return;

  if (!CONVERSATIONS->getSendTypingPref()) {
    input_text_length = 0;
    return;
  }

  size_t old_text_length = input_text_length;
  size_t new_text_length = activator.getTextLength();
  input_text_length = new_text_length;

  if (!new_text_length) {
    // all text is deleted, turn off typing
    purple_conv_im_stop_send_typed_timeout(im);

    serv_send_typing(purple_conversation_get_gc(conv),
        purple_conversation_get_name(conv), PURPLE_NOT_TYPING);
    return;
  }

  purple_conv_im_stop_send_typed_timeout(im);
  purple_conv_im_start_send_typed_timeout(im);

  time_t again = purple_conv_im_get_type_again(im);
  if ((!old_text_length && new_text_length) ||
      (again && time(NULL) > again)) {
    // the first letter is inserted or update is required for typing status
    unsigned int timeout = serv_send_typing(purple_conversation_get_gc(conv),
        purple_conversation_get_name(conv), PURPLE_TYPING);
    purple_conv_im_set_type_again(im, timeout);
  }
}

void Conversation::actionSend()
{
  const char *str = input->getText();
  if (!str || !str[0])
    return;

  purple_idle_touch();

  char *escaped = purple_markup_escape_text(str, strlen(str));
  char *html = purple_strdup_withhtml(escaped);
  if (processCommand(str, html)) {
    // the command was processed
  }
  else {
    PurpleConversationType type = purple_conversation_get_type(conv);
    if (type == PURPLE_CONV_TYPE_CHAT)
      purple_conv_chat_send(PURPLE_CONV_CHAT(conv), html);
    else if (type == PURPLE_CONV_TYPE_IM)
      purple_conv_im_send(PURPLE_CONV_IM(conv), html);
  }
  g_free(html);
  g_free(escaped);
  input->clear();
}

void Conversation::declareBindables()
{
  declareBindable("conversation", "send",
      sigc::mem_fun(this, &Conversation::actionSend),
      InputProcessor::BINDABLE_OVERRIDE);
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
