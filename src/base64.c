
#include <stddef.h>	// size_t
#include <stdio.h>	// fopen()
#include <string.h>	// strlen()
#include <stdint.h>
#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS)
#else
#include <getopt.h>
#endif
#include "base64_fast.h"

#define BUFSIZE 1024 * 1024

static char buf[BUFSIZE];
static char out[((BUFSIZE + 2) / 3) * 4 + 1];	// Technically 4/3 of input, but take some margin
ssize_t nread;
ssize_t nout;

static int
base64_enc(FILE *fp)
{
	int ret = 1;
    size_t out_len = sizeof(out) - 1;
	while ((nread = fread(buf, 1, BUFSIZE, fp)) > 0) {
		nout = base64_encode(buf, nread, out, out_len);
		if (nout > 0) {
			fwrite(out, nout, 1, stdout);
		}
		if (feof(fp)) {
			break;
		}
	}
	if (ferror(fp)) {
		fprintf(stderr, "read error\n");
		ret = 0;
		goto enc_out;
	}
enc_out:
    fclose(fp);
	fclose(stdout);
	return ret;
}

static int
base64_dec(FILE *fp)
{
	int ret = 1;
    size_t out_len = sizeof(out) - 1;
	while ((nread = fread(buf, 1, BUFSIZE, fp)) > 0) {
		if ((nout = base64_decode(buf, nread, out, out_len)) < 0) {
			fprintf(stderr, "decoding error\n");
			ret = 0;
			goto dec_out;
		}
		if (nout) {
			fwrite(out, nout, 1, stdout);
		}
		if (feof(fp)) {
			break;
		}
	}
	if (ferror(fp)) {
		fprintf(stderr, "read error\n");
		ret = 0;
	}
dec_out:
    fclose(fp);
	fclose(stdout);
	return ret;
}

int
main(int argc, char * argv[])
{
	char *file;
	FILE *fp;
	int decode = 0;

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS)
    errno_t err_no;
    int optind = argc - 1;
    for (int i = 1; i < argc; ++i) {
        if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "-decode") == 0)) {
            decode = 1;
        }
    }
#else
	// Parse options:
	for (;;)
	{
		int c;
		int opt_index = 0;
		static struct option opt_long[] = {
			{ "decode", 0, 0, 'd' },
			{ 0, 0, 0, 0 }
		};
		if ((c = getopt_long(argc, argv, "d", opt_long, &opt_index)) == -1) {
			break;
		}
		switch (c)
		{
			case 'd':
				decode = 1;
				break;
		}
	}
#endif

	// No options left on command line? Read from stdin:
	if (optind >= argc) {
		fp = stdin;
	}

	// One option left on command line? Treat it as a file:
	else if (optind + 1 == argc) {
		file = argv[optind];
		if (strcmp(file, "-") == 0) {
			fp = stdin;
		}
#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS)
		else if ((err_no = fopen_s(&fp, file, "rb")) != 0) {
            if (fp == NULL) {
			    printf("cannot open %s\n", file);
			    return 1;
            }
        }
#else
        else if ((fp = fopen(file, "rb")) == NULL) {
			printf("cannot open %s\n", file);
			return 1;
		}
#endif
	}

	// More than one option left on command line? Syntax error:
	else {
		printf("Usage: %s <file>\n", argv[0]);
		return 1;
	}

	// Invert return codes to create shell return code:
	int success = (decode) ? !base64_dec(fp) : !base64_enc(fp);
    return success;
}
