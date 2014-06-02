#include "sqlite/sqlite3.h"
#include <set>

#include "freelist_set.h"


namespace
{

std::set< sqlite_int64 > freelist;

}

void InserFreePageNumber( sqlite_int64 pageNumber )
{
     freelist.insert( pageNumber );
}

int IsFreePageExist( sqlite_int64 pageNumber )
{
     return static_cast<int>( freelist.find( pageNumber ) != freelist.end() );
}

void RemoveFreePageNamber( sqlite_int64 pageNumber )
{
     freelist.erase( pageNumber );
}

void ClearFreePageNumbers()
{
     freelist.clear();
}