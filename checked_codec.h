#ifndef checked_codec_h__
#define checked_codec_h__

#include "i_check_crc.h"

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