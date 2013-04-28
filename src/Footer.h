/*
 * Copyright (C) 2011-2013 by CenterIM developers
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

#ifndef __FOOTER_H__
#define __FOOTER_H__

#include "CenterIM.h"

#include <cppconsui/FreeWindow.h>
#include <cppconsui/Label.h>

#define FOOTER (Footer::instance())

// the bottom area of the screen containing context sensitive help
class Footer
: public CppConsUI::FreeWindow
{
public:
  static Footer *instance();

  // FreeWindow
  virtual void onScreenResized();

  void setText(const char *fmt, ...) _attribute((format(printf, 2, 3)));

protected:

private:
  typedef std::vector<std::string> Values;

  CppConsUI::Label *label;
  Values values;

  static Footer *my_instance;

  Footer();
  Footer(const Footer&);
  Footer &operator=(const Footer&);
  virtual ~Footer() {}

  static void init();
  static void finalize();
  friend class CenterIM;

  void updateText();
};

#endif // __FOOTER_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
