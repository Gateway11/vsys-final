//
//  rprintf.h
//
//  Created by 代祥 on 2023/5/22.
//

#ifndef __RPRINTF_H
#define __RPRINTF_H

#define RECV_DATA 256

int32_t __rprintf_sock_write(char* data, uint32_t size);

#if 0

#define rprintf(format, ...)

#else

#define rprintf(format, ...) {                                                  \
  char __str[RECV_DATA];                                                        \
  int32_t __ret = sprintf(__str, format, ##__VA_ARGS__);                        \
  __ret = __rprintf_sock_write(__str, __ret);                                   \
}
#endif

#endif /* __RPRINTF_H */
