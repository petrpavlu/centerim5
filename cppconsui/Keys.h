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

#define KEYS Keys::Instance()

class Keys
{
	public:
		static Keys* Instance(void);
		void Delete(void);

		static int Compare(const char *keys1, const char *keys2);
		const Glib::ustring Name(const char *key);
		void Refine(char *str, int bytes);

		/* thank you gntkeys.h :)
		 * a lot of the stuff below has been copied
		 * with a lot of changes (yay)
		 */
		#define SAFE(x) ((x) ? (x) : "")

		//TODO find names for these const key definitions which
		//dont clash with the ncurses names
		const char* Key_popup(void) { return SAFE(key_f16); }
		const char* Key_tab(void) { return "\t"; }

		/* arrow Keys */
		const char* Key_left(void) { return SAFE(key_left); }
		const char* Key_right(void) { return SAFE(key_right); }
		const char* Key_up(void) { return SAFE(key_up); }
		const char* Key_down(void) { return SAFE(key_down); }

		const char* Key_ctrl_up(void) { return SAFE(key_ctl_up); }
		const char* Key_ctrl_down(void) { return SAFE(key_ctl_down); }
		const char* Key_ctrl_right(void) { return SAFE(key_ctl_right); }
		const char* Key_ctrl_left(void) { return SAFE(key_ctl_left); }

		const char* Key_pgup(void) { return SAFE(key_ppage); }
		const char* Key_pgdown(void) { return SAFE(key_npage); }
		const char* Key_home(void) { return SAFE(key_home); }
		const char* Key_end(void) { return SAFE(key_end); }

		const char* Key_enter(void) { return carriage_return; }
		const char* Key_backspace(void) { return SAFE(key_backspace); }
		const char* Key_del(void) { return SAFE(key_dc); }
		const char* Key_ins(void) { return SAFE(key_ic); }
		const char* Key_esc(void) { return "\033"; }
		const char* Key_form_feed(void) { return SAFE(form_feed); }

		const char* Key_ctrl_a(void) { return "\001"; }
		const char* Key_ctrl_b(void) { return "\002"; }
		const char* Key_ctrl_c(void) { return "\003"; }
		const char* Key_ctrl_d(void) { return "\004"; }
		const char* Key_ctrl_e(void) { return "\005"; }
		const char* Key_ctrl_f(void) { return "\006"; }
		const char* Key_ctrl_g(void) { return "\007"; }
		const char* Key_ctrl_h(void) { return "\010"; }
		const char* Key_ctrl_i(void) { return "\011"; }
		const char* Key_ctrl_j(void) { return "\012"; }
		const char* Key_ctrl_k(void) { return "\013"; }
		const char* Key_ctrl_l(void) { return "\014"; }
		const char* Key_ctrl_m(void) { return "\012"; }
		const char* Key_ctrl_n(void) { return "\016"; }
		const char* Key_ctrl_o(void) { return "\017"; }
		const char* Key_ctrl_p(void) { return "\020"; }
		const char* Key_ctrl_q(void) { return "\021"; }
		const char* Key_ctrl_r(void) { return "\022"; }
		const char* Key_ctrl_s(void) { return "\023"; }
		const char* Key_ctrl_t(void) { return "\024"; }
		const char* Key_ctrl_u(void) { return "\025"; }
		const char* Key_ctrl_v(void) { return "\026"; }
		const char* Key_ctrl_w(void) { return "\027"; }
		const char* Key_ctrl_x(void) { return "\030"; }
		const char* Key_ctrl_y(void) { return "\031"; }
		const char* Key_ctrl_z(void) { return "\032"; }

		const char* Key_shift_tab(void) { return "\033\133\132"; }

		const char* Key_f1(void) { return SAFE(key_f1); }
		const char* Key_f2(void) { return SAFE(key_f2); }
		const char* Key_f3(void) { return SAFE(key_f3); }
		const char* Key_f4(void) { return SAFE(key_f4); }
		const char* Key_f5(void) { return SAFE(key_f5); }
		const char* Key_f6(void) { return SAFE(key_f6); }
		const char* Key_f7(void) { return SAFE(key_f7); }
		const char* Key_f8(void) { return SAFE(key_f8); }
		const char* Key_f9(void) { return SAFE(key_f9); }
		const char* Key_f10(void) { return SAFE(key_f10); }
		const char* Key_f11(void) { return SAFE(key_f11); }
		const char* Key_f12(void) { return SAFE(key_f12); }

	protected:

	private:
		Keys(void);
		~Keys(void);

		static Keys *instance;

		Keys& operator=(const Keys&);

		/* copied from gntkeys.h */
		/**
		 * terminfo/termcap doesn't provide all the information that I want to use, eg.
		 * ctrl-up, ctrl-down etc. So I am going to hard-code some of the information
		 * for some popular $TERMs
		 */
		//TODO make sure the above is accurate, if there
		//is a better way, use that
		const char *key_ctl_up;
		const char *key_ctl_down;
		const char *key_ctl_left;
		const char *key_ctl_right;

		std::map<Glib::ustring, Glib::ustring> specials;

		const char *term;
};

/* Bad term.h! */
#undef buttons
#undef columns
#undef newline

#endif /* __KEYS_H__ */
