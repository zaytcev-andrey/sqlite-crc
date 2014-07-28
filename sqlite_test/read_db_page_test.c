#include "stdafx.h"

#include "read_db_page_test.h"
#include "db_page_reader_impl.h"
#include "sqlite_mocks.h"
#include "Basic.h"

/// @brief
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

/// @brief
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

/// @brief
int MockMethodReadHeader( sqlite3_file* fd, void* buff, int iAmt, sqlite3_int64 iOfst )
{
     static const char header[] = 
     { 0x53, 0x51, 0x4C, 0x69, 0x74, 0x65, 0x20, 0x66, 0x6F, 0x72, 0x6D, 0x61, 0x74, 0x20, 0x33, 0x00, 
     0x04, 0x00, 0x01, 0x01, 0x10, 0x40, 0x20, 0x20, 0x00, 0x01, 0x87, 0xC6, 0x00, 0x00, 0x13, 0x93, 
     0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x87, 0xC6, 
     0x00, 0x2D, 0xE6, 0x00 };
     
     const int header_len = sizeof( header );     
     CU_ASSERT( header_len == 100 );

     // test passing params
     CU_ASSERT( iAmt == 100 );
     CU_ASSERT( iOfst == 0 );
     
     // copy db header in buff
     memcpy( buff, header, header_len );
     
     return SQLITE_OK;
}

/// @brief
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

/// @brief
void read_db_header_test()
{        
     db_info info;

     // setup
     Btree* btree = GetMockBtree();
     sqlite3_file* fd_db = GetMockFileDescriptor( btree );
     sqlite3_io_methods* io_methods = GetMockIoMethods();
     io_methods->xRead = MockMethodReadHeader; 

     memset( &info, 0, sizeof( db_info ) );

     // test method
     CU_ASSERT( ReadDbHeader( fd_db, &info ) == SQLITE_OK );

     // compare result
     CU_ASSERT( info.current_page_number == 1 );
     CU_ASSERT( info.freelist_trunk_page_number == 5 );
     CU_ASSERT( info.freelist_trunk_page_offset == 4096 );
     CU_ASSERT( info.total_number_of_freelist_pages == 12 );
     CU_ASSERT( info.current_page_offset == 0 );
     CU_ASSERT( info.page_header_offset == 100 );
     CU_ASSERT( info.page_size == 1024 );
     CU_ASSERT( info.page_header_offset == 100 );
}

// 
void read_db_page_test()
{
     CU_pSuite read_page_suite = CU_add_suite( "read db page test suite"
          , setup_read_db_page, teardown_read_db_page );

     CU_add_test( read_page_suite
          , "read database wrong size header test", read_db_wrong_header_test );

     CU_add_test( read_page_suite
          , "read database correct header test", read_db_header_test );
}