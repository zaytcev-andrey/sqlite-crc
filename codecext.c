#include "md5.h"
#include "checked_codec.c"
#include "db_state.h"
#include "db_page_reader.c"
#include "i_sqlite_internal_methods.h"

#include <stdio.h>

#ifndef SQLITE_OMIT_DISKIO
#ifdef SQLITE_HAS_CODEC

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

static int SetDbReservedState( sqlite3* db, Btree* btree, check_crc* crc_impl );

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

void* GetPageCopyWithReservedSpace( void* data, int page_size, checked_codec* ch_codec )
{
     const int hash_size = ch_codec->crc_methods->xGetCrcLength();
     const int page_data_size = page_size - hash_size;

     unsigned char* hash = malloc( hash_size );
     unsigned char* page_buff = malloc( page_size );

     memcpy( page_buff, data, page_data_size );
     memset( hash, 0, sizeof( hash ) );
     
     if ( ch_codec )
     {
          ch_codec->crc_methods->xGetCrc( page_buff, page_data_size, hash );
     }

     memcpy( page_buff + page_data_size, hash, hash_size );
     free( hash );

     return page_buff;
}

// Encrypt/Decrypt functionality, called by pager.c
void* sqlite3Codec(void *pCodec, void *data, Pgno nPageNum, int nMode)
{
     checked_codec* ch_codec = NULL;
     int pageSize = 0;
     int reserve = 0;

     if ( pCodec == NULL )
     {
          return data;
     }

     ch_codec = ( checked_codec* )pCodec;

     pageSize = sqlite3BtreeGetPageSize( ch_codec->btree );
     reserve = sqlite3BtreeGetReserve( ch_codec->btree );

     switch(nMode)
     {
          case 0: /* Undo a "case 7" journal file encryption */
          case 2: /* Reload a page */
          case 3: /* Load a page */
          break;
               
          case 6: /* Encrypt a page for the main database file */
               data = GetPageCopyWithReservedSpace( data, pageSize, ch_codec );
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
          data = GetPageCopyWithReservedSpace( data, pageSize, ch_codec );
          break;
  }

  return data;

}

/// wrapper methods for internal
static sqlite3_file* GetDbFileDescriptor( Btree* btree )
{
     return sqlite3BtreePager( btree )->fd;
}
 
///

int sqlite3CodecAttach(sqlite3 *db, int nDb, const void *zKey, int nKey)
{
     /* Attach a key to a database. */

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
               checked_codec* ch_codec = (checked_codec*)sqlite3_malloc( sizeof( checked_codec ) );
               check_crc* crc_impl = 0;
               sqlite_internal_methods int_methods;
               db_open_state open_state = DB_CLOSED;

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

               InitializeInternalMethods( &int_methods
                    , GetDbFileDescriptor );

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
               rc = GetDbOpenState( ch_codec->btree, &open_state, &int_methods );

               // проверка контрольных сумм только на существующей базе,
               // на только что созданной не проверяется.
               if ( rc == SQLITE_OK && open_state == DB_OPENED_EXISTING )
               {
                    sqlite3_file* fd = int_methods.xGetDbFileDescriptor( pBt );
                    rc = ReadDbFile( fd, ch_codec->crc_methods );
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
