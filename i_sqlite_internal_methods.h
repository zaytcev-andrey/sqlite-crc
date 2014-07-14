#ifndef i_sqlite_internal_h__
#define i_sqlite_internal_h__

#include "sqlite/sqlite3.h"

typedef struct Btree Btree;

/// @note struct sqlite3_file defined in sqlite3.h

typedef sqlite3_file* (*PFGetDbFileDescriptor)( Btree* btree );

typedef struct SQLITE_INTERNAL_METHODS
{
     PFGetDbFileDescriptor xGetDbFileDescriptor;
} sqlite_internal_methods;

void InitializeInternalMethods( sqlite_internal_methods* internal
     , PFGetDbFileDescriptor xGetDbFileDescriptor );

#endif // i_sqlite_internal_h__