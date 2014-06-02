#include "checked_codec.h"

void SetCheckedCodecBtree( checked_codec* codec, Btree* btree )
{
     codec->btree = btree;
}

void SetCheckedMethods( checked_codec* codec, check_crc* crc_methods )
{
     codec->crc_methods = crc_methods;
}