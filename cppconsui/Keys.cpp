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
	const char *term;
	int c, a, ch;
	char key[32];

	term = getenv("TERM");
	if (!term)
		term = "";  /* Just in case */

	if (strcmp(term, "xterm") == 0 || strcmp(term, "rxvt") == 0) {
		key_ctl_up = "\033" "[1;5A";
		key_ctl_down = "\033" "[1;5B";
		key_ctl_right = "\033" "[1;5C";
		key_ctl_left = "\033" "[1;5D";
	} else if (strcmp(term, "screen") == 0 || strcmp(term, "rxvt-unicode") == 0) {
		key_ctl_up = "\033" "Oa";
		key_ctl_down = "\033" "Ob";
		key_ctl_right = "\033" "Oc";
		key_ctl_left = "\033" "Od";
	}
	//TODO add more terms, and most importantly: defaults!
	//(tip use `cat|xxd' to see hex values of the keys)
	//TODO where are the alt keys? add those too

	INSERT_KEY("home", CUI_KEY_HOME);
	INSERT_KEY("end", CUI_KEY_END);
	INSERT_KEY("pageup", CUI_KEY_PGUP);
	INSERT_KEY("pagedown", CUI_KEY_PGDOWN);
	INSERT_KEY("insert", CUI_KEY_INS);
	INSERT_KEY("delete", CUI_KEY_DEL);

	INSERT_KEY("left", CUI_KEY_LEFT);
	INSERT_KEY("right", CUI_KEY_RIGHT);
	INSERT_KEY("up", CUI_KEY_UP);
	INSERT_KEY("down", CUI_KEY_DOWN);

	INSERT_KEY("tab", "\t");
	INSERT_KEY("menu", CUI_KEY_POPUP);

	INSERT_KEY("f1", CUI_KEY_F1);
	INSERT_KEY("f2", CUI_KEY_F2);
	INSERT_KEY("f3", CUI_KEY_F3);
	INSERT_KEY("f4", CUI_KEY_F4);
	INSERT_KEY("f5", CUI_KEY_F5);
	INSERT_KEY("f6", CUI_KEY_F6);
	INSERT_KEY("f7", CUI_KEY_F7);
	INSERT_KEY("f8", CUI_KEY_F8);
	INSERT_KEY("f9", CUI_KEY_F9);
	INSERT_KEY("f10", CUI_KEY_F10);
	INSERT_KEY("f11", CUI_KEY_F11);
	INSERT_KEY("f12", CUI_KEY_F12);

	/* Lower-case alphabets */
	for (c = 0; controls[c]; c++) {
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
	if (endwin() == ERR)
		;//TODO throw an exeption
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
