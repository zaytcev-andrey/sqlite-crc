#include "stdafx.h"

#include "sqlite_mocks.h"

static Btree mock_btree_;
static sqlite_internal_methods mock_internal_methods_;
static sqlite3_file fd_db_;
static sqlite3_io_methods io_methods_;
static int file_size_;

Btree* GetMockBtree()
{
     return &mock_btree_;
}

sqlite_internal_methods* GetMockInternalMethods()
{
     return &mock_internal_methods_;
}

sqlite3_file* GetMockFileDescriptor( Btree* btree )
{
     return &fd_db_;
}

sqlite3_io_methods* GetMockIoMethods()
{
     return &io_methods_;
}

int GetMockFileSize( sqlite3_file* fd, sqlite3_int64 *pSize )
{
     *pSize = file_size_;

     return SQLITE_OK;
}

void SetMockFileSize( sqlite3_int64 size )
{
     file_size_ = size;
}