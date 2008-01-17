#ifndef __CURSES_H__
#define __CURSES_H__

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif

typedef struct curses_imp_t {
	WINDOW* w;
	curses_imp_t(WINDOW *w): w(w) {}
};

#endif /* __CURSES_H__ */
