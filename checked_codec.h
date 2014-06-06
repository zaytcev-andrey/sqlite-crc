#ifndef checked_codec_h__
#define checked_codec_h__

/// @brief Объявления типов методов
typedef void (*PFGetCrc)( unsigned char* /*data*/, int /*length*/, unsigned char* /*digest*/ );
typedef void (*PFCheckCrc)( unsigned char* /*data*/, int /*length*/, unsigned char* /*crc*/ );
typedef unsigned int (*PFGetCrcLength)();

/// @brief Интерфейс для расчета контрольных сумм
typedef struct CHECK_CRC
{
     PFGetCrc xGetCrc;
     PFCheckCrc xCheckCrc;
     PFGetCrcLength xGetCrcLength;
} check_crc;

/// @brief Инициализация объекта расчета контрольных сумм
void InitializeCheckCrc( check_crc* crc
     , PFGetCrc xGetCrc
     , PFCheckCrc xCheckCrc
     , PFGetCrcLength xGetCrcLength );

/// @brief Интерфейс кодека с поддержкой контрольных сумм, без шифрования
typedef struct CHECKED_CODEC
{
     sqlite3* db;
     Btree* btree;
     check_crc* crc_methods;
} checked_codec;

/// @brief Инициализация кодека
void InitializeCheckedCodec( checked_codec* codec
     , Btree* btree
     , check_crc* crc_methods );

#endif // checked_c*odec_h__