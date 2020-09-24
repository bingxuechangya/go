#ifndef _COMMON_H_
#define _COMMON_H_ 1

#include <inttypes.h>
#include <stddef.h> // for "size_t"

#define IN
#define OUT
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

#ifdef __cplusplus
extern "C" {
#endif
void *xmalloc(size_t size);
void xfree(void *ptr);
#ifdef __cplusplus
}
#endif

extern int verbose_flag __attribute__((unused));

#endif /* ifndef _COMMON_H_ */
