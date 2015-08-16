/*
 * Copyright (C) 2013-2015 Petr Pavlu <setup@dagobah.cz>
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

#include "CppConsUI.h"

#include "ColorScheme.h"
#include "CoreManager.h"
#include "KeyConfig.h"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace CppConsUI {

ColorScheme *color_scheme = NULL;
CoreManager *core_manager = NULL;
KeyConfig *key_config = NULL;

Error::Error(ErrorCode code, const char *string)
  : error_code(code), error_string(NULL)
{
  setString(string);
}

Error::Error(const Error &other)
{
  assert(other.error_string != NULL);

  error_code = other.error_code;

  size_t size = std::strlen(other.error_string) + 1;
  error_string = new char[size];
  std::strcpy(error_string, other.error_string);
}

Error &Error::operator=(const Error &other)
{
  assert(other.error_string != NULL);

  size_t size = std::strlen(other.error_string) + 1;
  char *new_string = new char[size];
  std::strcpy(new_string, other.error_string);

  error_code = other.error_code;
  delete[] error_string;
  error_string = new_string;
}

Error::~Error()
{
  delete[] error_string;
}

void Error::setCode(ErrorCode code)
{
  error_code = code;
}

void Error::setString(const char *string)
{
  size_t size = 1;
  if (string != NULL)
    size += std::strlen(string);
  char *new_string = new char[size];

  if (string != NULL)
    std::strcpy(new_string, string);
  else
    new_string[0] = '\0';

  delete[] error_string;
  error_string = new_string;
}

void Error::setFormattedString(const char *format, ...)
{
  assert(format != NULL);

  va_list args;

  va_start(args, format);
  int size = std::vsnprintf(NULL, 0, format, args) + 1;
  char *new_string = new char[size];
  std::vsprintf(new_string, format, args);
  va_end(args);

  delete[] error_string;
  error_string = new_string;
}

void Error::clear()
{
  error_code = ERROR_NONE;
  delete[] error_string;
  error_string = NULL;
}

void initializeConsUI(AppInterface &interface)
{
  assert(color_scheme == NULL);
  assert(core_manager == NULL);
  assert(key_config == NULL);

  // Initialize ColorScheme and KeyConfig. These cannot fail.
  color_scheme = new ColorScheme;
  key_config = new KeyConfig;

  // CoreManager depends on KeyConfig so it has to be initialized after it.
  core_manager = new CoreManager(interface);
}

void finalizeConsUI()
{
  assert(color_scheme != NULL);
  assert(core_manager != NULL);
  assert(key_config != NULL);

  // Destroy CoreManager, KeyConfig and ColorScheme.
  delete core_manager;
  core_manager = NULL;
  delete key_config;
  key_config = NULL;
  delete color_scheme;
  color_scheme = NULL;
}

ColorScheme *getColorSchemeInstance()
{
  assert(color_scheme);
  return color_scheme;
}

CoreManager *getCoreManagerInstance()
{
  assert(core_manager);
  return core_manager;
}

KeyConfig *getKeyConfigInstance()
{
  assert(key_config);
  return key_config;
}

namespace UTF8 {

// some code below is based on the GLib code

/*
 * Bits  Length  Byte 1    Byte 2    Byte 3    Byte 4    Byte 5    Byte 6
 *   7     1     0xxxxxxx
 *  11     2     110xxxxx  10xxxxxx
 *  16     3     1110xxxx  10xxxxxx  10xxxxxx
 *  21     4     11110xxx  10xxxxxx  10xxxxxx  10xxxxxx
 *  26     5     111110xx  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx
 *  31     6     1111110x  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx
 */
