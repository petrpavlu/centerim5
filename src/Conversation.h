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
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CONVERSATION_H
#define CONVERSATION_H

#include "ConversationRoomList.h"
#include "Log.h"

#include <cppconsui/AbstractLine.h>
#include <cppconsui/TextEdit.h>
#include <cppconsui/TextView.h>
#include <cppconsui/VerticalLine.h>
#include <cppconsui/Window.h>
#include <libpurple/purple.h>

class Conversation : public CppConsUI::Window {
public:
  explicit Conversation(PurpleConversation *conv);
  virtual ~Conversation() override;

  // InputProcessor
  virtual bool processInput(const TermKeyKey &key) override;

  // Widget
  virtual void moveResize(int newx, int newy, int neww, int newh) override;
  virtual bool restoreFocus() override;
  virtual void ungrabFocus() override;

  // Window
  virtual void show() override;
  virtual void close() override;
  virtual void onScreenResized() override;

  void write(const char *name, const char *alias, const char *message,
    PurpleMessageFlags flags, time_t mtime);

  PurpleConversation *getPurpleConversation() const { return conv_; };

  ConversationRoomList *getRoomList() const { return room_list_; };

protected:
  class ConversationLine : public CppConsUI::AbstractLine {
  public:
    ConversationLine(const char *text);
    virtual ~ConversationLine() override;

    // Widget
    virtual int draw(
      CppConsUI::Curses::ViewPort area, CppConsUI::Error &error) override;

  protected:
    char *text_;
    std::size_t text_width_;

  private:
    CONSUI_DISABLE_COPY(ConversationLine);
  };

  CppConsUI::TextView *view_;
  CppConsUI::TextEdit *input_;
  ConversationLine *line_;

  PurpleConversation *conv_;

  char *filename_;
  GIOChannel *logfile_;

  std::size_t input_text_length_;

  // Only PURPLE_CONV_TYPE_CHAT have a room list.
  ConversationRoomList *room_list_;
  CppConsUI::VerticalLine *room_list_line_;

  char *stripHTML(const char *str) const;
  void destroyPurpleConversation(PurpleConversation *conv);
  void buildLogFilename();
  char *extractTime(time_t sent_time, time_t show_time) const;
  void loadHistory();
  bool processCommand(const char *raw, const char *html);
  void onInputTextChange(CppConsUI::TextEdit &activator);

  void actionSend();

private:
  CONSUI_DISABLE_COPY(Conversation);

  void declareBindables();
};

#endif // CONVERSATION_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
