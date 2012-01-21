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
#include "CoreManager.h"

#include <string.h>

namespace CppConsUI
{

namespace Keys
{

bool TermKeyCmp::operator()(const TermKeyKey& a, const TermKeyKey& b) const
{
  return termkey_keycmp(COREMANAGER->GetTermKeyHandle(), &a, &b) > 0;
}

bool Compare(const TermKeyKey& a, const TermKeyKey& b)
{
  return !termkey_keycmp(COREMANAGER->GetTermKeyHandle(), &a, &b);
}

TermKeyKey RefineKey(const TermKeyKey& k)
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

} // namespace Keys

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
