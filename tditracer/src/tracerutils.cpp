#if 0
const char* demangle(const char* s)
{
    if (s) {
        if (strstr(s, "_ZN")) {
            if (dlsym(RTLD_NEXT, "__cxa_demangle")) {
                size_t length = 0;
                int status = 0;
                return abi::__cxa_demangle(s, NULL, &length, &status);
            }
        }
    }

    return s;
}

char* addrinfo(void* addr)
{
    static char text[256];
    Dl_info dli;

    dladdr(addr, &dli);
    sprintf(text, "[0x%08x] %s (%s)", (int)addr, demangle(dli.dli_sname), dli.dli_fname);
    return text;
}


//extern "C" pthread_t pthread_self(void);

int pthreadid(pthread_t ptid)
{
    int threadId = 0;
    memcpy(&threadId, &ptid, 4);
    return threadId;
}

#define MAXFRAMES 32

static inline void print_stacktrace(int max_frames = MAXFRAMES)
{
    // storage array for stack trace address data
    void* addrlist[max_frames + 1];

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

    if (addrlen == 0) {
        printf("    <empty, possibly corrupt>\n");
        return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char** symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.

    // also skip 1 as it is in libtditracer.so
    for (int i = 2; i < addrlen; i++)
    {
        char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

        // find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = symbollist[i]; *p; ++p)
        {
            if (*p == '(')
                begin_name = p;
            else if (*p == '+')
                begin_offset = p;
            else if (*p == ')' && begin_offset) {
                end_offset = p;
                break;
            }
        }

        if (begin_name && begin_offset && end_offset && begin_name < begin_offset)
        {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():

            int status;
            char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
            if (status == 0) {
                funcname = ret; // use possibly realloc()-ed string
                printf("    [%ld][%2d][0x%08x]%s : %s+%s\n", syscall(SYS_gettid), i, (int)addrlist[i], symbollist[i], funcname, begin_offset);
            }
            else {
                // demangling failed. Output function name as a C function with no arguments.
                printf("    [%ld][%2d][0x%08x]%s : %s()+%s\n", syscall(SYS_gettid), i, (int)addrlist[i], symbollist[i], begin_name, begin_offset);
            }
        }
        else
        {
            // couldn't parse the line? print the whole line.
            printf("    [%ld][%2d][0x%08x],%s\n", syscall(SYS_gettid), i, (int)addrlist[i], symbollist[i]);
        }
    }

    free(funcname);
    free(symbollist);
}
#endif
