#include "codec.c"
#include "checked_codec.c"
#include "md5.h"
#include "freelist_set.h"

#include <stdio.h>

#ifndef SQLITE_OMIT_DISKIO
#ifdef SQLITE_HAS_CODEC

#define HASH_SIZE 16

// Guessing that "see" is related to SQLite Encryption Extension" (the semi-official, for-pay, encryption codec)
// Just as useful for initializing.
void sqlite3_activate_see(const char *info)
{
}

// Free the encryption codec, called from pager.c (address passed in sqlite3PagerSetCodec)
void sqlite3PagerFreeCodec(void *pCodec)
{
     if (pCodec)
     {
          sqlite3_free(pCodec);
     }
}

// Report the page size to the codec, called from pager.c (address passed in sqlite3PagerSetCodec)
void sqlite3CodecSizeChange(void *pCodec, int pageSize, int nReserve)
{
}

void* GetPageCopyWithReservedSpace( void* data, int page_size, Codec* codec )
{
     int* reserved_value = 0;
     unsigned char hash[ HASH_SIZE ];
     unsigned char* page_buff = malloc( page_size );
     memcpy( page_buff, data, page_size - HASH_SIZE );
     memset( hash, 0, sizeof( hash ) );
     
     if ( codec )
     {
          GetMD5Binary( page_buff, page_size - HASH_SIZE, hash );
     }

     memcpy( page_buff + page_size - HASH_SIZE, hash, HASH_SIZE );
     return page_buff;
}

// Encrypt/Decrypt functionality, called by pager.c
void* sqlite3Codec(void *pCodec, void *data, Pgno nPageNum, int nMode)
{
     Codec* codec = NULL;
     int* reserved = 0;
     int pageSize = 0;
     int reserve = 0;
     if (pCodec == NULL)
     {
          return data;
     }
     codec = (Codec*)pCodec;
     /*if ( !CodecIsEncrypted(codec) )
     {
          return data;
     }*/

     pageSize = sqlite3BtreeGetPageSize(CodecGetBtree(codec));
     reserve = sqlite3BtreeGetReserve( CodecGetBtree(codec) );

     reserved = ( int* )( ( char* )data + pageSize );

     switch(nMode)
     {
          case 0: /* Undo a "case 7" journal file encryption */
          case 2: /* Reload a page */
          case 3: /* Load a page */
          break;
               
          case 6: /* Encrypt a page for the main database file */
               data = GetPageCopyWithReservedSpace( data, pageSize, codec );
          break;

          case 7: /* Encrypt a page for the journal file */
          /* Under normal circumstances, the readkey is the same as the writekey.  However,
          when the database is being rekeyed, the readkey is not the same as the writekey.
          The rollback journal must be written using the original key for the
          database file because it is, by nature, a rollback journal.
          Therefore, for case 7, when the rollback is being written, always encrypt using
          the database's readkey, which is guaranteed to be the same key that was used to
          read the original data.
          */
          data = GetPageCopyWithReservedSpace( data, pageSize, codec );
          break;
  }

  return data;

}

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

static int ReadDbHeader( sqlite3_file* f_db, db_info* db_i );
static int ReadDbPage( sqlite3_file* f_db, db_info* db_i );
static int ConvertFromBigEndian( char* buff, int size );
static int GetDbOpenState( Btree* btree, db_info* db_i );
static int SetDbReservedState( sqlite3* db, Btree* btree, check_crc* crc_impl );

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

int GetDbOpenState( Btree* btree, db_info* db_i )
{
     sqlite3_file* f_db = sqlite3BtreePager( btree )->fd;

     int rc = SQLITE_ERROR;
     db_i->open_state = DB_CLOSED;
     
     if( !isOpen( f_db ) )
     {
          rc = sqlite3OsFileSize( f_db, &db_i->file_size );
          if( rc == SQLITE_IOERR_SHORT_READ )
          {
               rc = SQLITE_OK;
          }
          
          if ( rc == SQLITE_OK )
          {
               db_i->open_state = db_i->file_size ? 
                    DB_OPENED_EXISTING : DB_OPENED_CREATING;
          }
     }

     return rc;
}

