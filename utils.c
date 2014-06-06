#include "utils.h"

#include <memory.h>

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