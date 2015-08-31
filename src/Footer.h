// Copyright (C) 2011-2015 Petr Pavlu <setup@dagobah.cz>
//
// This file is part of CenterIM.
//
// CenterIM is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// CenterIM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef FOOTER_H
#define FOOTER_H

#include "CenterIM.h"

#include <cppconsui/Window.h>
#include <cppconsui/Label.h>

#define FOOTER (Footer::instance())

// Bottom area of the screen containing context sensitive help.
class Footer : public CppConsUI::Window {
public:
  static Footer *instance();

  // Window
  virtual void onScreenResized();

  void setText(const char *fmt, ...) _attribute((format(printf, 2, 3)));

private:
  typedef std::vector<std::string> Values;

  CppConsUI::Label *label_;
  Values values_;

  static Footer *my_instance_;

  Footer();
  virtual ~Footer() {}
  CONSUI_DISABLE_COPY(Footer);

  static void init();
  static void finalize();
  friend class CenterIM;

  void updateText();
};

#endif // FOOTER_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