int SetDbReservedState( sqlite3* db, Btree* btree, check_crc* crc_impl )
{
     int rc = SQLITE_ERROR;
     int reserve = sqlite3BtreeGetReserve( btree );
     FILE* f_db = 0;

     if ( reserve != crc_impl->xGetCrcLength() && reserve >= 0 )
     {
          reserve = crc_impl->xGetCrcLength();
          rc = sqlite3BtreeSetPageSize( btree, -1, reserve, 0 );

          if ( rc != SQLITE_OK )
          {
               sqlite3Error(db, rc, "unable to reserve page space for the codec");
          }
     }
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

int ConvertFromBigEndian( unsigned char* buff, int size )
{
     static int bing_endian = -1;
     unsigned int res = 0;
     int idx = 0;

     if ( bing_endian < 0 )
     {
          /* Are we little or big endian? This method is from Harbison & Steele. */
          union
          {
               long l;
               char c[ sizeof( long ) ];
          } u;
          u.l = 1;
          bing_endian = ( u.c[0] != 1 ) ? 1 : 0;
     }

     if ( bing_endian != 1 )
     {
          for ( idx = size - 1; idx >= 0; idx-- )
          {
               res += buff[ idx ] << 8 * ( size - idx - 1 );
          }
          return res;
     }

     memcpy( buff, &res, size );

     return res;
}

int sqlite3CodecAttach(sqlite3 *db, int nDb, const void *zKey, int nKey)
{
     /* Attach a key to a database. */
     Codec* codec = (Codec*) sqlite3_malloc(sizeof(Codec));
     checked_codec* ch_codec = (checked_codec*)sqlite3_malloc( sizeof( checked_codec ) );
     CodecInit(codec);

     /* No key specified, could mean either use the main db's encryption or no encryption */
     if (zKey == NULL || nKey <= 0)
     {
         
     }
     else
     {
          /* Key specified, setup encryption key for database */
          int rc = SQLITE_OK;
          Btree *pBt = db->aDb[nDb].pBt;

          sqlite3BtreeEnter( pBt );
          {
               check_crc* crc_impl = 0;
               db_info* db_i = 0;

               // инициализация объекта для проверки crc
               crc_impl = ( check_crc* )malloc( sizeof( check_crc ) );
               InitializeCheckCrc( crc_impl
                    , CheckMD5
                    , GetMD5Binary 
                    , GetMD5Length );

               // инициализация кодека
               InitializeCheckedCodec( ch_codec
                    , db->aDb[nDb].pBt
                    , crc_impl );

               // установка резервирования на странице
               rc = SetDbReservedState( db, ch_codec->btree, crc_impl );
               if ( rc != SQLITE_OK )
               {
                    sqlite3BtreeLeave( pBt );
                    return rc;
               }

               sqlite3PagerSetCodec( 
                    sqlite3BtreePager( pBt ), 
                    sqlite3Codec, 
                    sqlite3CodecSizeChange, 
                    sqlite3PagerFreeCodec, 
                    ch_codec ); 

               // определение состояния базы
               db_i = malloc( sizeof( db_info ) );
               rc = GetDbOpenState( ch_codec->btree, db_i );

               // проверка контрольных сумм только на существующей базе,
               // на только что созданной не проверяется.
               if ( rc == SQLITE_OK && db_i->open_state == DB_OPENED_EXISTING )
               {
                    Pager* pager = 0;
                    pager = sqlite3BtreePager( pBt );
                    rc = ReadDbFile( pager->fd, db_i );
                    free( db_i );
               }
          }
          sqlite3BtreeLeave( pBt );

          return rc;   
     }

     return SQLITE_OK;
}

void sqlite3CodecGetKey(sqlite3* db, int nDb, void **zKey, int *nKey)
{
}

int sqlite3_key(sqlite3 *db, const void *zKey, int nKey)
{
    return sqlite3CodecAttach(db, 0, zKey, nKey);
}

int sqlite3_key_v2(sqlite3 *db, const char *zDbName, 
     const void *pKey, int nKey)
{
     return SQLITE_OK;
}

int sqlite3_rekey(sqlite3 *db, const void *zKey, int nKey)
{
    return SQLITE_OK;
}

int sqlite3_rekey_v2( sqlite3 *db, const char *zDbName, 
     const void *pKey, int nKey )
{
     return SQLITE_OK;
}

#endif // SQLITE_HAS_CODEC

#endif // SQLITE_OMIT_DISKIO
