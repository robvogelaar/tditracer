
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <malloc.h>
#include <fcntl.h>

#include "texturecapture.h"

#define MINIZ_NO_STDIO
#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_TIME
#define MINIZ_NO_ZLIB_APIS
#include "miniz.c"

#include "gldefs.h"
#include "texturelinkedlist.h"


static int nr_textures = 0;

typedef unsigned char uint8;

void texturecapture_captexture(unsigned int name, int ttype, int frame, int xoffset, int yoffset, int width, int height, int format, int type, const void* pixels)
{
    void *pPNG_data;

    size_t png_data_size = 0;

    if (pixels) {

        if (format == GL_ALPHA) {
            pPNG_data = tdefl_write_image_to_png_file_in_memory_ex(pixels, ((width + 3) & ~3), height, 1, &png_data_size, 6, ttype == RENDER);
        } else {
            pPNG_data = tdefl_write_image_to_png_file_in_memory_ex(pixels, width, height, 4, &png_data_size, 6, ttype == RENDER);
        }

        if (!pPNG_data) {
            fprintf(stderr, "tdefl_write_image_to_png_file_in_memory_ex() failed!\n");
        }

        #if 0
        printf("datasize=[%dx%d][%s]=%d(%d)\n", width, height, (format == GL_ALPHA) ? "alpha" : "argb",
                  (format == GL_ALPHA) ? ((width + 3) & ~3) * height : width * height * 4, png_data_size);
        #endif

    } else {
    
        //printf("pixels=NULL\n");

    }

    texturelinkedlist_add_to_list(nr_textures, name, ttype, frame, xoffset, yoffset, width, height, format, pPNG_data, png_data_size, true);

    nr_textures++;
}


void texturecapture_writepngtextures(void)
{
    int frame;
    int texture;

    /*
     * on a per frame basis find all subtextures and add these into it's parent textures
     */

    #if 0
    printf("populating parent-textures from sub-textures...\n");

    for (frame = 0 ; frame < 100 ; frame++) {

        texture_struct_t *texture_ptr = NULL;

        for (texture = 0 ; texture < nr_textures ; texture++)
        {

            texture_ptr = texturelinkedlist_search_in_list(texture, NULL);
            if (texture_ptr) {

                if ((texture_ptr->frame == frame) && (texture_ptr->ttype == SUB)) {

                    texture_struct_t *sub_ptr = NULL;
                    texture_struct_t *parent_ptr = NULL;

                    sub_ptr = texture_ptr;

                    /*
                     * search parent (non sub) texture with the same name, across frame boundaries
                     * parent textures can be used (sub textured) multiple times across frame boundaries
                     */
                    int t;
                    for (t = 0 ; t < nr_textures ; t++) {
                        parent_ptr = texturelinkedlist_search_in_list(t, NULL);
                        if ((parent_ptr->ttype == PARENT) && (parent_ptr->name == sub_ptr->name))
                            break;
                    }

                    if (t == nr_textures) {
                        printf("frame:%3d, sub : (%5d,%8d), parent not found\n", sub_ptr->frame, sub_ptr->id, sub_ptr->name);
                    } else {

                        /*
                         * copy the subtexture into its parent at offset x,y
                         */

                        //printf("frame:%3d, sub : (%5d,%8d)[%4d x %4d + %4d + %4d], into parent : (%5d,%8d)[%4d x %4d]\n",
                        //    sub_ptr->frame,
                        //    sub_ptr->id,
                        //    sub_ptr->name,
                        //    sub_ptr->width,
                        //    sub_ptr->height,
                        //    sub_ptr->xoffset,
                        //    sub_ptr->yoffset,
                        //    parent_ptr->id,
                        //    parent_ptr->name,
                        //    parent_ptr->width,
                        //    parent_ptr->height);

                        /*
                         * will it fit?
                         */

                        if (((sub_ptr->xoffset + sub_ptr->width) > parent_ptr->width) ||
                            ((sub_ptr->yoffset + sub_ptr->height) > parent_ptr->height)) {

                            printf("frame:%3d, sub : (%5d,%8d), will not fit\n", sub_ptr->frame, sub_ptr->id, sub_ptr->name);

                        } else if (sub_ptr->format != parent_ptr->format) {

                            printf("frame:%3d, sub : (%5d,%8d), parent format does not match sub format\n", sub_ptr->frame, sub_ptr->id, sub_ptr->name);

                        } else {

                            /*
                             * Copy in
                             */
                            void* d = parent_ptr->png_data;
                            void* s = sub_ptr->png_data;

                            int y;
                            for (y = 0 ; y < sub_ptr->height ; y++ ) {

                                if (sub_ptr->format == GL_ALPHA) {
                                    memcpy(d + ((sub_ptr->yoffset + y) * ((parent_ptr->width + 3) & ~3)) + sub_ptr->xoffset,
                                           s + y * ((sub_ptr->width + 3) & ~3),
                                           sub_ptr->width);
                                } else {
                                    memcpy(d + ((sub_ptr->yoffset + y) * (parent_ptr->width * 4)) + (sub_ptr->xoffset * 4),
                                           s + y * (sub_ptr->width * 4),
                                           sub_ptr->width * 4);
                                }

                            }

                        }

                    }

                }

            }

        }

    }

    #endif

    //printf("writing textures...\n");

    for (texture = 0 ; texture < nr_textures ; texture++)
    {
        texture_struct_t *texture_ptr = NULL;

        texture_ptr = texturelinkedlist_search_in_list(texture, NULL);
        if (NULL == texture_ptr)
        {
            printf("\n Search [id = %d] failed, no such element found\n", texture);
        } else {

            /*
             * Do not write subtextures
             */

            //printf("t%05d-frame%03d-%s-%dx%d-%d\n", texture_ptr->id, texture_ptr->frame, FORMATSTRING(texture_ptr->format), texture_ptr->width, texture_ptr->height, texture_ptr->name);

            //if (!texture_ptr->subtexture) {

                char fname[64];
                sprintf(fname, "t%u-%05d-f%03d-%s-%d-%dx%d+%d+%d.png", texture_ptr->name, texture_ptr->id, texture_ptr->frame,
                    texture_ptr->ttype==PARENT?"p":texture_ptr->ttype==SUB?"s":"r",
                    FORMATSIZE(texture_ptr->format), texture_ptr->width, texture_ptr->height,
                    texture_ptr->xoffset, texture_ptr->yoffset);

                printf("writing texture: %s\n", fname);

                FILE *pFile = fopen(fname, "wb");
                fwrite(texture_ptr->png_data, 1, texture_ptr->png_data_size, pFile);
                chmod(fname, 0666);
                fclose(pFile);

            // }

        }

    }

}


void texturecapture_deletetextures(void)
{
    int i;
    for (i = 0 ; i < 1000 ; i++) {
        texturelinkedlist_delete_from_list(i);
    }
}
