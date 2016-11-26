
#ifdef __cplusplus
extern "C" {
#endif

int tditrace_init(void);
void tditrace(const char* format, ...);
void tditrace_ex(const char* format, ...);
void tditrace_rewind(void);
int tditrace_exit(void);

#ifdef __cplusplus
}
#endif
