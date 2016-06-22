
extern "C" int tditrace_init(void);
extern "C" void tditrace(const char* format, ...);
extern "C" void tditrace_ex(const char* format, ...);
extern "C" void tditrace_rewind(void);
extern "C" int tditrace_exit(void);
