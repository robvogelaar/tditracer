#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// char *socket_path = "./socket";
char *socket_path = (char *)"\0hidden";

int main(int argc, char *argv[]) {
  struct sockaddr_un addr;
  char buf[100];
  int fd, rc;

  if (argc > 1) socket_path = argv[1];

  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket error");
    exit(-1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  if (*socket_path == '\0') {
    *addr.sun_path = '\0';
    strncpy(addr.sun_path + 1, socket_path + 1, sizeof(addr.sun_path) - 2);
  } else {
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
  }

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("connect error");
    exit(-1);
  }

  while ((rc = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
    if (write(fd, buf, (rc - 1)) != (rc - 1)) {
      if (rc > 0)
        fprintf(stderr, "partial write");
      else {
        perror("write error");
        exit(-1);
      }
    }

    rc = read(fd, buf, sizeof(buf));
    if (rc == -1) {
      fprintf(stdout, "simclient:%d\n", rc);
      perror("read");
    } else if (rc == 0) {
      fprintf(stdout, "simclient:%d\n", rc);
    } else {
      buf[rc] = 0;
#if 0
      fprintf(stdout, "simclient:\"%s\"\n", buf);
#endif
    }
  }

  return 0;
}
