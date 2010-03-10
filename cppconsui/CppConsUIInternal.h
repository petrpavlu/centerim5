/*
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

#ifndef __CPPCONSUIINTERNAL_H__
#define __CPPCONSUIINTERNAL_H__

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(s)	gettext(s)
#else
#define _(s)	(s)
#endif // ENABLE_NLS

#endif /* __CPPCONSUIINTERNAL_H__ */
