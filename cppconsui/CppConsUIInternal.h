#ifndef __CPPCONSUIINTERNAL_H__
#define __CPPCONSUIINTERNAL_H__

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(s)	gettext(s)
#else
#define _(s)	(s)
#endif // ENABLE_NLS

#endif /* __CPPCONSUIINTERNAL_H__ */
