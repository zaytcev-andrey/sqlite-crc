#ifndef i_check_crc_h__
#define i_check_crc_h__

#define CHECKSUM_SUCCESS 0
#define CHECKSUM_ERROR 1

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

#endif // i_check_crc_h__