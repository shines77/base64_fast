
#define _POSIX_C_SOURCE 199309L		// for clock_gettime()

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "../src/base64_fast.h"

#define KB	        (1024)
#define MB	        (1024 * KB)
#define GB	        (1024 * MB)

#define RANDOMDEV   "/dev/urandom"

struct buffers {
	char *reg;
	char *enc;
	size_t regsz;
	size_t encsz;
};

struct bufsize {
	char   *label;
	size_t	len;
	int	    repeat;
	int	    batch;
};

typedef struct bufsize bufsize_t;

// Define buffer sizes to test with:
static bufsize_t sizes[] = {
	{ "10 MB",	MB * 10,	10,	    1	    },
	{ "1 MB",	MB * 1,		10,	    10	    },
	{ "100 KB",	KB * 100,	10,	    100	    },
	{ "10 KB",	KB * 10,	100,	100	    },
	{ "1 KB",	KB * 1,		100,	1000    },
};

static const uint32_t zoom_times = 1;

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS)
#ifndef inline
#define inline  __inline
#endif
#endif // _WIN32

static inline double
bytes_to_mb(size_t bytes)
{
	return (double)bytes * zoom_times / (double) MB;
}

static inline double
bytes_to_gb(size_t bytes)
{
	return (double)bytes * zoom_times / (double) GB;
}

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS)
static bool
get_random_data(struct buffers *b, char **errmsg)
{
	size_t total_read = 0;

    srand((unsigned)time(NULL));

	printf("Filling buffer with %.1f MB of random data...\n", bytes_to_mb(b->regsz));

    unsigned char * cur = b->reg;
	while (total_read < b->regsz) {
        *cur++ = (unsigned char)(rand() & 0xFFU);
		total_read++;
	}
	return true;
}
#else
static bool
get_random_data(struct buffers *b, char **errmsg)
{
	int fd;
	ssize_t nread;
	size_t total_read = 0;

	// Open random device for semi-random data:
	if ((fd = open(RANDOMDEV, O_RDONLY)) < 0) {
		*errmsg = "Cannot open " RANDOMDEV;
		return false;
	}

	printf("Filling buffer with %.1f MB of random data...\n", bytes_to_mb(b->regsz));

	while (total_read < b->regsz) {
		if ((nread = read(fd, b->reg + total_read, b->regsz - total_read)) < 0) {
			*errmsg = "Read error";
			close(fd);
			return false;
		}
		total_read += nread;
	}
	close(fd);
	return true;
}
#endif // _WIN32

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS)

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME  1
#endif

//
// See: http://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows
//

static LARGE_INTEGER
getFILETIMEoffset()
{
    SYSTEMTIME s;
    FILETIME f;
    LARGE_INTEGER t;

    s.wYear = 1970;
    s.wMonth = 1;
    s.wDay = 1;
    s.wHour = 0;
    s.wMinute = 0;
    s.wSecond = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = ((uint64_t)f.dwHighDateTime << 32) | (uint64_t)f.dwLowDateTime;
    return (t);
}

static int
clock_gettime(int flag, struct timespec *tv)
{
    LARGE_INTEGER           t;
    FILETIME                f;
    double                  nanoSeconds;
    static LARGE_INTEGER    offset;
    static double           frequencyToNanoseconds;
    static int              initialized = 0;
    static BOOL             usePerformanceCounter = FALSE;

    if (!initialized) {
        LARGE_INTEGER performanceFrequency;
        initialized = 1;
        usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
        if (usePerformanceCounter) {
            QueryPerformanceCounter(&offset);
            frequencyToNanoseconds = (double)performanceFrequency.QuadPart / 1e+9;
        } else {
            offset = getFILETIMEoffset();
            // GetSystemTimeAsFileTime()'s precision is 0.1 us (= 0.0001 ms, and = 100 ns).
            frequencyToNanoseconds = 1.0 / 100.0;
        }
    }
    if (usePerformanceCounter) {
        QueryPerformanceCounter(&t);
    }
    else {
        // The FILETIME structure is a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601.
        GetSystemTimeAsFileTime(&f);
        t.QuadPart = ((uint64_t)f.dwHighDateTime << 32) | (uint64_t)f.dwLowDateTime;
    }

    t.QuadPart -= offset.QuadPart;
    nanoSeconds = (double)t.QuadPart / frequencyToNanoseconds;
    t.QuadPart = (LONGLONG)nanoSeconds;
    tv->tv_sec = (time_t)(t.QuadPart / 1000000000);
    tv->tv_nsec = (long)(t.QuadPart % 1000000000);
    return (0);
}
#endif // _WIN32

