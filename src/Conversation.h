/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2012 by CenterIM developers
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

#include <cppconsui/HorizontalLine.h>
#include <cppconsui/TextEdit.h>
#include <cppconsui/TextView.h>
#include <cppconsui/Window.h>
#include <libpurple/purple.h>

class Conversation
: public CppConsUI::Window
{
public:
  Conversation(PurpleConversation *conv_);
  virtual ~Conversation();

  // InputProcessor
  virtual bool ProcessInput(const TermKeyKey& key);

  // Widget
  virtual void MoveResize(int newx, int newy, int neww, int newh);
  virtual bool RestoreFocus();
  virtual void UngrabFocus();

  // FreeWindow
  virtual void Close();
  virtual void OnScreenResized();

  void Write(const char *name, const char *alias, const char *message,
    PurpleMessageFlags flags, time_t mtime);

  PurpleConversation *GetPurpleConversation() const { return conv; };

protected:
  CppConsUI::TextView *view;
  CppConsUI::TextEdit *input;
  CppConsUI::HorizontalLine *line;

  PurpleConversation *conv;

  char *filename;
  GIOChannel *logfile;

  char *StripHTML(const char *str) const;
  void DestroyPurpleConversation(PurpleConversation *conv);
  void BuildLogFilename();
  char *ExtractTime(time_t sent_time, time_t show_time) const;
  void LoadHistory();

  void ActionSend();

private:
  Conversation();
  Conversation(const Conversation&);
  Conversation& operator=(const Conversation&);

  void DeclareBindables();
  void Beep();

  bool beep_on_msg;
};

#endif // __CONVERSATION_H__

/* vim: set tabstop=2 shiftwidth=2 tw=78 expandtab : */
