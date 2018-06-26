
#define MAXSTRLEN 128
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

extern "C" ssize_t send(int sockfd, const void* buf, size_t len, int flags) {
  static ssize_t (*__send)(int, const void*, size_t, int) = NULL;

  if (__send == NULL) {
    __send =
        (ssize_t(*)(int, const void*, size_t, int))dlsym(RTLD_NEXT, "send");
    if (NULL == __send) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (MAXSTRLEN) {
    if (buf) {
      char s[MAXSTRLEN + 1];
      strncpy(s, (const char*)buf, MIN(MAXSTRLEN, len));
      s[MIN(MAXSTRLEN, len)] = '\0';

      if (strncmp((const char*)buf, "GET", 3) == 0) {
        tditrace("@E+send()_GET %d %d \"%s\"%n", sockfd, len, s, ra);
      } else if (strncmp((const char*)buf, "PUT", 3) == 0) {
        tditrace("@E+send()_PUT %d %d \"%s\"%n", sockfd, len, s, ra);
      } else if (strncmp((const char*)buf, "POST", 4) == 0) {
        tditrace("@E+send()_POST %d %d \"%s\"%n", sockfd, len, s, ra);
      } else if (strncmp((const char*)buf, "{\"", 2) == 0) {
        tditrace("@E+send()_{ %d %d \"%s\"%n", sockfd, len, s, ra);
      } else {
        // s[MIN(4, len)] = '\0';
        // tditrace("@E+send()_%d_? %d \"%s...\"", sockfd, len, s);
        tditrace("@E+send() %d %d \"...\"%n", sockfd, len, ra);
      }

    } else {
      tditrace("@E+send() %d %d%n", sockfd, len, ra);
    }

  } else {
    tditrace("@E+send() %d %d", sockfd, len, ra);
  }

  ssize_t ret = __send(sockfd, buf, len, flags);

  return ret;
}

extern "C" ssize_t recv(int sockfd, void* buf, size_t len, int flags) {
  static ssize_t (*__recv)(int, void*, size_t, int) = NULL;

  if (__recv == NULL) {
    __recv = (ssize_t(*)(int, void*, size_t, int))dlsym(RTLD_NEXT, "recv");
    if (NULL == __recv) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  ssize_t ret = __recv(sockfd, buf, len, flags);

  if (ret == -1) {
    // tditrace("@I-recv() =-1");
    tditrace("@E+recv() %d %d =-1%n", sockfd, len, ra);
  } else if (ret == 0) {
    // tditrace("@I-recv() =0");
    tditrace("@E+recv() %d %d =0%n", sockfd, len, ra);
  } else {
    if (MAXSTRLEN) {
      if (buf) {
        char s[MAXSTRLEN + 1];
        strncpy(s, (const char*)buf, MIN(MAXSTRLEN, ret));
        s[MIN(MAXSTRLEN, ret)] = '\0';

        if (strncmp((const char*)buf, "HTTP", 4) == 0) {
          tditrace("@E+recv()_HTTP %d %d =%d \"%s\"%n", sockfd, len, ret, s,
                   ra);
        } else {
          int i;
          for (i = 0; i < MIN(MAXSTRLEN, ret); i++)
            if (s[i] < 0x20 || s[i] >= 0x7f) s[i] = '.';

          tditrace("@E+recv() %d %d =%d \"%s\"", sockfd, len, ret,
                   (s[0] >= 0x20 && s[0] < 0x7f) ? s : "?", ra);
        }
      }
    }
  }

  return ret;
}

extern "C" ssize_t sendfile(int out_fd, int in_fd, off_t* offset, size_t count);
{
  static ssize_t (*__sendfile)(int, int, off_t*, size_t) = NULL;

  if (__sendfile == NULL) {
    __sendfile =
        (ssize_t(*)(int, int, off_t*, size_t))dlsym(RTLD_NEXT, "sendfile");
    if (NULL == __sendfile) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  tditrace("@A+sendfile %d %d %d", out_fd, in_fd, count);

  ssize_t ret = __send(out_fd, in_fd, offset, count);

  tditrace("@A-sendfile =%d", ret);

  return ret;
}
