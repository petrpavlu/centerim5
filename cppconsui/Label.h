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

#ifndef __LABEL_H__
#define __LABEL_H__

#include "Widget.h"

#include <glibmm/ustring.h>

/* Initial size of buffer, in bytes */
#define MIN_SIZE 16

/* Maximum size of text buffer, in bytes */
#define MAX_SIZE G_MAXUSHORT

//TODO proper autosizing

class Label
: public Widget
{
	public:
		Label(Widget& parent, int x, int y, int w, int h, const gchar *fmt, ...);
		Label(Widget& parent, int x, int y, const char *fmt, ...);
		virtual ~Label();

		virtual void Draw(void);

		void SetText(const gchar *str);
		const gchar* GetText(void);

	protected:
		gchar *text; 
		guint16 text_length;  /* length in use, in chars */
		guint16 text_max_length;

		guint16 text_size;    /* allocated size, in bytes */
		guint16 n_bytes;      /* length in use, in bytes */

	private:
		Label(void);
		Label(const Label&);

		Label& operator=(const Label&);

};

#endif /* __LABEL_H__ */
