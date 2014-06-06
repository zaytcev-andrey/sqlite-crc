#ifndef checked_codec_h__
#define checked_codec_h__

/// @brief ���������� ����� �������
typedef void (*PFGetCrc)( unsigned char* /*data*/, int /*length*/, unsigned char* /*digest*/ );
typedef void (*PFCheckCrc)( unsigned char* /*data*/, int /*length*/, unsigned char* /*crc*/ );
typedef unsigned int (*PFGetCrcLength)();

/// @brief ��������� ��� ������� ����������� ����
typedef struct CHECK_CRC
{
     PFGetCrc xGetCrc;
     PFCheckCrc xCheckCrc;
     PFGetCrcLength xGetCrcLength;
} check_crc;

/// @brief ������������� ������� ������� ����������� ����
void InitializeCheckCrc( check_crc* crc
     , PFGetCrc xGetCrc
     , PFCheckCrc xCheckCrc
     , PFGetCrcLength xGetCrcLength );

/// @brief ��������� ������ � ���������� ����������� ����, ��� ����������
typedef struct CHECKED_CODEC
{
     sqlite3* db;
     Btree* btree;
     check_crc* crc_methods;
} checked_codec;

/// @brief ������������� ������
void InitializeCheckedCodec( checked_codec* codec
     , Btree* btree
     , check_crc* crc_methods );

#endif // checked_c*odec_h__