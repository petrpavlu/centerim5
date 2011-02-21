/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010 by CenterIM developers
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


#include "Conf.h"

#include <cstring>

#define CONF_PLUGIN_SAVE_PREF "/centerim/plugins/loaded"

Conf *Conf::instance = NULL;

Conf *Conf::Instance()
{
  return instance;
}

Conf::Conf()
{
  // load the desired plugins
  purple_plugins_load_saved(CONF_PLUGIN_SAVE_PREF);
}

Conf::~Conf()
{
  purple_plugins_save_loaded(CONF_PLUGIN_SAVE_PREF);
}

void Conf::Init()
{
  g_assert(!instance);

  instance = new Conf;
}

void Conf::Finalize()
{
  g_assert(instance);

  delete instance;
  instance = NULL;
}

int Conf::GetInt(const char *pref, int default_value)
{
  if (purple_prefs_exists(pref))
    return purple_prefs_get_int(pref);

  AddPath(pref);
  purple_prefs_add_int(pref, default_value);
  return default_value;
}

int Conf::GetInt(const char *pref, int default_value, int min, int max)
{
  if (purple_prefs_exists(pref)) {
    int i = purple_prefs_get_int(pref);
    if (i < min || i > max) {
      SetInt(pref, default_value);
      return default_value;
    }
    return i;
  }

  AddPath(pref);
  purple_prefs_add_int(pref, default_value);
  return default_value;
}

void Conf::SetInt(const char *pref, int value)
{
  purple_prefs_set_int(pref, value);
}

bool Conf::GetBool(const char *pref, bool default_value)
{
  if (purple_prefs_exists(pref))
    return purple_prefs_get_bool(pref);

  AddPath(pref);
  purple_prefs_add_bool(pref, default_value);
  return default_value;
}

void Conf::SetBool(const char *pref, const bool value)
{
  purple_prefs_set_bool(pref, value);
}

const char *Conf::GetString(const char *pref, const char *default_value)
{
  if (purple_prefs_exists(pref))
    return purple_prefs_get_string(pref);

  AddPath(pref);
  purple_prefs_add_string(pref, default_value);
  return default_value;
}

void Conf::SetString(const char *pref, const char *value)
{
  purple_prefs_set_string(pref, value);
}

void Conf::AddPath(const char *s)
{
  const char *p = s;

  while ((p = strstr(p + 1, "/"))) {
    char *a = g_strndup(s, p - s);
    purple_prefs_add_none(a);
    g_free(a);
  }
}
