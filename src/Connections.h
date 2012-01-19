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

#ifndef __CONNECTIONS_H__
#define __CONNECTIONS_H__

#include <libpurple/purple.h>

#define CONNECTIONS (Connections::Instance())

class Connections
{
public:
  static Connections *Instance();

protected:

private:
  PurpleConnectionUiOps centerim_connection_ui_ops;

  static Connections *instance;

  Connections();
  Connections(const Connections&);
  Connections& operator=(const Connections&);
  ~Connections();

  static void Init();
  static void Finalize();
  friend class CenterIM;

  void AccountReconnect(PurpleAccount *account);

  static void connect_progress_(PurpleConnection *gc, const char *text,
      size_t step, size_t step_count)
    { CONNECTIONS->connect_progress(gc, text, step, step_count); }
  static void connected_(PurpleConnection *gc)
    { CONNECTIONS->connected(gc) ;}
  static void disconnected_(PurpleConnection *gc)
    { CONNECTIONS->disconnected(gc); }
  static void notice_(PurpleConnection *gc, const char *text)
    { CONNECTIONS->notice(gc, text); }
  static void network_connected_()
    { CONNECTIONS->network_connected(); }
  static void network_disconnected_()
    { CONNECTIONS->network_disconnected(); }
  static void report_disconnect_reason_(PurpleConnection *gc,
      PurpleConnectionError reason, const char *text)
    { CONNECTIONS->report_disconnect_reason(gc, reason, text); }

  void connect_progress(PurpleConnection *gc, const char *text, size_t step,
      size_t step_count);
  void connected(PurpleConnection *gc);
  void disconnected(PurpleConnection *gc);
  void notice(PurpleConnection *gc, const char *text);
  void network_connected();
  void network_disconnected();
  void report_disconnect_reason(PurpleConnection *gc,
      PurpleConnectionError reason, const char *text);
};

#endif // __CONNECTIONS_H__

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
