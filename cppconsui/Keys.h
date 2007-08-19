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

#ifndef __KEYS_H__
#define __KEYS_H__

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <term.h>
#include <glibmm/ustring.h>
#include <map>

/* thank you gntkeys.h :)
 * a lot of the stuff below has been copied
 * with minor changes
 */
#define SAFE(x)   ((x) ? (x) : "")

#define CUI_KEY_POPUP   SAFE(key_f16)   /* Apparently */

/* Arrow keys */
#define CUI_KEY_LEFT   SAFE(key_left)
#define CUI_KEY_RIGHT  SAFE(key_right)
#define CUI_KEY_UP     SAFE(key_up)
#define CUI_KEY_DOWN   SAFE(key_down)

#define CUI_KEY_CTRL_UP     SAFE(gnt_key_cup)
#define CUI_KEY_CTRL_DOWN   SAFE(gnt_key_cdown)
#define CUI_KEY_CTRL_RIGHT  SAFE(gnt_key_cright)
#define CUI_KEY_CTRL_LEFT   SAFE(gnt_key_cleft)

#define CUI_KEY_PGUP   SAFE(key_ppage)
#define CUI_KEY_PGDOWN SAFE(key_npage)
#define CUI_KEY_HOME   SAFE(key_home)
#define CUI_KEY_END    SAFE(key_end)

#define CUI_KEY_ENTER  carriage_return

#define CUI_KEY_BACKSPACE SAFE(key_backspace)
#define CUI_KEY_DEL    SAFE(key_dc)
#define CUI_KEY_INS    SAFE(key_ic)

#define CUI_KEY_CTRL_A     "\001"
#define CUI_KEY_CTRL_B     "\002"
#define CUI_KEY_CTRL_C     "\003" /* why doesn't libgnt have this one? */
#define CUI_KEY_CTRL_D     "\004"
#define CUI_KEY_CTRL_E     "\005"
#define CUI_KEY_CTRL_F     "\006"
#define CUI_KEY_CTRL_G     "\007"
#define CUI_KEY_CTRL_H     "\010"
#define CUI_KEY_CTRL_I     "\011"
#define CUI_KEY_CTRL_J     "\012"
#define CUI_KEY_CTRL_K     "\013"
#define CUI_KEY_CTRL_L     "\014"
#define CUI_KEY_CTRL_M     "\012"
#define CUI_KEY_CTRL_N     "\016"
#define CUI_KEY_CTRL_O     "\017"
#define CUI_KEY_CTRL_P     "\020"
#define CUI_KEY_CTRL_R     "\022"
#define CUI_KEY_CTRL_T     "\024"
#define CUI_KEY_CTRL_U     "\025"
#define CUI_KEY_CTRL_V     "\026"
#define CUI_KEY_CTRL_W     "\027"
#define CUI_KEY_CTRL_X     "\030"
#define CUI_KEY_CTRL_Y     "\031"

#define CUI_KEY_F1         SAFE(key_f1)
#define CUI_KEY_F2         SAFE(key_f2)
#define CUI_KEY_F3         SAFE(key_f3)
#define CUI_KEY_F4         SAFE(key_f4)
#define CUI_KEY_F5         SAFE(key_f5)
#define CUI_KEY_F6         SAFE(key_f6)
#define CUI_KEY_F7         SAFE(key_f7)
#define CUI_KEY_F8         SAFE(key_f8)
#define CUI_KEY_F9         SAFE(key_f9)
#define CUI_KEY_F10        SAFE(key_f10)
#define CUI_KEY_F11        SAFE(key_f11)
#define CUI_KEY_F12        SAFE(key_f12)

class Keys
{
	public:
                ~Keys(void);

                static Keys* Instance(void);
                void Delete(void);

		static int Compare(const char *keys1, const char *keys2);
		const Glib::ustring Name(const char *key);

	protected:

	private:
		Keys(void);

                static Keys *instance;

		/* copied from gntkeys.h */
		/**
		 * terminfo/termcap doesn't provide all the information that I want to use, eg.
		 * ctrl-up, ctrl-down etc. So I am going to hard-code some of the information
		 * for some popular $TERMs
		 */
		//TODO make sure the above is accurate, if there
		//is a better way, use that
		char *key_ctl_up;
		char *key_ctl_down;
		char *key_ctl_left;
		char *key_ctl_right;

		std::map<Glib::ustring, Glib::ustring> specials;
};

#endif /* __KEYS_H__ */
