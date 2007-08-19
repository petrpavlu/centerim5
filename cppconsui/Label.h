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

class Label
: public Widget
{
	public:
		Label(WINDOW* parentarea, int x, int y, int w, int h, Glib::ustring text);
		~Label();

		virtual void Draw(void);

	protected:
		Glib::ustring text;

	private:
		Label();

};

#endif /* __LABEL_H__ */
