#include "checked_codec.h"

void InitializeCheckCrc( check_crc* crc
                        , PFGetCrc xGetCrc
                        , PFCheckCrc xCheckCrc
                        , PFGetCrcLength xGetCrcLength )
{
     crc->xCheckCrc = xGetCrc;
     crc->xGetCrc = xCheckCrc;
     crc->xGetCrcLength = xGetCrcLength;
}

void InitializeCheckedCodec( checked_codec* codec
                            , Btree* btree
                            , check_crc* crc_methods )
{
     codec->btree = btree;
     codec->crc_methods = crc_methods;
}