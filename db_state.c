#include "db_state.h"

static int isOpen( sqlite3_file* f_db );

int GetDbOpenState( Btree* btree, db_open_state* state, sqlite_internal_methods* int_methods )
{
     sqlite3_file* f_db = int_methods->xGetDbFileDescriptor( btree ); // sqlite3BtreePager
     sqlite3_int64 file_size = 0;

     int rc = SQLITE_ERROR;
     *state = DB_CLOSED;

     if( !isOpen( f_db ) )
     {
          rc = f_db->pMethods->xFileSize( f_db, &file_size ); // sqlite3OsFileSize
               
          if( rc == SQLITE_IOERR_SHORT_READ )
          {
               rc = SQLITE_OK;
          }

          if ( rc == SQLITE_OK )
          {
               *state = file_size ? 
               DB_OPENED_EXISTING : DB_OPENED_CREATING;
          }
     }

     return rc;
}

int isOpen( sqlite3_file* f_db )
{
     return f_db->pMethods ? 1 : 0;
}