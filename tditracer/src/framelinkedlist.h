
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct frame_struct_t frame_struct_t;

struct frame_struct_t
{
    int id;
    int name;
    int page;
    void* buf;
    frame_struct_t *next;
} ;

extern frame_struct_t*  framelinkedlist_create_list(int id, int name, int page, void* buf);
extern frame_struct_t*  framelinkedlist_add_to_list(int id, int name, int page, void* buf, bool add_to_end);
extern frame_struct_t*  framelinkedlist_search_in_list(int id, struct frame_struct_t **prev);
extern int              framelinkedlist_delete_from_list(int id);
extern void             framelinkedlist_print_list(void);
extern int              framelinkedlist_test(void);
