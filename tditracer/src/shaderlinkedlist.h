
#include <stdio.h>
#include <stdlib.h>

typedef struct shader_struct_t shader_struct_t;

struct shader_struct_t
{
    int id;
    unsigned int name;
    unsigned int program;
    int count;
    char** string;
    int* length;
    shader_struct_t *next;
};

extern shader_struct_t* shaderlinkedlist_create_list(int id, unsigned int name, unsigned int program, int count, char** string, int* length);
extern shader_struct_t* shaderlinkedlist_add_to_list(int id, unsigned int name, unsigned int program, int count, char** string, int* length, bool add_to_end);
extern shader_struct_t* shaderlinkedlist_search_in_list(int id, struct shader_struct_t **prev);
extern int              shaderlinkedlist_delete_from_list(int id);
extern void             shaderlinkedlist_print_list(void);
extern int              shaderlinkedlist_test(void);
