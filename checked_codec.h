#ifndef checked_codec_h__
#define checked_codec_h__

typedef struct CHECK_CRC
{
     void (*xGetCrc)( unsigned char* /*data*/, int /*length*/, unsigned char* /*digest*/ );
     void (*xCheckCrc)( unsigned char* /*data*/, int /*length*/, unsigned char* /*crc*/ );
     unsigned int (*xGetCrcLength)();
} check_crc;

typedef struct CHECKED_CODEC
{
     sqlite3* db;
     Btree* btree;
     check_crc* crc_methods;
} checked_codec;

void SetCheckedCodecBtree( checked_codec* codec, Btree* btree );

void SetCheckedMethods( checked_codec* codec, check_crc* crc_methods );

#endif // checked_c*odec_h__