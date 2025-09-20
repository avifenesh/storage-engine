#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

static double now_ms(void) { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return ts.tv_sec*1000.0 + ts.tv_nsec/1e6; }

int main(int argc, char **argv)
{
    const char *file;
    size_t nbytes;
    double t0, t1;
    int fd;
    size_t left, chunk;
    char *buf;
    void *map;
    unsigned char *p;
    volatile unsigned long sum;
    size_t i;
    size_t align;
    void *dbuf;

    file = (argc > 1) ? argv[1] : "io_profile.tmp";
    nbytes = (argc > 2) ? strtoull(argv[2], NULL, 10) : (64ull<<20);

    // Buffered write
    t0 = now_ms();
    fd = open(file, O_CREAT|O_TRUNC|O_WRONLY, 0644);
    if (fd < 0) { perror("open"); return 1; }
    left = nbytes; chunk = 1<<20; // 1MB
    buf = (char*)malloc(chunk); memset(buf, 7, chunk);
    while (left) { size_t w = left < chunk ? left : chunk; if (write(fd, buf, w) != (ssize_t)w) { perror("write"); break; } left -= w; }
    fsync(fd); close(fd); t1 = now_ms();
    printf("Buffered write: %.2f MB/s\n", nbytes/1e6/((t1-t0)/1000.0));

    // mmap read
    fd = open(file, O_RDONLY); if (fd < 0) { perror("open r"); return 1; }
    t0 = now_ms();
    map = mmap(NULL, nbytes, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map != MAP_FAILED) { sum = 0; p = (unsigned char*)map; for (i=0;i<nbytes;i+=4096) sum += p[i]; munmap(map, nbytes); }
    close(fd); t1 = now_ms();
    printf("mmap read:     %.2f MB/s\n", nbytes/1e6/((t1-t0)/1000.0));

    // Direct I/O write (best-effort; may fail if FS not aligned)
    /* Optional O_DIRECT test; compile-time optional */
#ifdef O_DIRECT
    fd = open("io_profile_direct.tmp", O_CREAT|O_TRUNC|O_WRONLY|O_DIRECT, 0644);
    if (fd >= 0) {
        // Require alignment; we’ll skip if posix_memalign fails.
        align = 4096; dbuf = NULL; if (posix_memalign(&dbuf, align, chunk) == 0) {
            memset(dbuf, 3, chunk); left = nbytes; t0 = now_ms();
            while (left) { size_t w = left < chunk ? left : chunk; if (write(fd, dbuf, w) != (ssize_t)w) break; left -= w; }
            fsync(fd); t1 = now_ms(); free(dbuf);
            printf("O_DIRECT write: %.2f MB/s\n", nbytes/1e6/((t1-t0)/1000.0));
        }
        close(fd);
    } else { printf("O_DIRECT write: open failed\n"); }
#else
    printf("O_DIRECT write: not supported here\n");
#endif

    free(buf);
    return 0;
}
