/*
 * Copyright (C) 2011 by CenterIM developers
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

#ifndef __FOOTER_H__
#define __FOOTER_H__

#include <cppconsui/FreeWindow.h>
#include <cppconsui/Label.h>

#define FOOTER (Footer::Instance())

// the bottom area of the screen containing context sensitive help
class Footer
: public FreeWindow
{
public:
  static Footer *Instance();

  // FreeWindow
  virtual void ScreenResized();

  void SetText(const char *text);
  const char *GetText() const;

protected:

private:
  Label *label;

  static Footer *instance;

  Footer();
  Footer(const Footer&);
  Footer &operator=(const Footer&);
  virtual ~Footer() {}

  static void Init();
  static void Finalize();
  friend class CenterIM;
};

#endif // __FOOTER_H__
