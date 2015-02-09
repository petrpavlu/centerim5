/*
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

#include "Utils.h"

namespace Utils
{

const char *getStatusIndicator(PurpleStatus *status)
{
  PurpleStatusType *status_type = purple_status_get_type(status);
  PurpleStatusPrimitive prim = purple_status_type_get_primitive(status_type);

  switch (prim) {
    case PURPLE_STATUS_UNSET:
      return "[x]";
    case PURPLE_STATUS_OFFLINE:
      return "";
    case PURPLE_STATUS_AVAILABLE:
      return "[o]";
    case PURPLE_STATUS_UNAVAILABLE:
      return "[u]";
    case PURPLE_STATUS_INVISIBLE:
      return "[i]";
    case PURPLE_STATUS_AWAY:
      return "[a]";
    case PURPLE_STATUS_EXTENDED_AWAY:
      return "[A]";
    case PURPLE_STATUS_MOBILE:
      return "[m]";
    case PURPLE_STATUS_TUNE:
      return "[t]";
    case PURPLE_STATUS_MOOD:
      return "[d]";
    default:
      return "[X]";
  }
}

char *getColorSchemeString(const char *base_color_scheme, PurpleBuddy *buddy)
{
  if (!purple_account_is_connected(purple_buddy_get_account(buddy))) {
    return g_strdup_printf("%s_offline", base_color_scheme);
  }

  PurplePresence *presence = purple_buddy_get_presence(buddy);
  PurpleStatus *status = purple_presence_get_active_status(presence);
  PurpleStatusType *status_type = purple_status_get_type(status);
  PurpleStatusPrimitive prim = purple_status_type_get_primitive(status_type);

  switch (prim) {
    case PURPLE_STATUS_UNSET:
      return g_strdup_printf("%s_offline", base_color_scheme);
    case PURPLE_STATUS_OFFLINE:
      return g_strdup_printf("%s_offline", base_color_scheme);
    case PURPLE_STATUS_AVAILABLE:
      return g_strdup_printf("%s_online", base_color_scheme);
    case PURPLE_STATUS_UNAVAILABLE:
      return g_strdup_printf("%s_na", base_color_scheme);
    case PURPLE_STATUS_INVISIBLE:
      return g_strdup_printf("%s_na", base_color_scheme);
    case PURPLE_STATUS_AWAY:
      return g_strdup_printf("%s_away", base_color_scheme);
    case PURPLE_STATUS_EXTENDED_AWAY:
      return g_strdup_printf("%s_away", base_color_scheme);
    case PURPLE_STATUS_MOBILE:
      return g_strdup_printf("%s_online", base_color_scheme);
    case PURPLE_STATUS_TUNE:
      return g_strdup_printf("%s_online", base_color_scheme);
    case PURPLE_STATUS_MOOD:
      return g_strdup_printf("%s_online", base_color_scheme);
    default:
      return g_strdup_printf("%s_offline", base_color_scheme);
  }
}

char *stripAccelerator(const char *label)
{
  g_assert(label);

  // calculate the string length without accelerators
  size_t len = 1;
  for (const char *p = label; *p; p++)
    if (*p != '_')
      len++;

  // allocate and populate the resulting string
  char *res = g_new(char, len);
  char *o = res;
  for (const char *p = label; *p; p++)
    if (*p != '_')
      *o++ = *p;
  *o = '\0';

  return res;
}

} // namespace utils

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
