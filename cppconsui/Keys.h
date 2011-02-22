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

#ifndef __KEYS_H__
#define __KEYS_H__

#include <glib.h>
#include <libtermkey/termkey.h>

namespace Keys
{

/** Compares two keys. */
struct TermKeyCmp
{
  bool operator()(const TermKeyKey &a, const TermKeyKey &b) const;
};

bool Compare(const TermKeyKey &a, const TermKeyKey &b);

/**
 * Changes symbol termkeys TERMKEY_SYM_TAB, TERMKEY_SYM_ENTER,
 * TERMKEY_SYM_SPACE to unicode termkeys.
 */
TermKeyKey RefineKey(const TermKeyKey &k);

TermKeyKey UnicodeTermKey(const char *symbol, int modifiers = 0);
TermKeyKey FunctionTermKey(int number, int modifiers = 0);
TermKeyKey SymbolTermKey(TermKeySym sym, int modifiers = 0);

} // namespace Keys

#endif // __KEYS_H__
