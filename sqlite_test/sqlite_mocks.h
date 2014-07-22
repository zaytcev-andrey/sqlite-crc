#ifndef sqlite_mocks_h__
#define sqlite_mocks_h__

#include "sqlite/sqlite3.h"
#include "i_sqlite_internal_methods.h"

typedef struct Btree Btree;
struct Btree { int dumy; };

/// @group Methods that returns mock objects
/// {
Btree* GetMockBtree();

sqlite_internal_methods* GetMockInternalMethods();

sqlite3_file* GetMockFileDescriptor( Btree* btree );

sqlite3_io_methods* GetMockIoMethods();

int GetMockFileSize( sqlite3_file* fd, sqlite3_int64 *pSize );
void SetMockFileSize( sqlite3_int64 size );
/// }

#endif // sqlite_mocks_h__