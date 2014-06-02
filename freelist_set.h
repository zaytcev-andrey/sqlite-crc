#ifndef freelist_set_h__
#define freelist_set_h__

#ifdef __cplusplus
extern "C" 
{
#endif

void InserFreePageNumber( sqlite_int64 pageNumber );

int IsFreePageExist( sqlite_int64 pageNumber );

void RemoveFreePageNamber( sqlite_int64 pageNumber );

void ClearFreePageNumbers();

#ifdef __cplusplus
}
#endif

#endif // freelist_set_h__