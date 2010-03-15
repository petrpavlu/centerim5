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

#include "CenterMain.h"

#include "CenterIM.h"
#include "Defines.h"
#include "gettext.h"

int main(int argc, char **argv)
{
	CenterIM* cim;

	g_thread_init(NULL);

	g_set_prgname(PACKAGE_NAME);

#ifdef ENABLE_NLS
	bindtextdomain(PACKAGE_NAME, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
	textdomain(PACKAGE_NAME);
#endif

	setlocale(LC_ALL, "");

	try {
		cim = &CenterIM::Instance();
	}
	catch (int i) {
		switch (i) {
		case EXCEPTION_ICONV_INIT:
			fprintf(stderr, _("could not initialize iconv\n"));
			break;
		case EXCEPTION_PURPLE_CORE_INIT:
			fprintf(stderr, _("could not initialize libpurple core\n"));
			break;
		default:
			fprintf(stderr, _("unknown error `%d'\n"), i);
		}
		return i;
	}

	cim->Run();

	return 0;
}
