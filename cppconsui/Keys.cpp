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

#include "Keys.h"

#include <cstring>

namespace Keys
{

bool TermKeyCmp::operator()(const TermKeyKey &a, const TermKeyKey &b) const
{
  if (a.type != b.type)
    return a.type > b.type;

  if (a.modifiers != b.modifiers)
    return a.modifiers > b.modifiers;

  switch (a.type) {
    case TERMKEY_TYPE_UNICODE:
      return a.code.codepoint > b.code.codepoint;
    case TERMKEY_TYPE_FUNCTION:
      return a.code.number > b.code.number;
    case TERMKEY_TYPE_KEYSYM:
      return a.code.sym > b.code.sym;
    default:
      g_assert_not_reached();
  }
  return false;
}

bool Compare(const TermKeyKey &a, const TermKeyKey &b)
{
  if (a.type != b.type)
    return false;

  if (a.modifiers != b.modifiers)
    return false;

  switch (a.type) {
    case TERMKEY_TYPE_UNICODE:
      if (a.code.codepoint == b.code.codepoint)
        return true;
      break;
    case TERMKEY_TYPE_FUNCTION:
      if (a.code.number == b.code.number)
        return true;
      break;
    case TERMKEY_TYPE_KEYSYM:
      if (a.code.sym == b.code.sym)
        return true;
      break;
    default:
      break;
  }
  return false;
}

TermKeyKey RefineKey(const TermKeyKey &k)
{
  if (k.type != TERMKEY_TYPE_KEYSYM)
    return k;

  TermKeyKey res = k;
  if (res.code.sym == TERMKEY_SYM_TAB) {
    res.type = TERMKEY_TYPE_UNICODE;
    strcpy(res.utf8, "\t");
    res.code.codepoint = g_utf8_get_char(res.utf8);
  }
  else if (res.code.sym == TERMKEY_SYM_ENTER) {
    res.type = TERMKEY_TYPE_UNICODE;
    strcpy(res.utf8, "\n");
    res.code.codepoint = g_utf8_get_char(res.utf8);
  }
  else if (res.code.sym == TERMKEY_SYM_SPACE) {
    res.type = TERMKEY_TYPE_UNICODE;
    strcpy(res.utf8, " ");
    res.code.codepoint = g_utf8_get_char(res.utf8);
  }
  return res;
}

TermKeyKey UnicodeTermKey(const char *symbol, int modifiers)
{
  g_assert(symbol);
  int len = strlen(symbol);
  g_assert(len <= 6);

  TermKeyKey res = {TERMKEY_TYPE_UNICODE, {0}, modifiers, ""};
  res.code.codepoint = g_utf8_get_char(symbol);
  memcpy(res.utf8, symbol, len + 1);

  return res;
}

TermKeyKey FunctionTermKey(int number, int modifiers)
{
  TermKeyKey res = {TERMKEY_TYPE_FUNCTION, {0}, modifiers, ""};
  res.code.number = number;
  return res;
}

TermKeyKey SymbolTermKey(TermKeySym sym, int modifiers)
{
  TermKeyKey res = {TERMKEY_TYPE_KEYSYM, {0}, modifiers, ""};
  res.code.sym = sym;
  return res;
}

} // namespace Keys
