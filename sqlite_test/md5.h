#ifndef MD5_H_DFF2AC80_7706_47FC_BE30_D95BE2F48565
#define MD5_H_DFF2AC80_7706_47FC_BE30_D95BE2F48565

/// @brief Рассчитывает MD5 для набора данных
/// @param[in] data Набор данных, для которых рассчитывается MD5
/// @param[in] length Размер данных
/// @param[out] digest Результат MD5 хэш
void GetMD5Binary( unsigned char* data, int length, unsigned char* digest );

/// @brief Сравнивает MD5 для набора данных с рассчитанной ранее MD5
/// @param[in] data Набор данных, для которых сравнивается MD5
/// @param[in] length Размер данных
/// @param[out] md5 MD5 хэш, с которым сравнивается
bool CheckMD5( unsigned char* data, int length, unsigned char* md5 );

#endif // MD5_H_DFF2AC80_7706_47FC_BE30_D95BE2F48565