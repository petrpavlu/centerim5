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

#ifndef __GENERALMENU_H__
#define __GENERALMENU_H__

#include <cppconsui/MenuWindow.h>
#include <libpurple/purple.h>

class GeneralMenu
: public CppConsUI::MenuWindow
{
public:
  GeneralMenu();
  virtual ~GeneralMenu() {}

  // FreeWindow
  virtual void ScreenResized();

protected:

private:
  GeneralMenu(const GeneralMenu&);
  GeneralMenu& operator=(const GeneralMenu&);

  void OpenAccountWindow(CppConsUI::Button& activator);
  void OpenAddBuddyRequest(CppConsUI::Button& activator);
  void OpenAddChatRequest(CppConsUI::Button& activator);
  void OpenOptionWindow(CppConsUI::Button& activator);
  void RequestTest(CppConsUI::Button& activator);

  static void input_ok_cb_(void *data, const char *text)
    { reinterpret_cast<GeneralMenu *>(data)->input_ok_cb(text); }
  void input_ok_cb(const char *text);

  static void choice_ok_cb_(void *data, int selected)
    { reinterpret_cast<GeneralMenu *>(data)->choice_ok_cb(selected); }
  void choice_ok_cb(int selected);

  static void action_cb_(void *data, int action)
    { reinterpret_cast<GeneralMenu *>(data)->action_cb(action); }
  void action_cb(int action);

  static void fields_ok_cb_(void *data, PurpleRequestFields *fields)
    { reinterpret_cast<GeneralMenu *>(data)->fields_ok_cb(fields); }
  void fields_ok_cb(PurpleRequestFields *fields);
};

#endif // __GENERALMENU_H__
