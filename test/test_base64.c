
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../src/base64_fast.h"
#include "moby_dick.h"

static char out[2000];
static size_t outlen;

static bool
assert_enc(const char *src, const char *dest)
{
	size_t srclen = strlen(src);
	size_t destlen = strlen(dest);
    ssize_t encode_size;

    outlen = sizeof(out) - 1; 
	encode_size = base64_encode_fast(src, srclen, out, outlen);
    outlen = encode_size;

	if (outlen != destlen) {
		printf("FAIL: encoding of '%s': length expected %lu, got %lu\n", src,
			(unsigned long)destlen,
			(unsigned long)outlen
		);
		return true;
	}
	if (strncmp(dest, out, outlen) != 0) {
		out[outlen] = '\0';
		printf("FAIL: encoding of '%s': expected output '%s', got '%s'\n", src, dest, out);
		return true;
	}
	return false;
}

static bool
assert_dec(const char *src, const char *dest)
{
	size_t srclen = strlen(src);
	size_t destlen = strlen(dest);
    ssize_t decode_size;

    outlen = sizeof(out) - 1;
	if ((decode_size = base64_decode_fast(src, srclen, out, outlen)) < 0) {
		printf("FAIL: decoding of '%s': decoding error\n", src);
		return true;
	}
    outlen = decode_size;
	if (outlen != destlen) {
		printf("FAIL: encoding of '%s': "
			"length expected %lu, got %lu\n", src,
			(unsigned long)destlen,
			(unsigned long)outlen
		);
		return true;
	}
	if (strncmp(dest, out, outlen) != 0) {
		out[outlen] = '\0';
		printf("FAIL: decoding of '%s': expected output '%s', got '%s'\n", src, dest, out);
		return true;
	}
	return false;
}

static int
assert_roundtrip(const char *src)
{
	char tmp[1500];
	size_t tmplen;
	size_t srclen = strlen(src);
    ssize_t encode_size, decode_size;

	// Encode the input into global buffer:
    outlen = sizeof(out) - 1;
	encode_size = base64_encode_fast(src, srclen, out, outlen);
    outlen = encode_size;

	// Decode the global buffer into local temp buffer:
    tmplen = sizeof(tmp) - 1;
	if ((decode_size = base64_decode_fast(out, outlen, tmp, tmplen)) < 0) {
		printf("FAIL: decoding of '%s': decoding error\n", out);
		return true;
	}
    tmplen = decode_size;

	// Check that 'src' is identical to 'tmp':
	if (srclen != tmplen) {
		printf("FAIL: roundtrip of '%s': "
			"length expected %lu, got %lu\n", src,
			(unsigned long)srclen,
			(unsigned long)tmplen
		);
		return true;
	}
	if (strncmp(src, tmp, tmplen) != 0) {
		tmp[tmplen] = '\0';
		printf("FAIL: roundtrip of '%s': got '%s'\n", src, tmp);
		return true;
	}

	return false;
}

static int
test_char_table()
{
	bool fail = false;
	char chr[256];
	char enc[400], dec[400];
	size_t enclen, declen;
    ssize_t encode_size, decode_size;

	// Fill array with all characters 0..255:
	for (int i = 0; i < 256; i++)
		chr[i] = (unsigned char)i;

	// Loop, using each char as a starting position to increase test coverage:
	for (int i = 0; i < 256; i++) {

		size_t chrlen = 256 - i;
        enclen = sizeof(enc) - 1;
		encode_size = base64_encode_fast(&chr[i], chrlen, enc, enclen);
        enclen = encode_size;

        declen = sizeof(dec) - 1;
		if ((decode_size = base64_decode_fast(enc, enclen, dec, declen)) < 0) {
			printf("FAIL: decoding @ %d: decoding error\n", i);
			fail = true;
			continue;
		}
        declen = decode_size;
		if (declen != chrlen) {
			printf("FAIL: roundtrip @ %d: "
				"length expected %lu, got %lu\n", i,
				(unsigned long)chrlen,
				(unsigned long)declen
			);
			fail = true;
			continue;
		}
		if (strncmp(&chr[i], dec, declen) != 0) {
			printf("FAIL: roundtrip @ %d: decoded output not same as input\n", i);
			fail = true;
		}
	}

	return fail;
}

