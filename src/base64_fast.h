
#ifndef _BASE64_FAST_H_
#define _BASE64_FAST_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef ptrdiff_t ssize_t;

ssize_t base64_encode(const char * src, size_t src_len, char * dest, size_t dest_len);
ssize_t base64_decode(const char * src, size_t src_len, char * dest, size_t dest_len);

ssize_t base64_encode_fast(const char * src, size_t src_len, char * dest, size_t dest_len);
ssize_t base64_decode_fast(const char * src, size_t src_len, char * dest, size_t dest_len);

ssize_t base64_encode_alloc(const char * src, size_t src_len, char ** dest);
ssize_t base64_decode_alloc(const char * src, size_t src_len, char ** dest);

#ifdef __cplusplus
}
#endif

#endif  /* !_JIMIC_SYSTEM_CONSOLE_H_ */
