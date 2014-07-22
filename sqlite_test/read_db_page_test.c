#include "stdafx.h"

#include "read_db_page_test.h"
#include "db_page_reader_impl.h"
#include "sqlite_mocks.h"
#include "Basic.h"


int setup_read_db_page()
{
     Btree* btree = GetMockBtree();
     sqlite3_io_methods* io_methods = GetMockIoMethods();
     sqlite3_file* fd_db = GetMockFileDescriptor( btree );
     sqlite_internal_methods* internal_methods = GetMockInternalMethods();

     fd_db->pMethods = io_methods;
     internal_methods->xGetDbFileDescriptor = GetMockFileDescriptor;

     return 0;
}

int teardown_read_db_page()
{
     Btree* btree = GetMockBtree();
     sqlite_internal_methods* internal_methods = GetMockInternalMethods();
     sqlite3_file* fd_db = GetMockFileDescriptor( btree );
     sqlite3_io_methods* io_methods = GetMockIoMethods();

     memset( internal_methods, 0, sizeof( sqlite_internal_methods ) );
     memset( fd_db, 0, sizeof( sqlite3_file ) );
     memset( io_methods, 0, sizeof( sqlite3_io_methods ) );

     return 0;
}

/// @brief
int MockMethodReadWrongSize( sqlite3_file* fd, void* buff, int iAmt, sqlite3_int64 iOfst )
{
     return SQLITE_ERROR;
}

void read_db_wrong_header_test()
{        
     db_info info;
     db_info empty;
     
     Btree* btree = GetMockBtree();
     sqlite3_file* fd_db = GetMockFileDescriptor( btree );
     sqlite3_io_methods* io_methods = GetMockIoMethods();
     io_methods->xRead = MockMethodReadWrongSize; 

     memset( &info, 0, sizeof( db_info ) );
     memset( &empty, 0, sizeof( db_info ) );

     CU_ASSERT( ReadDbHeader( fd_db, &info ) == SQLITE_ERROR );
     CU_ASSERT( memcmp( &info, &empty, sizeof( db_info ) ) == 0 );
}

void read_db_page_test()
{
     CU_pSuite read_page_suite = CU_add_suite( "read db page test suite"
          , setup_read_db_page, teardown_read_db_page );

     CU_add_test( read_page_suite
          , "read database wrong size header test", read_db_wrong_header_test );
}