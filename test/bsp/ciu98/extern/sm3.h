/**
 * \file sm3.h
 */
#ifndef SM3_H
#define SM3_H

/**
 * \brief          Output = SM3( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SM3 checksum result
 */
void sm3( unsigned char *input, int ilen,
           unsigned char output[32]);

#endif /* sm3.h */          