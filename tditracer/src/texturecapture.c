
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <malloc.h>
#include <fcntl.h>
#define PNG_DEBUG 3
#include <png.h>


extern void write_png_file(char* file_name, int width, int height, png_bytep *row_pointers);

#include "gldefs.h"
#include "texturelinkedlist.h"

void* memcpy_arm(void*, const void*, size_t);
void* memcpy_neon(void*, const void*, size_t);

int  texturecapture_inited = 0;

int  texturecapture_init(void);
void texturecapture_captexture(unsigned int name, bool subtexture, int frame, int xoffset, int yoffset, int width, int height, int format, int type, void* pixels);
void texturecapture_writepngframes(int textures);

static int nr_textures = 0;

int texturecapture_init(void)
{
    return 0;
}


void texturecapture_captexture(unsigned int name, bool subtexture, int frame, int xoffset, int yoffset, int width, int height, int format, int type, void* pixels)
{
    if (!texturecapture_inited) {
        texturecapture_init();
        texturecapture_inited = 1;
    }

    int texturesize = (format == GL_ALPHA) ? ((width + 3) & ~3) * height : width * height * 4;

    uint8_t* pixelbuf = (uint8_t *)memalign(4, texturesize);

    if (!pixels) {
        /*
         * in case no pixelbuf is provided, create an empty one
         * this is typically used as a placeholder for subsequent subtextures
         */

        //printf("t%05d-frame%03d-%s-%dx%d-%d = empty\n", nr_textures, frame, FORMATSTRING(format), width, height, name);

        memset(pixelbuf, 0x0, texturesize);
    } else {
        memcpy_arm(pixelbuf, pixels, texturesize);
    }

    texturelinkedlist_add_to_list(nr_textures, name, subtexture, frame, xoffset, yoffset, width, height, format, pixelbuf, true);

    nr_textures++;
}


void texturecapture_writepngtextures(void)
{
    int frame;
    int texture;

    /*
     * on a per frame basis find all subtextures and add these into it's parent textures
     */

    printf("populating parent-textures from sub-textures...\n");

    for (frame = 0 ; frame < 100 ; frame++) {

        texture_struct_t *texture_ptr = NULL;

        for (texture = 0 ; texture < nr_textures ; texture++)
        {

            texture_ptr = texturelinkedlist_search_in_list(texture, NULL);
            if (texture_ptr) {

                if ((texture_ptr->frame == frame) && (texture_ptr->subtexture)) {

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
                        if ((!parent_ptr->subtexture) && (parent_ptr->name == sub_ptr->name))
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
                            void* d = parent_ptr->pixels;
                            void* s = sub_ptr->pixels;

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
                sprintf(fname, "t%05d-frame%03d-%s-%s-%dx%d-%d.png", texture_ptr->id, texture_ptr->frame, texture_ptr->subtexture?"sub":"parent", FORMATSTRING(texture_ptr->format), texture_ptr->width, texture_ptr->height, texture_ptr->name);

                printf("writing texture: %s\n", fname);

                int x,y;

                png_bytep *row_pointers;
                row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * texture_ptr->height);

                for (y = 0 ; y < texture_ptr->height ; y++)
                {
                    row_pointers[y] = (png_byte*)malloc(texture_ptr->width * 4);
                }

                for (y = 0 ; y < texture_ptr->height ; y++)
                {
                    png_byte* row = row_pointers[y];
                    for (x = 0; x < texture_ptr->width; x++)
                    {
                        png_byte* ptr = &(row[x * 4]);

                        uint8_t* p = (uint8_t*)texture_ptr->pixels;

                        if ((texture_ptr->format == GL_RGBA) || (texture_ptr->format == GL_BGRA_EXT)) {

                            ptr[0] = p[ (y * texture_ptr->width + x) * 4 + 2 ]; /* R */
                            ptr[1] = p[ (y * texture_ptr->width + x) * 4 + 1 ]; /* G */
                            ptr[2] = p[ (y * texture_ptr->width + x) * 4 + 0 ]; /* B */
                            ptr[3] = p[ (y * texture_ptr->width + x) * 4 + 3 ]; /* A */

                        } else if ((texture_ptr->format == GL_ALPHA)) {

                            ptr[0] = 0x00; /* R */
                            ptr[1] = 0x00; /* G */
                            ptr[2] = 0xff; /* B */
                            ptr[3] = p[ (y * ((texture_ptr->width + 3) & ~3) + x) ]; /* A */

                        }
                    }
                }

                write_png_file(fname, texture_ptr->width, texture_ptr->height, row_pointers);
                /*
                 * write_png_file frees row_pointers
                 */

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
