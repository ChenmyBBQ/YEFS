#pragma once
#include <QtCore/QtGlobal>

#if defined(DBCOMPT_LIBRARY)
#  define DBCOMPT_EXPORT Q_DECL_EXPORT
#else
#  define DBCOMPT_EXPORT Q_DECL_IMPORT
#endif
