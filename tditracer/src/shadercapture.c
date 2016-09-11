
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#define PNG_DEBUG 3
#include <png.h>

#include "gldefs.h"
#include "shaderlinkedlist.h"

int shadercapture_inited = 0;

int shadercapture_init(void);
void shadercapture_capshader(unsigned int name, int count, const char** string,
                             const int* length);
void shadercapture_referenceprogram(unsigned int shader, unsigned int program);
void shadercapture_writeshaders(void);

struct test_struct* shaderslist = NULL;

static int current_shader = 0;

int shadercapture_init(void) { return 0; }

void shadercapture_capshader(unsigned int name, int count, const char** string,
                             const int* length) {
  int i;

  if (!shadercapture_inited) {
    shadercapture_init();
    shadercapture_inited = 1;
  }

  /*
   * copy the array of strings
   * copy the array of string lengths
   */

  char** string_in = (char**)string;
  int* length_in = (int*)length;

  /*
   * always allocate space for the strings array
   */
  char** copiedstring = malloc(count * sizeof(char*));

  /*
   * if length == null, then no length array, copiedlength = length = null
   */
  int* copiedlength;
  if (length_in) {
    copiedlength = malloc(count * sizeof(int));
  } else {
    copiedlength = length_in;
  }

  char** string_out = copiedstring;
  int* length_out = copiedlength;

  // printf("capshader, %2d, %08d\n", current_shader, name);
  // printf("count = %d\n", count);

  for (i = 0; i < count; i++) {
    /*
     * if length == NULL, each string is null-terminated
     * else, each string is of length length unless length is < 0 then
     * string is null-terminated
     */
    if (length_in == NULL) {
      // printf("length_in == NULL\n");

      *copiedstring = malloc(strlen(*string_in));
      strcpy(*copiedstring, *string_in);
      // printf("%s\n", *copiedstring);

    } else {
      // printf("*length_in = %d\n", *length_in);

      if (*length_in < 0) {
        *copiedstring = malloc(strlen(*string_in));
        strcpy(*copiedstring, *string_in);
        // printf("%s\n", *copiedstring);
      } else {
        *copiedstring = malloc(*length_in);
        strncpy(*copiedstring, *string_in, *length_in);
        *copiedlength = *length_in;

        // int j;
        // for (j = 0 ; j < *copiedlength ; j++)
        // {
        //     printf("%c", (*copiedstring)[j]);
        // }
        // printf("\n");
      }

      length_in++;
      copiedlength++;
    }

    copiedstring++;
    string_in++;
  }

  int program = 0;

  shaderlinkedlist_add_to_list(current_shader, name, program, count, string_out,
                               length_out, true);

  current_shader++;
}

void shadercapture_writeshaders(void) {
  int shader;

  // printf("writing shaders...\n");

  for (shader = 0; shader < current_shader; shader++) {
    shader_struct_t* shader_ptr = NULL;

    shader_ptr = shaderlinkedlist_search_in_list(shader, NULL);
    if (NULL == shader_ptr) {
      printf("\n Search [id = %d] failed, no such element found\n", shader);
    } else {
      char fname[64];
      sprintf(fname, "s%02d-%08d-%08d.txt", shader_ptr->id, shader_ptr->name,
              shader_ptr->program);
      printf("writing shader: %s\n", fname);

      FILE* pFile;
      pFile = fopen(fname, "w");
      if (pFile == NULL) {
        printf("could not open %s\n", fname);

      } else {
        int i;

        char** string = shader_ptr->string;
        int* length = shader_ptr->length;

        // printf("count = %d\n", shader_ptr->count);

        for (i = 0; i < shader_ptr->count; i++) {
          /*
           * if length == NULL, each string is null-terminated
           * else, each string is of length length unless length is <
           * 0 then string is null-terminated
           */

          if (length == NULL) {
            // printf("length == NULL\n");
            fputs(*string, pFile);
          } else {
            // printf("*length = %d\n", *length);

            if (*length < 0) {
              fputs(*string, pFile);
            } else {
              int j;
              for (j = 0; j < *length; j++) {
                fputc((*string)[j], pFile);
              }
              fputc(13, pFile);
            }

            length++;
          }

          string++;
        }
        chmod(fname, 0666);
        fclose(pFile);
      }
    }
  }
}

void shadercapture_referenceprogram(unsigned int shader, unsigned int program) {
  /*
   * find the shader and add the program
   */

  int s;

  for (s = 0; s < current_shader; s++) {
    shader_struct_t* shader_ptr = NULL;

    shader_ptr = shaderlinkedlist_search_in_list(s, NULL);
    if (NULL == shader_ptr) {
      printf("\n Search [id = %d] failed, no such element found\n", s);
    } else {
      if (shader_ptr->name == shader) {
        shader_ptr->program = program;
      }
    }
  }
}
