#ifndef IOBUSLIB_GLOBAL_H
#define IOBUSLIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIB_LIBRARY)
#  define LIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // IOBUSLIB_GLOBAL_H
