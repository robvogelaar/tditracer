
/* Output info about the default buffering parameters
 * applied by libc to stdin, stdout and stderr.
 * Note the info is sent to stderr, as redirecting it
 * makes no difference to its buffering parameters.
 * Note gnulib has fbufmode() to make this portable.
 */
#include <stdio_ext.h>
#include <unistd.h>
#include <stdlib.h>

FILE* fileno2FILE(int fileno){
    switch(fileno) {
        case 0:  return stdin;
        case 1:  return stdout;
        case 2:  return stderr;
        default: return NULL;
    }
}

const char* fileno2name(int fileno){
    switch(fileno) {
        case 0:  return "stdin";
        case 1:  return "stdout";
        case 2:  return "stderr";
        default: return NULL;
    }
}

int main(void)
{
    if (isatty(0)) {
        fprintf(stderr,"Hit Ctrl-d to initialise stdin\n");
    } else {
        fprintf(stderr,"Initialising stdin\n");
    }
    char data[4096];
    fread(data,sizeof(data),1,stdin);
    if (isatty(1)) {
        fprintf(stdout,"Initialising stdout\n");
    } else {
        fprintf(stdout,"Initialising stdout\n");
        fprintf(stderr,"Initialising stdout\n");
    }
    fprintf(stderr,"Initialising stderr\n"); //redundant

    int i;
    for (i=0; i<3; i++) {
        fprintf(stderr,"%6s: tty=%d, lb=%d, size=%d\n",
                fileno2name(i),
                isatty(i),
                __flbf(fileno2FILE(i))?1:0,
                __fbufsize(fileno2FILE(i)));
    }
    return EXIT_SUCCESS;
}

