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

#include "Keys.h"

#include <stdlib.h>

#include <cstring>

Keys* Keys::instance = NULL;

Keys* Keys::Instance(void)
{
	if (!instance) instance = new Keys();
	return instance;
}

Keys::Keys(void)
{
#define INSERT_KEY(k, code) { \
	specials.insert(std::make_pair(code, k)); \
}
#define REM_LENGTH  (sizeof(key) - (cur - key))
#define INSERT_COMBO(k, code) { \
	snprintf(key, sizeof(key), "%s%s%s", controls[c], alts[a], k);  \
	INSERT_KEY(key, code);  \
}

	const char *controls[] = {"", "c-", "ctrl-", "ctr-", "ctl-", NULL};
	const char *alts[] = {"", "alt-", "a-", "m-", "meta-", NULL};
	int c, a, ch;
	char key[32];

	term = getenv("TERM");
	if (!term)
		term = "";  /* Just in case */

	/* The default values here are based on xterm/rxvt */
	key_ctl_up = "\033" "[1;5A";
	key_ctl_down = "\033" "[1;5B";
	key_ctl_right = "\033" "[1;5C";
	key_ctl_left = "\033" "[1;5D";

	if (strncmp(term, "screen", strlen("screen")) == 0 || strcmp(term, "rxvt-unicode") == 0) {
		/* The check for screen includes other screen variants (eg: screen-bce)*/
		key_ctl_up = "\033" "Oa";
		key_ctl_down = "\033" "Ob";
		key_ctl_right = "\033" "Oc";
		key_ctl_left = "\033" "Od";
	}
	//TODO add more terms
	//(tip use `cat|xxd' to see hex values of the keys)
	//TODO where are the alt keys? add those too

	INSERT_KEY("home", Key_home());
	INSERT_KEY("end", Key_end());
	INSERT_KEY("pageup", Key_pgup());
	INSERT_KEY("pagedown", Key_pgdown());
	INSERT_KEY("insert", Key_ins());
	INSERT_KEY("delete", Key_del());
	INSERT_KEY("esc", Key_esc());

	INSERT_KEY("left", Key_left());
	INSERT_KEY("right", Key_right());
	INSERT_KEY("up", Key_up());
	INSERT_KEY("down", Key_down());

	INSERT_KEY("tab", Key_tab());
	INSERT_KEY("menu", Key_popup());

	INSERT_KEY("f1", Key_f1());
	INSERT_KEY("f2", Key_f2());
	INSERT_KEY("f3", Key_f3());
	INSERT_KEY("f4", Key_f4());
	INSERT_KEY("f5", Key_f5());
	INSERT_KEY("f6", Key_f6());
	INSERT_KEY("f7", Key_f7());
	INSERT_KEY("f8", Key_f8());
	INSERT_KEY("f9", Key_f9());
	INSERT_KEY("f10", Key_f10());
	INSERT_KEY("f11", Key_f11());
	INSERT_KEY("f12", Key_f12());

	/* Lower-case alphabets */
	for (a = 0, c = 0; controls[c]; c++, a = 0) {
		if (c) {
			INSERT_COMBO("up", key_ctl_up);
			INSERT_COMBO("down", key_ctl_down);
			INSERT_COMBO("left", key_ctl_left);
			INSERT_COMBO("right", key_ctl_right);
		}

		for (a = 0; alts[a]; a++) {
			for (ch = 0; ch < 26; ch++) {
				char str[2] = {'a' + ch, 0}, code[5] = "\0\0\0\0";
				int ind = 0;
				if (a)
					code[ind++] = '\033';
				code[ind] = (c ? 1 : 'a') + ch;
				INSERT_COMBO(str, code);
			}
		}
	}

	for (a = 0; alts[a]; a++) {
		/* Upper-case alphabets */
		for (ch = 0; ch < 26; ch++) {
			char str[2] = {'A' + ch, 0}, code[] = {'\033', 'A' + ch, 0};
			INSERT_COMBO(str, code);
		}
		/* Digits */
		for (ch = 0; ch < 10; ch++) {
			char str[2] = {'0' + ch, 0}, code[] = {'\033', '0' + ch, 0};
			INSERT_COMBO(str, code);
		}
	}
#undef REM_LENGTH
#undef INSERT_COMBO
#undef INSERT_KEY
}

Keys::~Keys(void)
{
	/* this is done by the window manager
	if (endwin() == ERR)
		;//TODO throw an exeption
	*/
}

void Keys::Delete(void)
{
	if (instance) {
		delete instance;
		instance = NULL;
	}
}

int Keys::Compare(const char *keys1, const char *keys2)
{
	return strncmp(keys1, keys2, strlen(keys1));
}

const Glib::ustring Keys::Name(const char *key)
{
	std::map<Glib::ustring, Glib::ustring>::iterator i;

	i = specials.find(key);

	if (i != specials.end())
		return i->second;
	else
		return key;
}

void Keys::Refine(char *str, int bytes)
{
	int i = 0;

	while (i < bytes) {
		if (str[i] == 27 && str[i+1] == '[' && str[i+2] >= 'A' && str[i+2] <= 'D') {
			/* Apparently this is necessary for urxvt and screen and xterm */
			if (strncmp(term, "screen", strlen("screen")) == 0
					|| strcmp(term, "rxvt-unicode") == 0
					|| strcmp(term, "xterm") == 0) {
				str[i+1] = 'O';
			}

			i += 3;
		} else if ((unsigned char)str[0] == 195) {
			if (str[i+2] == 0 && strcmp(term, "xterm") == 0) {
				str[i] = 27;
				str[i+1] -= 64; /* Say wha? */

				i += 3;
			}
		}
		i++;
	}
}
