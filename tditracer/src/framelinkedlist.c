
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "framelinkedlist.h"

#define printf(...)

frame_struct_t *framelinkedlist_head = NULL;
frame_struct_t *framelinkedlist_curr = NULL;

frame_struct_t *framelinkedlist_create_list(int id, int name, void *buf,
                                            int size) {
    printf("\n creating list with headnode as [%d]\n", id);
    frame_struct_t *ptr = (frame_struct_t *)malloc(sizeof(frame_struct_t));
    if (NULL == ptr) {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->id = id;
    ptr->name = name;
    ptr->buf = buf;
    ptr->size = size;
    ptr->next = NULL;

    framelinkedlist_head = framelinkedlist_curr = ptr;
    return ptr;
}

frame_struct_t *framelinkedlist_add_to_list(int id, int name, void *buf,
                                            int size, bool add_to_end) {
    if (NULL == framelinkedlist_head) {
        return (framelinkedlist_create_list(id, name, buf, size));
    }

    if (add_to_end)
        printf("\n Adding node to end of list with id [%d]\n", id);
    else
        printf("\n Adding node to beginning of list with id [%d]\n", id);

    frame_struct_t *ptr = (frame_struct_t *)malloc(sizeof(frame_struct_t));
    if (NULL == ptr) {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->id = id;
    ptr->name = name;
    ptr->buf = buf;
    ptr->size = size;
    ptr->next = NULL;

    if (add_to_end) {
        framelinkedlist_curr->next = ptr;
        framelinkedlist_curr = ptr;
    } else {
        ptr->next = framelinkedlist_head;
        framelinkedlist_head = ptr;
    }
    return ptr;
}

frame_struct_t *framelinkedlist_search_in_list(int id, frame_struct_t **prev) {
    frame_struct_t *ptr = framelinkedlist_head;
    frame_struct_t *tmp = NULL;
    bool found = false;

    printf("\n Searching the list for id [%d] \n", id);

    while (ptr != NULL) {
        if (ptr->id == id) {
            found = true;
            break;
        } else {
            tmp = ptr;
            ptr = ptr->next;
        }
    }

    if (true == found) {
        if (prev)
            *prev = tmp;
        return ptr;
    } else {
        return NULL;
    }
}

int framelinkedlist_delete_from_list(int id) {
    frame_struct_t *prev = NULL;
    frame_struct_t *del = NULL;

    printf("\n Deleting id [%d] from list\n", id);

    del = framelinkedlist_search_in_list(id, &prev);
    if (del == NULL) {
        return -1;
    } else {

        if (del->buf) {
            free(del->buf);
        }

        if (prev != NULL)
            prev->next = del->next;

        if (del == framelinkedlist_curr) {
            framelinkedlist_curr = prev;
        } else if (del == framelinkedlist_head) {
            framelinkedlist_head = del->next;
        }
    }

    free(del);
    del = NULL;

    return 0;
}

void framelinkedlist_print_list(void) {
    frame_struct_t *ptr = framelinkedlist_head;

    printf("\n -------Printing list Start------- \n");
    while (ptr != NULL) {
        printf("\n [%d] \n", ptr->id);
        ptr = ptr->next;
    }
    printf("\n -------Printing list End------- \n");

    return;
}

#if 0
int framelinkedlist_test(void)
{
    int i = 0, ret = 0;
    frame_struct_t *ptr = NULL;

    print_list();

    for(i = 5; i<10; i++)
        add_to_list(i, 100, 100, 1, 0, true);

    print_list();

    for(i = 4; i>0; i--)
        add_to_list(i, 50, 50 , 1, 0, false);

    print_list();

    for(i = 1; i<10; i += 4)
    {
        ptr = framelinkedlist_search_in_list(i, NULL);
        if(NULL == ptr)
        {
            printf("\n Search [id = %d] failed, no such element found\n",i);
        }
        else
        {
            printf("\n Search passed [id = %d]\n",ptr->id);
        }

        print_list();

        ret = framelinkedlist_delete_from_list(i);
        if(ret != 0)
        {
            printf("\n delete [id = %d] failed, no such element found\n",i);
        }
        else
        {
            printf("\n delete [id = %d]  passed \n",i);
        }

        print_list();
    }

    return 0;
}
#endif