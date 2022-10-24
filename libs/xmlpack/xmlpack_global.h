#ifndef XMLPACK_LIB_GLOBAL_H
#define XMLPACK_LIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIB_LIBRARY)
#  define LIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // XMLPACK_LIB_GLOBAL_H
