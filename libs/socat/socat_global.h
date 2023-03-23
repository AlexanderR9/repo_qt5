#ifndef SOCAT_LIB_GLOBAL_H
#define SOCAT_LIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIB_LIBRARY)
#  define LIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // SOCAT_LIB_GLOBAL_H
