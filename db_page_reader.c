#include "utils.h"
#include "md5.h"
#include "freelist_set.h"

#define HASH_SIZE 16

typedef enum { DB_OPENED_CREATING, DB_OPENED_EXISTING, DB_CLOSED } db_open_state;

typedef struct DB_INFO 
{
     db_open_state open_state;
     int current_page_number;
     int freelist_trunk_page_number;
     int freelist_trunk_page_offset;
     int total_number_of_freelist_pages; // count of all ( trunk and leaf ) pages in freelist 
     i64 current_page_offset;
     int page_header_offset;
     int page_size;
     i64 file_size;
     unsigned char* page_buff;
} db_info;

static int ReadDbFile( sqlite3_file* f_db, db_info* db_i );
static int ReadDbHeader( sqlite3_file* f_db, db_info* db_i );
static int ReadDbPage( sqlite3_file* f_db, db_info* db_i );


int ReadDbFile( sqlite3_file* f_db, db_info* db_i )
{
     int res = SQLITE_ERROR;

     memset( db_i, 0, sizeof( db_info ) );

     res = ReadDbHeader( f_db, db_i );
     if ( res != SQLITE_OK )
     {
          return res;
     }

     res = ReadDbPage( f_db, db_i );

     return res;
}

int ReadDbHeader( sqlite3_file* f_db, db_info* db_i )
{
     char zDbHeader[256];
     int freelist_trunk_page_number = 0;
     int rc = SQLITE_ERROR;

     memset( zDbHeader, 0, sizeof( zDbHeader ) );

     rc = sqlite3OsRead( f_db, zDbHeader, 100, 0 );
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

int ReadDbPage( sqlite3_file* f_db, db_info* db_i )
{
     unsigned char page_type = 0;
     unsigned char reversed[ HASH_SIZE ];
     int page_length = db_i->page_size;
     int page_data_lenght = page_length ? page_length - HASH_SIZE : 0; // FIXME сделать обработку первичного старта
     int md5_check_result = CHECKSUM_ERROR;
     int rc = SQLITE_ERROR;

     rc = sqlite3OsRead( f_db, db_i->page_buff, page_length, db_i->current_page_offset );

     if ( rc == SQLITE_OK )
     {
          memcpy( &page_type, db_i->page_buff, 1 );

          db_i->page_header_offset = 0;

          if ( db_i->current_page_offset == db_i->freelist_trunk_page_offset )
          {
               int idx = 0;
               char* buff_freelist_leaf_page_numbers = 0;

               // количество номеров листовых неиспользуемых страниц
               int freelist_leaf_page_count = ConvertFromBigEndian( db_i->page_buff + 4, 4 );

               // следующая страница в связанном списке freelist
               i64 next_freelist_trunk_page = ConvertFromBigEndian( db_i->page_buff, 4 );
               db_i->freelist_trunk_page_number = 
                    next_freelist_trunk_page ? next_freelist_trunk_page : -1;
               db_i->freelist_trunk_page_offset = 
                    next_freelist_trunk_page ? ( next_freelist_trunk_page - 1 ) * db_i->page_size : -1;

               // чтение номеров листовых неиспользуемых страниц
               buff_freelist_leaf_page_numbers = db_i->page_buff + 8;
               for ( idx = 0; idx < freelist_leaf_page_count; idx++ )
               {
                    int page_number = ConvertFromBigEndian( buff_freelist_leaf_page_numbers, 4 );
                    InserFreePageNumber( page_number );
                    buff_freelist_leaf_page_numbers += 4;
               }
          }

          if ( IsFreePageExist( db_i->current_page_number ) )
          {
               RemoveFreePageNamber( db_i->current_page_number );

               md5_check_result = CHECKSUM_SUCCESS;
          }
          else
          {
               memcpy( reversed, db_i->page_buff + page_length - HASH_SIZE, HASH_SIZE );

               md5_check_result = CheckMD5( db_i->page_buff, page_data_lenght, reversed );
          }

          if ( md5_check_result == CHECKSUM_SUCCESS )
          {
               db_i->current_page_number++;
               db_i->current_page_offset += page_length;

               if ( db_i->current_page_offset < db_i->file_size )
               {
                    return ReadDbPage( f_db, db_i );
               }

               return SQLITE_OK;
          }

          ClearFreePageNumbers();
          return SQLITE_ERROR;
     }

     return rc;
}