
#define VERSION "0.1"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>


void usage(void)
{
    printf("tdidump v%s (%s %s)\n", VERSION, __DATE__, __TIME__);
    printf("\nUsage: tdidump <option>\n\n");
    printf("tdidump <filename> : convert tdi tracebuffer, and write to filename\n");
}


/******************************************************************************/
int main(int argc, char *argv[])
{
    FILE*   file;

    if (argc == 1) {
        usage();
        return 0;
    }

    if ((file = fopen("/tmp/.tditracebuffer", "r")) == 0) {
        printf("Not found: \"/tmp/.tditracebuffer\"\n");
        return -1;
    }


    struct stat st;
    stat("/tmp/.tditracebuffer", &st);

    //printf("filesize=%d\n", st.st_size);

    tditrace_exit(argv[1], (char *)mmap(0, st.st_size, PROT_READ, MAP_SHARED, fileno(file), 0));

    return 0;
}
