#include "utils.h"

#include "db_page_reader_impl.h"


int ReadDbHeader( sqlite3_file* f_db, db_info* db_i )
{
     char zDbHeader[256];
     int freelist_trunk_page_number = 0;
     int rc = SQLITE_ERROR;

     memset( zDbHeader, 0, sizeof( zDbHeader ) );

     rc = f_db->pMethods->xRead( f_db, zDbHeader, 100, 0); // sqlite3OsRead
     if( rc == SQLITE_IOERR_SHORT_READ )
     {
          rc = SQLITE_OK;
     }

     if ( rc == SQLITE_OK )
     {
          db_i->page_size = ConvertFromBigEndian( zDbHeader + 16, 2 );
          db_i->page_buff = ( unsigned char* )malloc( db_i->page_size );

          freelist_trunk_page_number = ConvertFromBigEndian( zDbHeader + 32, 4 );
          db_i->current_page_number = 1;
          db_i->freelist_trunk_page_number = 
               freelist_trunk_page_number ? freelist_trunk_page_number : -1;
          db_i->freelist_trunk_page_offset = 
               freelist_trunk_page_number ? ( freelist_trunk_page_number - 1 ) * db_i->page_size : -1;
          db_i->total_number_of_freelist_pages = ConvertFromBigEndian( zDbHeader + 36, 4 );
          db_i->page_header_offset = 100;
     }

     return rc;
}

/*int ReadDbPage( sqlite3_file* f_db, db_info* db_i, check_crc* crc  )
{
     return 0; // FIXME
}*/