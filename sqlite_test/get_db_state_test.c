#include "stdafx.h"

#include "get_db_state_test.h"
#include "db_state.h"
#include "sqlite_mocks.h"
#include "Basic.h"

int setup_get_db_state()
{
     Btree* btree = GetMockBtree();
     sqlite3_io_methods* io_methods = GetMockIoMethods();
     sqlite3_file* fd_db = GetMockFileDescriptor( btree );
     sqlite_internal_methods* internal_methods = GetMockInternalMethods();

     io_methods->xFileSize = GetMockFileSize;
     fd_db->pMethods = io_methods;
     internal_methods->xGetDbFileDescriptor = GetMockFileDescriptor;

     return 0;
}

int teardown_db_state()
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

/// @brief closed database test
void get_db_closed_state_test()
{
     db_open_state open_state = DB_OPENED_EXISTING;

     // closed database
     Btree* btree = GetMockBtree();
     sqlite3_file* fd_db = GetMockFileDescriptor( btree );
     fd_db->pMethods = 0; 

     GetDbOpenState( btree
          , &open_state
          , GetMockInternalMethods() );

     CU_ASSERT( open_state == DB_CLOSED );
}

/// @brief created database test
void get_db_open_creating_state_test()
{
     db_open_state open_state = DB_CLOSED; 

     // just opened database, db file does not exist
     Btree* btree = GetMockBtree();
     sqlite3_file* fd_db = GetMockFileDescriptor( btree );
     sqlite3_io_methods* io_methods = GetMockIoMethods();
     fd_db->pMethods = io_methods;
     SetMockFileSize( 0 );

     GetDbOpenState( btree
          , &open_state
          , GetMockInternalMethods() );

     CU_ASSERT( open_state == DB_OPENED_CREATING );
}

/// @brief existing database test
void get_db_open_existing_state_test()
{
     db_open_state open_state = DB_CLOSED; 
     const int fake_size = 1024; 

     // just opened already existed database
     Btree* btree = GetMockBtree();
     sqlite3_file* fd_db = GetMockFileDescriptor( btree );
     sqlite3_io_methods* io_methods = GetMockIoMethods();
     fd_db->pMethods = io_methods;
     SetMockFileSize( fake_size );

     GetDbOpenState( btree
          , &open_state
          , GetMockInternalMethods() );

     CU_ASSERT( open_state == DB_OPENED_EXISTING );
}

void get_db_state_test()
{
     CU_pSuite get_state_suite = CU_add_suite( "get db state suite"
          , setup_get_db_state, teardown_db_state );

     CU_add_test( get_state_suite
          , "get db closed state test", get_db_closed_state_test );

     CU_add_test( get_state_suite
          , "get db creating state test", get_db_open_creating_state_test );

     CU_add_test( get_state_suite
          , "get db existing state test", get_db_open_existing_state_test );
}