static double
timediff_sec(struct timespec *start, struct timespec *end)
{
	return (end->tv_sec - start->tv_sec) + ((double)(end->tv_nsec - start->tv_nsec)) / 1e+9;
}

static void
codec_bench_enc(struct buffers *b, const struct bufsize *bs, const char *name)
{
	double timediff, fastest = -1.0;
	struct timespec start, end;

	// Reset buffer size:
	b->regsz = bs->len;

	// Repeat benchmark a number of times for a fair test:
	for (int i = bs->repeat; i; i--) {

		// Timing loop, use batches to increase timer resolution:
		clock_gettime(CLOCK_REALTIME, &start);
		for (int j = bs->batch * zoom_times; j; j--)
			base64_encode_fast((const char *)b->reg, b->regsz, b->enc, b->encsz);
		clock_gettime(CLOCK_REALTIME, &end);

		// Calculate average time of batch:
		timediff = timediff_sec(&start, &end) / bs->batch;

		// Update fastest time seen:
		if (fastest < 0.0 || timediff < fastest)
			fastest = timediff;
	}

	printf("%s\tencode\t%.02f MB/sec, fastest\t%0.3f ms\n", name,
        bytes_to_mb(b->regsz) / fastest, fastest * 1000.0);
}

static void
codec_bench_dec(struct buffers *b, const struct bufsize *bs, const char *name)
{
	double timediff, fastest = -1.0;
	struct timespec start, end;

	// Reset buffer size:
	b->encsz = bs->len;

	// Repeat benchmark a number of times for a fair test:
	for (int i = bs->repeat; i; i--) {

		// Timing loop, use batches to increase timer resolution:
		clock_gettime(CLOCK_REALTIME, &start);
		for (int j = bs->batch * zoom_times; j; j--)
			base64_decode_fast((const char *)b->enc, b->encsz, b->reg, b->regsz);
		clock_gettime(CLOCK_REALTIME, &end);

		// Calculate average time of batch:
		timediff = timediff_sec(&start, &end) / bs->batch;

		// Update fastest time seen:
		if (fastest < 0.0 || timediff < fastest)
			fastest = timediff;
	}

	printf("%s\tdecode\t%.02f MB/sec, fastest\t%0.3f ms\n", name,
        bytes_to_mb(b->encsz) / fastest, fastest * 1000.0);
}

static void
codec_bench(struct buffers *b, const struct bufsize *bs)
{
	codec_bench_enc(b, bs, "plain");
	codec_bench_dec(b, bs, "plain");
}

int main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

	int ret = 0;
	char *errmsg = NULL;
	struct buffers b;

	// Set buffer sizes to largest buffer length:
	b.regsz = sizes[0].len;
	b.encsz = sizes[0].len * 5 / 3;

	// Allocate space for megabytes of random data:
	if ((b.reg = (char *)malloc(b.regsz)) == NULL) {
		errmsg = "Out of memory";
		ret = 1;
		goto err0;
	}

	// Allocate space for encoded output:
	if ((b.enc = (char *)malloc(b.encsz)) == NULL) {
		errmsg = "Out of memory";
		ret = 1;
		goto err1;
	}

	// Fill buffer with random data:
	if (get_random_data(&b, &errmsg) == false) {
		ret = 1;
		goto err2;
	}

	// Loop over all buffer sizes:
	for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
		printf("Testing with buffer size %s, fastest of %d * %d\n",
			sizes[i].label, sizes[i].repeat, sizes[i].batch);

        codec_bench(&b, &sizes[i]);
	};

	// Free memory:
err2:
    free(b.enc);
err1:
    free(b.reg);
err0:
    if (errmsg)
		fputs(errmsg, stderr);

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS)
    system("pause");
#endif
	return ret;
}
