
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "texturelinkedlist.h"

#define verboseprintf(...)

texture_struct_t *texturelinkedlist_head = NULL;
texture_struct_t *texturelinkedlist_curr = NULL;

texture_struct_t* texturelinkedlist_create_list(int id, unsigned int name, bool subtexture, int frame, int xoffset, int yoffset, int width, int height, int format, void* png_data, size_t png_data_size)
{
    verboseprintf("\n creating list with headnode as [%d]\n",id);
    texture_struct_t *ptr = (texture_struct_t*)malloc(sizeof(texture_struct_t));
    if(NULL == ptr)
    {
        verboseprintf("\n Node creation failed \n");
        return NULL;
    }
    ptr->id = id;
    ptr->name = name;
    ptr->subtexture = subtexture;
    ptr->frame = frame;
    ptr->xoffset = xoffset;
    ptr->yoffset = yoffset;
    ptr->width = width;
    ptr->height = height;
    ptr->format = format;
    ptr->png_data = png_data;
    ptr->png_data_size = png_data_size;
    
    ptr->next = NULL;

    texturelinkedlist_head = texturelinkedlist_curr = ptr;
    return ptr;
}

texture_struct_t* texturelinkedlist_add_to_list(int id, unsigned int name, bool subtexture, int frame, int xoffset, int yoffset, int width, int height, int format, void* png_data, size_t png_data_size, bool add_to_end)
{
    if(NULL == texturelinkedlist_head)
    {
        return (texturelinkedlist_create_list(id, name, subtexture, frame, xoffset, yoffset, width, height, format, png_data, png_data_size));
    }

    if(add_to_end)
        verboseprintf("\n Adding node to end of list with id [%d]\n",id);
    else
        verboseprintf("\n Adding node to beginning of list with id [%d]\n",id);

    texture_struct_t *ptr = (texture_struct_t*)malloc(sizeof(texture_struct_t));
    if(NULL == ptr)
    {
        verboseprintf("\n Node creation failed \n");
        return NULL;
    }
    ptr->id = id;
    ptr->name = name;
    ptr->subtexture = subtexture;
    ptr->frame = frame;
    ptr->xoffset = xoffset;
    ptr->yoffset = yoffset;
    ptr->width = width;
    ptr->height = height;
    ptr->format = format;
    ptr->png_data = png_data;
    ptr->png_data_size = png_data_size;
    
    ptr->next = NULL;

    if(add_to_end)
    {
        texturelinkedlist_curr->next = ptr;
        texturelinkedlist_curr = ptr;
    }
    else
    {
        ptr->next = texturelinkedlist_head;
        texturelinkedlist_head = ptr;
    }
    return ptr;
}

texture_struct_t* texturelinkedlist_search_in_list(int id, texture_struct_t **prev)
{
    texture_struct_t *ptr = texturelinkedlist_head;
    texture_struct_t *tmp = NULL;
    bool found = false;

    verboseprintf("\n Searching the list for id [%d] \n",id);

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

int texturelinkedlist_delete_from_list(int id)
{
    texture_struct_t *prev = NULL;
    texture_struct_t *del = NULL;

    verboseprintf("\n Deleting id [%d] from list\n",id);

    del = texturelinkedlist_search_in_list(id, &prev);
    if (del == NULL)
    {
        return -1;
    }
    else
    {
        free(del->png_data);

        if (prev != NULL) {
            prev->next = del->next;
        }

        if(del == texturelinkedlist_curr)
        {
            texturelinkedlist_curr = prev;
            
            if(del == texturelinkedlist_head)
            {
                texturelinkedlist_head = del->next;
            }
            
        }
        else if(del == texturelinkedlist_head)
        {
            texturelinkedlist_head = del->next;
        }
    }

    free(del);

    return 0;
}

void texturelinkedlist_print_list(void)
{
    texture_struct_t *ptr = texturelinkedlist_head;

    verboseprintf("-------Printing list Start-------\n");
    while(ptr != NULL)
    {
        printf("[%d]\n", ptr->id);
        ptr = ptr->next;
    }
    verboseprintf("-------Printing list End-------\n");

    return;
}

#if 0
int texturelinkedlist_test(void)
{
    int i = 0, ret = 0;
    texture_struct_t *ptr = NULL;

    print_list();

    for(i = 5; i<10; i++)
        add_to_list(i, 100, 100, 1, 0, true);

    print_list();

    for(i = 4; i>0; i--)
        add_to_list(i, 50, 50 , 1, 0, false);

    print_list();

    for(i = 1; i<10; i += 4)
    {
        ptr = texturelinkedlist_search_in_list(i, NULL);
        if(NULL == ptr)
        {
            verboseprintf("\n Search [id = %d] failed, no such element found\n",i);
        }
        else
        {
            verboseprintf("\n Search passed [id = %d]\n",ptr->id);
        }

        print_list();

        ret = texturelinkedlist_delete_from_list(i);
        if(ret != 0)
        {
            verboseprintf("\n delete [id = %d] failed, no such element found\n",i);
        }
        else
        {
            verboseprintf("\n delete [id = %d]  passed \n",i);
        }

        print_list();
    }

    return 0;
}
#endif
