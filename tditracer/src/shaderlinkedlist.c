
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "shaderlinkedlist.h"

#define printf(...)

shader_struct_t *shaderlinkedlist_head = NULL;
shader_struct_t *shaderlinkedlist_curr = NULL;

shader_struct_t* shaderlinkedlist_create_list(int id, unsigned int name, unsigned int program, int count, char** string, int* length)
{
    printf("\n creating list with headnode as [%d]\n",id);
    shader_struct_t *ptr = (shader_struct_t*)malloc(sizeof(shader_struct_t));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->id = id;
    ptr->name = name;
    ptr->program = program;
    ptr->count = count;
    ptr->string = string;
    ptr->length = length;
    ptr->next = NULL;

    shaderlinkedlist_head = shaderlinkedlist_curr = ptr;
    return ptr;
}

shader_struct_t* shaderlinkedlist_add_to_list(int id, unsigned int name, unsigned int program, int count, char** string, int* length, bool add_to_end)
{
    if(NULL == shaderlinkedlist_head)
    {
        return (shaderlinkedlist_create_list(id, name, program, count, string, length));
    }

    if(add_to_end)
        printf("\n Adding node to end of list with id [%d]\n",id);
    else
        printf("\n Adding node to beginning of list with id [%d]\n",id);

    shader_struct_t *ptr = (shader_struct_t*)malloc(sizeof(shader_struct_t));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->id = id;
    ptr->name = name;
    ptr->program = program;
    ptr->count = count;
    ptr->string = string;
    ptr->length = length;
    ptr->next = NULL;

    if(add_to_end)
    {
        shaderlinkedlist_curr->next = ptr;
        shaderlinkedlist_curr = ptr;
    }
    else
    {
        ptr->next = shaderlinkedlist_head;
        shaderlinkedlist_head = ptr;
    }
    return ptr;
}

shader_struct_t* shaderlinkedlist_search_in_list(int id, shader_struct_t **prev)
{
    shader_struct_t *ptr = shaderlinkedlist_head;
    shader_struct_t *tmp = NULL;
    bool found = false;

    printf("\n Searching the list for id [%d] \n",id);

    while(ptr != NULL)
    {
        if(ptr->id == id)
        {
            found = true;
            break;
        }
        else
        {
            tmp = ptr;
            ptr = ptr->next;
        }
    }

    if(true == found)
    {
        if(prev)
            *prev = tmp;
        return ptr;
    }
    else
    {
        return NULL;
    }
}

int shaderlinkedlist_delete_from_list(int id)
{
    shader_struct_t *prev = NULL;
    shader_struct_t *del = NULL;

    printf("\n Deleting id [%d] from list\n",id);

    del = shaderlinkedlist_search_in_list(id,&prev);
    if(del == NULL)
    {
        return -1;
    }
    else
    {
        if(prev != NULL)
            prev->next = del->next;

        if(del == shaderlinkedlist_curr)
        {
            shaderlinkedlist_curr = prev;
        }
        else if(del == shaderlinkedlist_head)
        {
            shaderlinkedlist_head = del->next;
        }
    }

    free(del);
    del = NULL;

    return 0;
}

void shaderlinkedlist_print_list(void)
{
    shader_struct_t *ptr = shaderlinkedlist_head;

    printf("\n -------Printing list Start------- \n");
    while(ptr != NULL)
    {
        printf("\n [%d] \n",ptr->id);
        ptr = ptr->next;
    }
    printf("\n -------Printing list End------- \n");

    return;
}


#if 0
int shaderlinkedlist_test(void)
{
    int i = 0, ret = 0;
    shader_struct_t *ptr = NULL;

    print_list();

    for(i = 5; i<10; i++)
        add_to_list(i, 100, 100, 1, 0, true);

    print_list();

    for(i = 4; i>0; i--)
        add_to_list(i, 50, 50 , 1, 0, false);

    print_list();

    for(i = 1; i<10; i += 4)
    {
        ptr = shaderlinkedlist_search_in_list(i, NULL);
        if(NULL == ptr)
        {
            printf("\n Search [id = %d] failed, no such element found\n",i);
        }
        else
        {
            printf("\n Search passed [id = %d]\n",ptr->id);
        }

        print_list();

        ret = shaderlinkedlist_delete_from_list(i);
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