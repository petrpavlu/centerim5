/* GTK - The GIMP Toolkit
 * gtktexttypes.h Copyright (C) 2000 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __GTK_TEXT_TYPES_H__
#define __GTK_TEXT_TYPES_H__

//#include <gtk/gtk.h>
//#include <gtk/gtktexttagprivate.h>

#include "TextSegment.h"

G_BEGIN_DECLS

/*
typedef struct _GtkTextCounter GtkTextCounter;
typedef struct _GtkTextLineSegment GtkTextLineSegment;
typedef struct _GtkTextLineSegmentType GtkTextLineSegmentType;
typedef struct _GtkTextToggleBody GtkTextToggleBody;
typedef struct _GtkTextMarkBody GtkTextMarkBody;
*/

/*
 * Declarations for variables shared among the text-related files:
 */

/*
#ifdef G_OS_WIN32
#ifdef GTK_COMPILATION
#define VARIABLE __declspec(dllexport)
#else
#define VARIABLE extern __declspec(dllimport)
#endif
#else
#define VARIABLE extern
#endif
*/


/* In gtktextbtree.c */
//class TextLineSegmentChar;
//class TextLineSegmentToggle;
//extern const TextLineSegmentType text_char_type;
//extern const TextLineSegmentType text_toggle_on_type;
//extern const TextLineSegmentType text_toggle_off_type;

/* In gtktextmark.c */
//extern const TextLineSegmentType text_left_mark_type;
//extern const TextLineSegmentType text_right_mark_type;

/* In gtktextchild.c */
//extern const TextLineSegmentType text_pixbuf_type;
//extern const TextLineSegmentType text_child_type;

/*
 * UTF 8 Stubs
 */

#define TEXT_UNKNOWN_CHAR 0xFFFC
/*VARIABLE*/ const gchar text_unknown_char_utf8[] = { '\xEF', '\xBF', '\xBC', '\0' };

//TODO put in textiter? or at least in cppconsui namespace
bool text_byte_begins_utf8_char (const gchar *byte);

G_END_DECLS

#endif

