#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "tdi.h"

extern "C" int extra(int in, int *out) {
    static int (*__extra)(int, int *) = NULL;

    if (__extra == NULL) {
        __extra = (int (*)(int, int *))dlsym(RTLD_NEXT, "extra");
        if (NULL == __extra) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    tditrace_ex("@T+extra()");
    int ret = __extra(in, out);
    tditrace_ex("@T-extra()");

    return ret;
}