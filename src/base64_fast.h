
#ifndef _BASE64_FAST_H_
#define _BASE64_FAST_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(__linux__)
#  include <endian.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#  include <sys/endian.h>
#elif defined(__OpenBSD__)
#  include <sys/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef ptrdiff_t ssize_t;

// Endian conversion functions
#if !defined(__BYTE_ORDER) || (__BYTE_ORDER != __BIG_ENDIAN || __BIG_ENDIAN == 0)
  #if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS)
    uint32_t __byteswap32(uint32_t v);
    uint64_t __byteswap64(uint64_t v);
    #define INLINE __inline
  #else
    #define __byteswap32(x) __builtin_bswap32(x)
    #define __byteswap64(x)	__builtin_bswap64(x)
    #define INLINE inline
  #endif
#else
    #define __byteswap32(x)	(x)
    #define __byteswap64(x)	(x)
    #define INLINE inline
#endif

ssize_t base64_encode(const char * src, size_t src_len, char * dest, size_t dest_len);
ssize_t base64_decode(const char * src, size_t src_len, char * dest, size_t dest_len);

ssize_t base64_encode_fast(const char * src, size_t src_len, char * dest, size_t dest_len);
ssize_t base64_decode_fast(const char * src, size_t src_len, char * dest, size_t dest_len);

ssize_t base64_encode_malloc(const char * src, size_t src_len, char ** dest);
ssize_t base64_decode_malloc(const char * src, size_t src_len, char ** dest);

extern const unsigned char base64_enc_chars[];
extern const unsigned char base64_dec_table[];

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS)

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
static INLINE
__declspec(naked)
uint32_t __byteswap32(uint32_t v)
{
    __asm {
        mov eax, v
        bswap eax
        ret
    }
}

#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
static INLINE
__declspec(naked)
uint64_t __byteswap64(uint64_t v)
{
    __asm {
        bswap rdi
        movq rax, rdi
        ret
    }
}
#else
static INLINE
uint64_t __byteswap64(uint64_t v)
{
    union __uint64_u {
        struct __uint64_t {
            uint32_t low;
            uint32_t high;
        } v;
        uint64_t u64;
    };

    union __uint64_u * u64 = (union __uint64_u *)&v;
    uint32_t low = __byteswap32(u64->v.low);
    uint32_t high = __byteswap32(u64->v.high);
    union __uint64_u r;
    r.v.low = high;
    r.v.high = low;
    return r.u64;
}
#endif // _WIN64
#endif // _MSC_VER

#endif // _WIN32

#ifdef __cplusplus
}
#endif

#endif  /* !_JIMIC_SYSTEM_CONSOLE_H_ */
