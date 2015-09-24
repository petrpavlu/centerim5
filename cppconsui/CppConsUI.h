// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
// Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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

/// @file
/// General classes, functions and enumerations.
///
/// @ingroup cppconsui

#ifndef CPPCONSUI_H
#define CPPCONSUI_H

#include <sigc++/sigc++.h>

#include <cstdlib>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>

#define COLORSCHEME (CppConsUI::getColorSchemeInstance())
#define COREMANAGER (CppConsUI::getCoreManagerInstance())
#define KEYCONFIG (CppConsUI::getKeyConfigInstance())

#define CONSUI_DISABLE_COPY(Class)                                             \
  Class(const Class &);                                                        \
  Class &operator=(const Class &)

#define PRINTF_WIDTH(type) ((CHAR_BIT * sizeof(type) + 2) / 3 + 1)

#ifdef __GNUC__
#define CPPCONSUI_GNUC_ATTRIBUTE(x) __attribute__(x)
#else
#define CPPCONSUI_GNUC_ATTRIBUTE(x)
#endif

namespace CppConsUI {

enum ErrorCode {
  ERROR_NONE,

  ERROR_LIBTERMKEY_INITIALIZATION,
  ERROR_ICONV_INITIALIZATION,
  ERROR_SCREEN_RESIZING_INITIALIZATION,
  ERROR_SCREEN_RESIZING_FINALIZATION,

  ERROR_INPUT_CONVERSION,

  ERROR_CURSES_INITIALIZATION,
  ERROR_CURSES_FINALIZATION,
  ERROR_CURSES_ADD_CHARACTER,
  ERROR_CURSES_ATTR,
  ERROR_CURSES_COLOR_LIMIT,
  ERROR_CURSES_COLOR_INIT,
  ERROR_CURSES_CLEAR,
  ERROR_CURSES_REFRESH,
  ERROR_CURSES_BEEP,
  ERROR_CURSES_RESIZE,
};

class Error {
public:
  explicit Error(ErrorCode code = ERROR_NONE, const char *string = NULL);
  Error(const Error &other);
  Error &operator=(const Error &other);
  virtual ~Error();

  bool present() const { return error_code_ != ERROR_NONE; }

  void setCode(ErrorCode code);
  ErrorCode getCode() const { return error_code_; }

  void setString(const char *string);
  void setFormattedString(const char *format, ...)
    CPPCONSUI_GNUC_ATTRIBUTE((format(printf, 2, 3)));
  const char *getString() const { return error_string_; }

  void clear();

protected:
  ErrorCode error_code_;
  char *error_string_;
};

enum WrapMode {
  WRAP_NONE,
  WRAP_CHAR,
  WRAP_WORD,
};

struct Point {
  Point() : x(0), y(0) {}
  Point(int x, int y) : x(x), y(y) {}

  int getX() const { return x; }
  int getY() const { return y; }

  int x, y;
};

struct Size {
  Size() : width(0), height(0) {}
  Size(int w, int h) : width(w), height(h) {}

  int getWidth() const { return width; }
  int getHeight() const { return height; }

  int width, height;
};

struct Rect : public Point, public Size {
  Rect() {}
  Rect(int x, int y, int w, int h) : Point(x, y), Size(w, h) {}

  int getLeft() const { return x; }
  int getTop() const { return y; }
  int getRight() const { return x + width - 1; }
  int getBottom() const { return y + height - 1; }
};

struct AppInterface {
  sigc::slot<void> redraw;
  sigc::slot<void, const char *> logDebug;
};

void initializeConsUI(AppInterface &interface);
void finalizeConsUI();

class ColorScheme;
class CoreManager;
class KeyConfig;

ColorScheme *getColorSchemeInstance();
CoreManager *getCoreManagerInstance();
KeyConfig *getKeyConfigInstance();

namespace UTF8 {

typedef uint32_t UniChar;
#define UNICHAR_FORMAT PRIu32
UniChar getUniChar(const char *p);
bool isUniCharWide(UniChar uc);
bool isUniCharDigit(UniChar uc);
bool isUniCharSpace(UniChar uc);
const char *getNextChar(const char *p);
const char *getPrevChar(const char *p);
const char *findNextChar(const char *p, const char *end);
const char *findPrevChar(const char *start, const char *p);

} // namespace UTF8

} // namespace CppConsUI

#endif // CPPCONSUI_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
