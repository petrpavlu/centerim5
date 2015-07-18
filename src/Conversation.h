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

#ifndef __CONVERSATION_H__
#define __CONVERSATION_H__

#include "Log.h"
#include "ConversationRoomList.h"

#include <cppconsui/AbstractLine.h>
#include <cppconsui/VerticalLine.h>
#include <cppconsui/TextEdit.h>
#include <cppconsui/TextView.h>
#include <cppconsui/Window.h>
#include <libpurple/purple.h>

class Conversation : public CppConsUI::Window {
public:
  Conversation(PurpleConversation *conv_);
  virtual ~Conversation();

  // InputProcessor
  virtual bool processInput(const TermKeyKey &key);

  // Widget
  virtual void moveResize(int newx, int newy, int neww, int newh);
  virtual bool restoreFocus();
  virtual void ungrabFocus();

  // Window
  virtual void show();
  virtual void close();
  virtual void onScreenResized();

  void write(const char *name, const char *alias, const char *message,
    PurpleMessageFlags flags, time_t mtime);

  PurpleConversation *getPurpleConversation() const { return conv; };

  ConversationRoomList *getRoomList() const { return room_list; };

protected:
  class ConversationLine : public CppConsUI::AbstractLine {
  public:
    ConversationLine(const char *text_);
    virtual ~ConversationLine();

    // Widget
    virtual void draw(CppConsUI::Curses::ViewPort area);

  protected:
    char *text;
    size_t text_width;

  private:
    CONSUI_DISABLE_COPY(ConversationLine);
  };

  CppConsUI::TextView *view;
  CppConsUI::TextEdit *input;
  ConversationLine *line;

  PurpleConversation *conv;

  char *filename;
  GIOChannel *logfile;

  size_t input_text_length;

  // only PURPLE_CONV_TYPE_CHAT have a room list
  ConversationRoomList *room_list;
  CppConsUI::VerticalLine *room_list_line;

  char *stripHTML(const char *str) const;
  void destroyPurpleConversation(PurpleConversation *conv);
  void buildLogFilename();
  char *extractTime(time_t sent_time, time_t show_time) const;
  void loadHistory();
  bool processCommand(const char *raw, const char *html);
  void onInputTextChange(CppConsUI::TextEdit &activator);

  void actionSend();

private:
  Conversation();
  CONSUI_DISABLE_COPY(Conversation);

  void declareBindables();
};

#endif // __CONVERSATION_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
