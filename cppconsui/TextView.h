/* GTK - The GIMP Toolkit
 * gtktextview.h Copyright (C) 2000 Red Hat, Inc.
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

//#if defined(GTK_DISABLE_SINGLE_INCLUDES) && !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
//#error "Only <gtk/gtk.h> can be included directly."
//#endif

#ifndef __TEXTVIEW_H__
#define __TEXTVIEW_H__

#include "Widget.h"
#include "TextBuffer.h"

class TextView
: public Widget
{
	public:
		enum Justification {LEFT, RIGHT, CENTER, FILL};

		TextView(Widget& parent, int x, int y, int w, int h, TextBuffer *buffer);
		TextView(Widget& parent, int x, int y, int w, int h);
		~TextView(void);

	protected:

	private:

};

#endif /* __TEXTVIEW_H__ */
