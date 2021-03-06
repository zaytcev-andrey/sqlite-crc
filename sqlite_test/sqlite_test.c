// sqlite_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define SQLITE_HAS_CODEC 1

#include <sqlite/sqlite3.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <Windows.h>

// unit-testing framework
#include "Basic.h"

// mocks
#include "sqlite_mocks.h"

// tests
#include "get_db_state_test.h"
#include "read_db_page_test.h"

static const char* dbname_ref = "./testdb_ref";
static const char* key = "anotherkey";

    
static const char * CREATE_TABLE_TEST = 
     "create table 'test' (id INTEGER PRIMARY KEY, name TEXT, creationtime TEXT);";
static const char * CREATE_TABLE_TEST2 = 
     "create table 'test2' (id INTEGER PRIMARY KEY, name TEXT, creationtime TEXT);";
static const char * INSERT_INTO_TEST = 
     "INSERT INTO test (name, creationtime) VALUES ('widget', '1st time');\
     INSERT INTO test (name, creationtime) VALUES ('widget', '2nd time');\
     INSERT INTO test (name, creationtime) VALUES ('widget', '3rd time');\
     INSERT INTO test (name, creationtime) VALUES ('widget', '4th time');\
     INSERT INTO test (name, creationtime) VALUES ('widget', '5th time');";
static const char * INSERT_INTO_TEST2 = 
     "INSERT INTO test2 (name, creationtime) VALUES ('widget2', '1st time2');\
     INSERT INTO test2 (name, creationtime) VALUES ('widget2', '2nd time2');\
     INSERT INTO test2 (name, creationtime) VALUES ('widget2', '3rd time2');\
     INSERT INTO test2 (name, creationtime) VALUES ('widget2', '4th time2');\
     INSERT INTO test2 (name, creationtime) VALUES ('widget2', '5th time2');";
static const char * INSERT_INTO_TEST_ITEM = 
     "INSERT INTO test (name, creationtime) VALUES ('long string for test %d', '%d time');";
static const char * DELETE_FROM_TEST_ITEM = 
     "DELETE FROM test WHERE name = 'long string for test %d';";
static const char * SELECT_FROM_TEST = 
     "SELECT * FROM test;";
static const char * SELECT_FROM_TEST2 = 
     "SELECT * FROM test2;";

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
     int i;
     fprintf(stderr, "\t");
     for(i=0; i<argc; i++){
          fprintf(stderr, "%s = %s | ", azColName[i], argv[i] ? argv[i] : "NULL");
     }
     fprintf(stderr, "\n");
     return 0;
}

int insert_item( sqlite3* db, int idx )
{
     char* error = 0;
     static char buffer[ 1024 ];
     int rc = SQLITE_ERROR;

     sprintf( buffer, INSERT_INTO_TEST_ITEM, idx );
     rc = sqlite3_exec(db, buffer, 0, 0, &error);
     if (rc != SQLITE_OK) { fprintf(stderr, "SQL error: %s\n", error); }
     return rc;
}

int insert_items( sqlite3* db )
{
     int rc = SQLITE_OK;
     size_t idx = 0;
     const int count = 100; 
     fprintf(stdout, "Inserting %d records to test\n");
     
     for ( idx = 3; idx < count + 3; idx++ )
     {
          rc = insert_item( db, idx );

          if (rc != SQLITE_OK) { fprintf(stderr, "insert_items error\n"); return rc; }
     }

     return rc;
}

int delete_item( sqlite3* db, int idx )
{
     char* error = 0;
     static char buffer[ 1024 ];
     int rc = SQLITE_ERROR;

     sprintf( buffer, DELETE_FROM_TEST_ITEM, idx );
     rc = sqlite3_exec(db, buffer, 0, 0, &error);
     if (rc != SQLITE_OK) { fprintf(stderr, "SQL error: %s\n", error); }
     return rc;
}

int delete_items( sqlite3* db )
{
     int rc = SQLITE_OK;
     const int count = 80;
     size_t idx = 0;

     fprintf(stdout, "Deleting %d records from test\n");

     for ( idx = 0; idx < count; idx++ )
     {
          int rc = delete_item( db, idx );

          if (rc != SQLITE_OK) { fprintf(stderr, "delete_items error\n"); return rc; }
     }

     return rc;
}

int create_and_close_test_db( const char* db_path )
{
     char* error = 0;
     sqlite3 * db = 0;
     int rc = SQLITE_ERROR;

     if ( !DeleteFileA( db_path ) )
     {
          if ( GetLastError() != ERROR_FILE_NOT_FOUND )
          {
               fprintf(stderr, "Can't delete database: %s\n", db_path);
               return SQLITE_ERROR; 
          }
     }

     fprintf(stderr, "Creating Database \"%s\"\n", db_path);
     rc = sqlite3_open(db_path, &db);
     if (rc != SQLITE_OK) { fprintf(stderr, "Can't open/create database: %s\n", sqlite3_errmsg(db)); return rc; }

     fprintf(stderr, "Keying Database with key \"%s\"\n", key);
     rc = sqlite3_key(db, key, strlen( key ) );
     if (rc != SQLITE_OK) { fprintf(stderr, "Can't key database: %s\n", sqlite3_errmsg(db)); return rc; } 

     fprintf(stderr, "Creating table \"test\"\n");
     rc = sqlite3_exec(db, CREATE_TABLE_TEST, 0, 0, &error);

     fprintf(stderr, "Inserting into table \"test\"\n");
     rc = sqlite3_exec(db, INSERT_INTO_TEST, 0, 0, &error);
     if (rc != SQLITE_OK) { fprintf(stderr, "SQL error: %s\n", error); return rc; }

     insert_items( db );
     if (rc != SQLITE_OK) { return rc; }

     delete_items( db );

     fprintf(stderr, "Closing Database \"%s\"\n", db_path);
     rc = sqlite3_close(db);

     return rc;
}

// Unit tests



int _tmain(int argc, _TCHAR* argv[])
{
     sqlite3 * db;
     char * error=0;

     CU_initialize_registry();

     // get db state test suite
     get_db_state_test();

     // read db page test suite
     read_db_page_test();
     
     CU_basic_set_mode( CU_BRM_VERBOSE );
     CU_set_error_action( CUEA_IGNORE );

     CU_basic_run_tests();
     CU_cleanup_registry();
     
     return 0;

     // FIXME acceptance test, need to repair
     {
          int rc = SQLITE_ERROR;

          rc = create_and_close_test_db( dbname_ref );

          if ( rc != SQLITE_OK) { fprintf(stderr, "Can't create reference database\n"); return 1; }
          
          fprintf(stderr, "Opening Database \"%s\"\n", dbname_ref);
          rc = sqlite3_open(dbname_ref, &db);
          if (rc != SQLITE_OK) { fprintf(stderr, "Can't open/create database: %s\n", sqlite3_errmsg(db)); return 1; }

          fprintf(stderr, "Keying Database with key \"%s\"\n", key);
          rc = sqlite3_key(db, key, strlen( key ));
          if (rc != SQLITE_OK) { fprintf(stderr, "Can't key database: %s\n", sqlite3_errmsg(db)); return 1; }

          fprintf(stderr, "Closing Database \"%s\"\n", dbname_ref);
          sqlite3_close(db); 
          
          return 0;

     }
}

