#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "tdi.h"

#if 0
extern "C" int extra(int in, int *out) {

    static int (*__extra)(int, int *) = NULL;

    if (__extra == NULL) {
        __extra = (int (*)(int, int *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __extra) {
            fprintf(stderr, "Error in `dlsym`: %s %s\n", __func__, dlerror());
        }
    }

    tditrace("@T+extra()");
    int ret = __extra(in, out);
    tditrace("@T-extra()");

    return ret;
}

#endif
