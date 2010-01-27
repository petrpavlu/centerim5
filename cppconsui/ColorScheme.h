/*
 * Copyright (C) 2008 by Mark Pustjens <pustjens@dds.nl>
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

#ifndef __COLORSCHEME_H__
#define __COLORSCHEME_H__

#include "ConsuiCurses.h"

class ColorScheme
{
	public:
		enum ColorType {Normal = 0, Focus, Disabled};

		ColorScheme(void);
		ColorScheme(const ColorScheme *colorscheme);
		~ColorScheme();

		#define ColorSchemeDefault ColorSchemeNormal
		static ColorScheme* ColorSchemeNormal(void);

		void On(Curses::Window *area, const ColorType type);
		void Off(Curses::Window *area, const ColorType type);

		void SetColor(Curses::Window *area, const int x, const int y, const int n, const ColorType type);

		void SetColorType(ColorType type, int scheme);
		void GetColorType(ColorType type);

	protected:
		int schemes[Disabled+1];

	private:
		ColorScheme(const ColorScheme&);

		ColorScheme& operator=(ColorScheme&);
};

#endif /* __COLORSCHEME_H__ */
