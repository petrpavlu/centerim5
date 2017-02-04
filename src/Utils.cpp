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

#include "Utils.h"

#include "Log.h"

#include "gettext.h"
#include <cerrno>
#include <cstdlib>

namespace Utils {

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

char *stripAccelerator(const char *label)
{
  g_assert(label != nullptr);

  // Calculate length of the string without accelerators.
  std::size_t len = 1;
  for (const char *p = label; *p != '\0'; ++p)
    if (*p != '_')
      ++len;

  // Allocate and populate the resulting string.
  char *res = g_new(char, len);
  char *o = res;
  for (const char *p = label; *p != '\0'; ++p)
    if (*p != '_')
      *o++ = *p;
  *o = '\0';

  return res;
}

bool stringToNumber(const char *text, long min, long max, long *out)
{
  g_assert(text != nullptr);
  g_assert(out != nullptr);

  char *endptr;
  errno = 0;
  *out = std::strtol(text, &endptr, 10);
  if (*text == '\0' || *endptr != '\0') {
    LOG->warning(_("Value is not a number."));
    return false;
  }
  if (errno == ERANGE || *out < min || *out > max) {
    *out = CLAMP(*out, min, max);
    LOG->warning(_("Value is out of range [%ld, %ld]."), min, max);
    return false;
  }
  return true;
}

} // namespace Utils

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
