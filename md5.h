#ifndef MD5_H_DFF2AC80_7706_47FC_BE30_D95BE2F48565
#define MD5_H_DFF2AC80_7706_47FC_BE30_D95BE2F48565

#define CHECKSUM_SUCCESS 0
#define CHECKSUM_ERROR 1

/// @brief ������������ MD5 ��� ������ ������
/// @param[in] data ����� ������, ��� ������� �������������� MD5
/// @param[in] length ������ ������
/// @param[out] digest ��������� MD5 ���
void GetMD5Binary( unsigned char* data, int length, unsigned char* digest );

/// @brief ���������� MD5 ��� ������ ������ � ������������ ����� MD5
/// @param[in] data ����� ������, ��� ������� ������������ MD5
/// @param[in] length ������ ������
/// @param[out] md5 MD5 ���, � ������� ������������
int CheckMD5( unsigned char* data, int length, unsigned char* md5 );

/// @brief 
unsigned int GetMD5Length();

#endif // MD5_H_DFF2AC80_7706_47FC_BE30_D95BE2F48565