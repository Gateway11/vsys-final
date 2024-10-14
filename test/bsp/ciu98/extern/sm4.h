/**
 * \file sm4.h
 */
#ifndef SM4_H
#define SM4_H

#define SM4_ENCRYPT     1
#define SM4_DECRYPT     0


#ifdef __cplusplus
extern "C" {
#endif

#define SYM_ECB_EN 0x00
#define SYM_ECB_DE 0x40
#define SYM_CBC_EN 0x20
#define SYM_CBC_DE 0x60

/**
 * \brief          SM4-ECB block encryption/decryption
 * \param symkey   SM4 key
 * \param mode     SM4_ENCRYPT or SM4_DECRYPT
 * \param length   length of the input data
 * \param input    input block
 * \param output   output block
 */
int sm4_crypt_ecb( unsigned char *symkey,
				     int mode,
					 int length,
                     unsigned char *input,
                     unsigned char *output);

/**
 * \brief          SM4-CBC buffer encryption/decryption
 * \param symkey   SM4 key
 * \param mode     SM4_ENCRYPT or SM4_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 */
int sm4_crypt_cbc( unsigned char *symkey,
                     int mode,
                     int length,
                     unsigned char iv[16],
                     unsigned char *input,
                     unsigned char *output );

#ifdef __cplusplus
}
#endif

#endif /* sm4.h */
