
extern "C" int tditrace_init(void);
extern "C" void tditrace(const char* format, ...);
extern "C" void tditrace_rewind(void);

#define TDITRACE(...)  tditrace(__VA_ARGS__)
#define TDIPRINTF(...)  fprintf(stdout, __VA_ARGS__)
