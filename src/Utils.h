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

#ifndef UTILS_H
#define UTILS_H

#include <libpurple/purple.h>

namespace Utils {

const char *getStatusIndicator(PurpleStatus *status);
char *stripAccelerator(const char *label);

// Converts a string to a number. Logs a warning if the value is non-numeric or
// out-of-range. Returns true if the conversion was successful, false otherwise.
bool stringToNumber(const char *text, long min, long max, long *out);

} // namespace Utils

#endif // UTILS_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
