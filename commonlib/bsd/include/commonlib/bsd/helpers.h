/* SPDX-License-Identifier: BSD-3-Clause */
/* Vendored minimal stub — only macros needed by intelmetool */
#ifndef _COMMONLIB_BSD_HELPERS_H_
#define _COMMONLIB_BSD_HELPERS_H_
#include <stddef.h>
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif
#ifndef MIN
#define MIN(a, b) ({ __typeof__(a) _a=(a); __typeof__(b) _b=(b); _a<_b?_a:_b; })
#endif
#ifndef MAX
#define MAX(a, b) ({ __typeof__(a) _a=(a); __typeof__(b) _b=(b); _a>_b?_a:_b; })
#endif
#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(x,y) (((x)+(y)-1)/(y))
#endif
#endif /* _COMMONLIB_BSD_HELPERS_H_ */
