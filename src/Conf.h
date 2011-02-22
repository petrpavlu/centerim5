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

#ifndef __CONF_H__
#define __CONF_H__

#include <libpurple/purple.h>

#define CONF (Conf::Instance())

class Conf
{
public:
  static Conf *Instance();

  // configuration base get/set methods
  int GetInt(const char *pref, int default_value);
  int GetInt(const char *pref, int default_value, int min, int max);
  void SetInt(const char *pref, int value);

  bool GetBool(const char *pref, bool default_value);
  void SetBool(const char *pref, bool value);

  const char *GetString(const char *pref, const char *default_value);
  void SetString(const char *pref, const char *value);

protected:

private:
  static Conf *instance;

  Conf();
  Conf(const Conf&);
  Conf& operator=(const Conf&);
  ~Conf();

  static void Init();
  static void Finalize();
  friend class CenterIM;

  void AddPath(const char *s);
};

#endif // __CONF_H__
