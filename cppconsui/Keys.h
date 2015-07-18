/*
 * Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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

#ifndef __KEYS_H__
#define __KEYS_H__

#include "libtermkey/termkey.h"

namespace CppConsUI {

namespace Keys {

/** Compares two keys. */
struct TermKeyCmp {
  bool operator()(const TermKeyKey &a, const TermKeyKey &b) const;
};

/**
 * Changes symbol termkeys TERMKEY_SYM_TAB, TERMKEY_SYM_ENTER,
 * TERMKEY_SYM_SPACE to unicode termkeys.
 */
TermKeyKey refineKey(const TermKeyKey &k);

} // namespace Keys

} // namespace CppConsUI

#endif // __KEYS_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