UniChar getUniChar(const char *p)
{
  assert(p);

  UniChar res;
  unsigned char c = *p++;
  int rest;

  if ((c & 0x80) == 0x00)
    return c & 0x7f;
  else if ((c & 0xe0) == 0xc0) {
    rest = 1;
    res = c & 0x1f;
  }
  else if ((c & 0xf0) == 0xe0) {
    rest = 2;
    res = c & 0x0f;
  }
  else if ((c & 0xf8) == 0xf0) {
    rest = 3;
    res = c & 0x07;
  }
  else if ((c & 0xfc) == 0xf8) {
    rest = 4;
    res = c & 0x03;
  }
  else if ((c & 0xfe) == 0xfc) {
    rest = 5;
    res = c & 0x01;
  }
  else
    return -1;

  while (rest--) {
    c = *p++;
    if ((c & 0xc0) != 0x80)
      return -1;
    res <<= 6;
    res |= (c & 0x3f);
  }

  return res;
}

struct Interval {
  UniChar start, end;
};

static int interval_compare(const void *key, const void *elt)
{
  const UniChar uc = *static_cast<const UniChar *>(key);
  const Interval *interval = static_cast<const Interval *>(elt);

  if (uc < interval->start)
    return -1;
  if (uc > interval->end)
    return +1;

  return 0;
}

bool isUniCharWide(UniChar uc)
{
  static const Interval wide[] = {{0x1100, 0x115F}, {0x2329, 0x232A},
    {0x2E80, 0x2E99}, {0x2E9B, 0x2EF3}, {0x2F00, 0x2FD5}, {0x2FF0, 0x2FFB},
    {0x3000, 0x303E}, {0x3041, 0x3096}, {0x3099, 0x30FF}, {0x3105, 0x312D},
    {0x3131, 0x318E}, {0x3190, 0x31BA}, {0x31C0, 0x31E3}, {0x31F0, 0x321E},
    {0x3220, 0x3247}, {0x3250, 0x32FE}, {0x3300, 0x4DBF}, {0x4E00, 0xA48C},
    {0xA490, 0xA4C6}, {0xA960, 0xA97C}, {0xAC00, 0xD7A3}, {0xF900, 0xFAFF},
    {0xFE10, 0xFE19}, {0xFE30, 0xFE52}, {0xFE54, 0xFE66}, {0xFE68, 0xFE6B},
    {0xFF01, 0xFF60}, {0xFFE0, 0xFFE6}, {0x1B000, 0x1B001}, {0x1F200, 0x1F202},
    {0x1F210, 0x1F23A}, {0x1F240, 0x1F248}, {0x1F250, 0x1F251},
    {0x20000, 0x2FFFD}, {0x30000, 0x3FFFD}};

  if (std::bsearch(&uc, wide, sizeof(wide) / sizeof(wide[0]), sizeof(wide[0]),
        interval_compare))
    return true;

  return false;
}

bool isUniCharDigit(UniChar uc)
{
  // note: this function does not behave according to the Unicode standard

  if (uc > '0' && uc < '9')
    return true;
  return false;
}

bool isUniCharSpace(UniChar uc)
{
  // note: this function does not behave according to the Unicode standard

  if (uc == '\t' || uc == '\n' || uc == '\r' || uc == '\f')
    return true;
  return false;
}

static const char utf8_skip_data[256] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x00-0x0f */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x10-0x1f */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x20-0x2f */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x30-0x3f */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x40-0x4f */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x50-0x5f */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x60-0x6f */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x70-0x7f */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x80-0x8f */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x90-0x9f */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0xa0-0xaf */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0xb0-0xbf */
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /* 0xc0-0xcf */
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /* 0xd0-0xdf */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0xe0-0xef */
  4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1  /* 0xf0-0xff */
};

const char *getNextChar(const char *p)
{
  return p + utf8_skip_data[static_cast<unsigned char>(*p)];
}

const char *getPrevChar(const char *p)
{
  while (true) {
    p--;
    if ((*p & 0xc0) != 0x80)
      return p;
  }
}

const char *findNextChar(const char *p, const char *end)
{
  if (!end)
    return getNextChar(p);

  while (p + 1 < end) {
    p++;
    if ((*p & 0xc0) != 0x80)
      return p;
  }
  return NULL;
}

const char *findPrevChar(const char *start, const char *p)
{
  while (p > start) {
    p--;
    if ((*p & 0xc0) != 0x80)
      return p;
  }
  return NULL;
}

} // namespace UTF8

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
