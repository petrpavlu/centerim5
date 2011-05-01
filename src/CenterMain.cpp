/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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

#include "CenterIM.h"
#include "gettext.h"
#include <locale.h>

#define CIM_CONFIG_PATH ".centerim5"

void print_usage(FILE *out, const char *prg_name)
{
  fprintf(out, _("Usage: %s [-c config_path]\n"), prg_name);
}

int main(int argc, char *argv[])
{
  g_set_prgname(PACKAGE_NAME);

#ifdef ENABLE_NLS
  bindtextdomain(PACKAGE_NAME, LOCALEDIR);
  bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
  bind_textdomain_codeset("pidgin", "UTF-8");
  textdomain(PACKAGE_NAME);
#endif

  setlocale(LC_ALL, "");

  // parse args
  const char *config_path = CIM_CONFIG_PATH;
  int opt;
  while ((opt = getopt(argc, argv, "hc:")) != -1) {
    switch (opt) {
      case 'c':
        config_path = optarg;
        break;
      case 'h':
        print_usage(stdout, argv[0]);
        return 0;
      default:
        print_usage(stderr, argv[0]);
        return 1;
    }
  }
  if (optind < argc) {
    fprintf(stderr, _("%s: unexpected argument after options\n"), argv[0]);
    print_usage(stderr, argv[0]);
    return 1;
  }

  CenterIM *cim = CenterIM::Instance();
  return cim->Run(config_path);
}
