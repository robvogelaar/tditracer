
#define VERSION "0.1"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

/*
 **************************************
 */
#ifdef __cplusplus
extern "C" {
#endif
int  tditrace_init(void);
void tditrace(const char *format, ...);
#ifdef __cplusplus
}
#endif
int tditrace_inited = 0;
#define TDITRACE(...)               \
    do                              \
    {                               \
        if (!tditrace_inited) {     \
            tditrace_init();        \
            tditrace_inited = 1;    \
        }                           \
        tditrace(__VA_ARGS__);      \
    } while (0)                     \
/*
 **************************************
 */


static void usage(void);


/******************************************************************************/
int main(int argc, char *argv[])
{
    FILE*   file;

    if (argc == 1) {
        usage();
        exit(0);
    }

    if ((file = fopen("/tmp/.tditracebuffer", "r")) == 0) {
        printf("Error opening file /tmp/.tditracebuffer");
        return -1;
    }


    struct stat st;
    stat("/tmp/.tditracebuffer", &st);

    //printf("filesize=%d\n", st.st_size);

    char *map_base;

    map_base = (char *)mmap(0, st.st_size, PROT_READ, MAP_SHARED, fileno(file), 0);

    tditrace_exit(argv[1], map_base);

    return 0;

}

void usage(void)
{
    printf("tdidump v%s (%s %s)\n", VERSION, __DATE__, __TIME__);
    printf("\nUsage: tdidump <option>\n\n");
    printf("tdidump <filename> : convert tdi tracebuffer, and write to filename\n");
}
