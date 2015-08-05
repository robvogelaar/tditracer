
#define VERSION "0.1"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

void usage(void) {
    printf("tdidump v%s (%s %s)\n", VERSION, __DATE__, __TIME__);
    printf("\nUsage: tdidump [tracebuffer]\n\n");
    printf("tdidump : convert tditracebuffer(s) to tdi.\n");
}

/******************************************************************************/
int main(int argc, char *argv[]) {

    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        usage();
        return 0;
    }

    tditrace_exit(argc, argv);

    return 0;
}
