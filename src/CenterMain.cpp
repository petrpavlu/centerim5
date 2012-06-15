/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2012 by CenterIM developers
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
 */

#include "CenterIM.h"
#include "gettext.h"
#include <getopt.h>
#include <locale.h>

#define CIM_CONFIG_PATH ".centerim5"

static void print_usage(FILE *out, const char *prg_name)
{
  fprintf(out, _(
"Usage: %s [option]...\n\n"
"Options:\n"
"  -a, --ascii                use ASCII characters to draw lines and boxes\n"
"  -h, --help                 display command line usage\n"
"  -v, --version              show the program version info\n"
"  -b, --basedir <directory>  specify another base directory\n"
"  -o, --offline              start with all protocols set offline\n"),
      prg_name);
}

static void print_version(FILE *out)
{
  fprintf(out, "CenterIM %s\n", PACKAGE_VERSION);
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

  signal(SIGPIPE, SIG_IGN);

  // parse args
  bool ascii = false;
  bool offline = false;
  const char *config_path = CIM_CONFIG_PATH;
  int opt;
  struct option long_options[] = {
    {"ascii",   no_argument,       NULL, 'a'},
    {"help",    no_argument,       NULL, 'h'},
    {"version", no_argument,       NULL, 'v'},
    {"basedir", required_argument, NULL, 'b'},
    {"offline", no_argument,       NULL, 'o'},
    {NULL,      0,                 NULL,  0 }
  };
  while ((opt = getopt_long(argc, argv, "ahvb:o", long_options, NULL))
      != -1) {
    switch (opt) {
      case 'a':
        ascii = true;
        break;
      case 'h':
        print_usage(stdout, argv[0]);
        return 0;
      case 'v':
        print_version(stdout);
        return 0;
      case 'b':
        config_path = optarg;
        break;
      case 'o':
        offline = true;
        break;
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
  return cim->Run(config_path, ascii, offline);
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
