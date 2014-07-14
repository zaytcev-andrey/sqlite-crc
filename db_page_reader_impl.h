#ifndef db_page_reader_impl_h__
#define db_page_reader_impl_h__

#include "sqlite/sqlite3.h"
#include "db_state.h"
#include "i_check_crc.h"

typedef struct DB_INFO 
{
     db_open_state open_state;
     int current_page_number;
     int freelist_trunk_page_number;
     int freelist_trunk_page_offset;
     int total_number_of_freelist_pages; // count of all ( trunk and leaf ) pages in freelist 
     sqlite3_int64 current_page_offset;
     int page_header_offset;
     int page_size;
     sqlite3_int64 file_size;
     unsigned char* page_buff;
} db_info;

int ReadDbHeader( sqlite3_file* f_db, db_info* db_i );
// int ReadDbPage( sqlite3_file* f_db, db_info* db_i, check_crc* crc );

#endif // db_page_reader_impl_h__