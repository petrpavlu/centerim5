/*
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

#ifndef __NOTIFY_H__
#define __NOTIFY_H__

#include <libpurple/purple.h>

#define NOTIFY (Notify::Instance())

class Notify
{
public:
  static Notify *Instance();

protected:

private:
  PurpleNotifyUiOps centerim_notify_ui_ops;

  static Notify *instance;

  Notify();
  Notify(const Notify&);
  Notify& operator=(const Notify&);
  ~Notify();

  static void Init();
  static void Finalize();
  friend class CenterIM;

  static void *notify_message_(PurpleNotifyMsgType type, const char *title,
      const char *primary, const char *secondary)
    { return NOTIFY->notify_message(type, title, primary, secondary); }

  void *notify_message(PurpleNotifyMsgType type, const char *title,
      const char *primary, const char *secondary);
};

#endif // __NOTIFY_H__
