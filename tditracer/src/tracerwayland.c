#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

#include "tdi.h"


typedef int (*wl_dispatcher_func_t)(const void *, void *, uint32_t,
                                    const struct wl_message *,
                                    union wl_argument *);

struct wl_array {
  size_t size;
  size_t alloc;
  void *data;
};

struct wl_object {
  const struct wl_interface *interface;
  const void *implementation;
  uint32_t id;
};

struct wl_list {
  struct wl_list *prev;
  struct wl_list *next;
};

struct wl_map {
  struct wl_array client_entries;
  struct wl_array server_entries;
  uint32_t side;
  uint32_t free_list;
};

enum wl_proxy_flag {
  WL_PROXY_FLAG_ID_DELETED = (1 << 0),
  WL_PROXY_FLAG_DESTROYED = (1 << 1)
};

struct wl_proxy {
  struct wl_object object;
  struct wl_display *display;
  struct wl_event_queue *queue;
  uint32_t flags;
  int refcount;
  void *user_data;
  wl_dispatcher_func_t dispatcher;
};

struct wl_global {
  uint32_t id;
  char *interface;
  uint32_t version;
  struct wl_list link;
};

struct wl_event_queue {
  struct wl_list event_list;
  struct wl_display *display;
};

struct wl_display {
  struct wl_proxy proxy;
  struct wl_connection *connection;

  /* errno of the last wl_display error */
  int last_error;

  /* When display gets an error event from some object, it stores
   * information about it here, so that client can get this
   * information afterwards */
  struct {
    /* Code of the error. It can be compared to
     * the interface's errors enumeration. */
    uint32_t code;
    /* interface (protocol) in which the error occurred */
    const struct wl_interface *interface;
    /* id of the proxy that caused the error. There's no warranty
     * that the proxy is still valid. It's up to client how it will
     * use it */
    uint32_t id;
  } protocol_error;
  int fd;
  struct wl_map objects;
  struct wl_event_queue display_queue;
  struct wl_event_queue default_queue;
  pthread_mutex_t mutex;

  int reader_count;
  uint32_t read_serial;
  pthread_cond_t reader_cond;
};


/*
 *****************
 */

struct wl_display *wl_display_connect(const char *name) {
  static struct wl_display* (*__wl_display_connect)(const char *) = NULL;

  if (__wl_display_connect == NULL) {
    __wl_display_connect = (struct wl_display*(*)(const char *))dlsym(
        RTLD_NEXT, "wl_display_connect");
    if (__wl_display_connect == NULL) {
      fprintf(stderr, "Error in dlsym: %s(%s)\n", dlerror() ? dlerror() : "?",
              "wl_display_connect");
    }
  }

  if (1) tditrace("@I+wl_display_connect() \"%s\"", name?name:"");

  struct wl_display* disp = __wl_display_connect(name);

  if (1) tditrace("@I-wl_display_connect() =%x", disp);

  return disp;
}


// struct wl_display * wl_display_connect_to_fd(int fd)
