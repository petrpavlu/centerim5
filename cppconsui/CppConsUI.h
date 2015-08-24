/*
 * Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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

/**
 * @file
 * General classes, functions and enumerations.
 *
 * @ingroup cppconsui
 */

#ifndef __CPPCONSUI_H__
#define __CPPCONSUI_H__

#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <cstdlib>

#define COLORSCHEME (CppConsUI::getColorSchemeInstance())
#define COREMANAGER (CppConsUI::getCoreManagerInstance())
#define KEYCONFIG (CppConsUI::getKeyConfigInstance())

#define CONSUI_DISABLE_COPY(Class)                                             \
  Class(const Class &);                                                        \
  Class &operator=(const Class &)

#define PRINTF_WIDTH(type) ((CHAR_BIT * sizeof(type) + 2) / 3 + 1)

#ifdef __GNUC__
#define CPPCONSUI_COMPILER_ATTRIBUTE(x) __attribute__(x)
#else
#define CPPCONSUI_COMPILER_ATTRIBUTE(x)
#endif

namespace CppConsUI {

enum ErrorCode {
  ERROR_NONE,

  ERROR_LIBTERMKEY_INITIALIZATION,
  ERROR_ICONV_INITIALIZATION,
  ERROR_SCREEN_RESIZING_INITIALIZATION,
  ERROR_SCREEN_RESIZING_FINALIZATION,

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

  bool present() const { return error_code != ERROR_NONE; }

  void setCode(ErrorCode code);
  ErrorCode getCode() const { return error_code; }

  void setString(const char *string);
  void setFormattedString(const char *format, ...)
    CPPCONSUI_COMPILER_ATTRIBUTE((format(printf, 2, 3)));
  const char *getString() const { return error_string; }

  void clear();

protected:
  ErrorCode error_code;
  char *error_string;
};

enum WrapMode {
  WRAP_NONE,
  WRAP_CHAR,
  WRAP_WORD,
};

class Point {
public:
  Point() : x(0), y(0) {}
  Point(int x_, int y_) : x(x_), y(y_) {}

  int getX() const { return x; }
  int getY() const { return y; }

  int x, y;
};

class Size {
public:
  Size() : width(0), height(0) {}
  Size(int w, int h) : width(w), height(h) {}

  int getWidth() const { return width; }
  int getHeight() const { return height; }

  int width, height;
};

class Rect : public Point {
public:
  Rect() : width(0), height(0) {}
  Rect(int x, int y, int w, int h) : Point(x, y), width(w), height(h) {}

  int getWidth() const { return width; }
  int getHeight() const { return height; }
  int getLeft() const { return x; }
  int getTop() const { return y; }
  int getRight() const { return x + width - 1; }
  int getBottom() const { return y + height - 1; }

  int width, height;
};

enum InputCondition {
  INPUT_CONDITION_READ = 1 << 0,
  INPUT_CONDITION_WRITE = 1 << 1,
};

typedef bool (*SourceFunction)(void *data);
typedef void (*InputFunction)(int fd, InputCondition cond, void *data);

struct AppInterface {
  unsigned (*timeoutAdd)(unsigned interval, SourceFunction func, void *data);
  bool (*timeoutRemove)(unsigned handle);
  unsigned (*inputAdd)(
    int fd, InputCondition cond, InputFunction func, void *data);
  bool (*inputRemove)(unsigned handle);
  void (*logError)(const char *message);
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

#endif // __CPPCONSUI_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
