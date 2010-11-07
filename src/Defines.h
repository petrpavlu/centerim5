/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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

#ifndef __DEFINES_H__
#define __DEFINES_H__

//TODO: configurable path using ./configure
#define CIM_CONFIG_PATH ".centerim5"

/* Some hardcoded defaults. */
#define CONF_DEFAULT_LOG_MAX_LINES 1000
#define CONF_DEFAULT_LOG_MIN_LINES 10

#define CONF_DEFAULT_LOG_WINDOW_HEIGHT 6
#define CONF_DEFAULT_LOG_WINDOW_WIDTH 80

/* Only change these if you know what you are doing. */
#define CONF_PREFIX "/centerim/"
#define CONF_PLUGIN_SAVE_PREF "/centerim/plugins/loaded"

#define EXCEPTION_NONE 0
#define EXCEPTION_ICONV_INIT 1
#define EXCEPTION_PURPLE_CORE_INIT 100

#endif // __DEFINES_H__