static int
test_streaming()
{
	bool fail = false;
	char chr[256];
	char ref[400], enc[400];
	size_t reflen;
    ssize_t encode_size, decode_size;

	// Fill array with all characters 0..255:
	for (int i = 0; i < 256; i++)
		chr[i] = (unsigned char)i;

	// Create reference base64 encoding:
    reflen = sizeof(ref) - 1;
	encode_size = base64_encode_fast(chr, 256, ref, reflen);
    reflen = encode_size;

	// Encode the table with various block sizes and compare to reference:
	for (size_t bs = 1; bs < 255; bs++)
	{
		size_t inpos   = 0;
		size_t partlen = 0;
		size_t enclen  = 0;

		memset(enc, 0, 400);
		for (;;) {
            partlen = (sizeof(enc) - 1) - enclen;
			encode_size = base64_encode_fast(&chr[inpos], (inpos + bs > 256) ? 256 - inpos : bs, &enc[enclen], partlen);
            if (encode_size > 0) {
                enclen += encode_size;
                if (inpos + bs > 256) {
                    break;
                }
                inpos += bs;
            }
            else break;
		}

		if (enclen != reflen) {
			printf("FAIL: stream encoding gave incorrect size: "
				"%lu instead of %lu\n",
				(unsigned long)enclen,
				(unsigned long)reflen
			);
			fail = true;
		}
		if (strncmp(ref, enc, reflen) != 0) {
			printf("FAIL: stream encoding with blocksize %lu failed\n",
				(unsigned long)bs
			);
			fail = true;
		}
	}

	// Decode the reference encoding with various block sizes and
	// compare to input char table:
	for (size_t bs = 1; bs < 255; bs++)
	{
		size_t inpos   = 0;
		size_t partlen = 0;
		size_t enclen  = 0;

		memset(enc, 0, 400);
        decode_size = 1;
		while (decode_size > 0) {
            if (enclen > sizeof(enc) - 1)
                break;
            partlen = (sizeof(enc) - 1) - enclen;
            decode_size = base64_decode_fast(&ref[inpos], (inpos + bs > reflen) ? reflen - inpos : bs, &enc[enclen], partlen);
            if (decode_size > 0) {
    			enclen += decode_size;
	    		inpos += bs;
            }
		}
		if (enclen != 256) {
			printf("FAIL: stream decoding gave incorrect size: "
				"%lu instead of 255\n",
				(unsigned long)enclen
			);
			fail = true;
		}
		if (strncmp(chr, enc, 256) != 0) {
			printf("FAIL: stream decoding with blocksize %lu failed\n",
				(unsigned long)bs
			);
			fail = true;
		}
	}

	return fail;
}

static int
test_one_codec()
{
	bool fail = false;

	printf("Codec %s:\n", "plain");

	// Test vectors:
	struct {
		const char *in;
		const char *out;
	} vec[] = {

		// These are the test vectors from RFC4648:
		{ "",		""         },
		{ "f",		"Zg=="     },
		{ "fo",		"Zm8="     },
		{ "foo",	"Zm9v"     },
		{ "foob",	"Zm9vYg==" },
		{ "fooba",	"Zm9vYmE=" },
		{ "foobar",	"Zm9vYmFy" },

		// The first paragraph from Moby Dick,
		// to test the SIMD codecs with larger blocksize:
		{ moby_dick_plain, moby_dick_base64 },
	};

	for (size_t i = 0; i < sizeof(vec) / sizeof(vec[0]); i++) {

		// Encode plain string, check against output:
		fail |= assert_enc(vec[i].in, vec[i].out);

		// Decode the output string, check if we get the input:
		fail |= assert_dec(vec[i].out, vec[i].in);

		// Do a roundtrip on the inputs and the outputs:
		fail |= assert_roundtrip(vec[i].in);
		fail |= assert_roundtrip(vec[i].out);
	}

	fail |= test_char_table();
	fail |= test_streaming();

	if (!fail)
		puts("  all tests passed.");

	return fail;
}

int
main()
{
	bool fail = false;

	// Test this codec, merge the results:
    fail |= test_one_codec();

	return (fail) ? 1 : 0;
}
