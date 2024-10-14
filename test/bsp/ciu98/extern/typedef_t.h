
#ifndef TYPEDEF_T_H__
#define TYPEDEF_T_H__
/******************************************************************************/
#include <stdint.h>               /* for standaed integer types               */
#include <string.h>               /* for string library                       */
#include <stddef.h>               /* for NULL declaration                     */
#include <stdbool.h>              /* for boolean type                         */
#include <limits.h>               /* for UINT_MAX type                        */
/******************************************************************************/
typedef bool bool_t;
/******************************************************************************/
#ifndef NULL
#define NULL                      ((void *)0)
#endif

#define ADDR_NULL                 ((uint32_t)NULL)
#define INVALID_ADDRESS           ((uint32_t)UINT_MAX)

#define SET_BIT(x, y)             (x |= ((uint32_t)0x01 << y))
#define CLR_BIT(x, y)             (x &= (~((uint32_t)0x01 << y)))
#define CHECK_BIT(x, y)           (x &  ((uint32_t)0x01 << y))
#define GET_BIT(x, y)             (x &  ((uint32_t)0x01 << y))

/**
  * @brief      If you have some code which relies on certain constants being
  *             equal, or other compile-time-evaluated condition, you should use
  *             BUILD_BUG_ON to detect if someone changes it.
  *             The implementation uses gcc's reluctance to create a negative
  *             array, but gcc (as of 4.4) only emits that error for obvious
  *             cases (eg. not arguments to inline functions). So as a fallback
  *             we use the optimizer; if it can't prove the condition is false,
  *             it will cause a link error on the undefined
  *             ¡°__build_bug_on_failed¡±. This error message can be harder to
  *             track down though, hence the two different methods.
  * @param[in]  condition:the condition which the compiler should know is false..
  * @param[out] None.
  * @return     None.
  * @note       None.
  */
#define BUILD_BUG_ON(condition)  ((void)sizeof(char[1 - 2 * !!(condition)]))
/******************************************************************************/
#endif/* TYPEDEF_T_H__ */

