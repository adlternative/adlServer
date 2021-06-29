#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define ADD_EPOLL_OPTION 0 // define as EPOLLET or 0

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#if !defined(__linux__) && !defined(__CYGWIN__)
#include <sys/event.h>
#define reactor_epoll 0
#else
#define reactor_epoll 1
#include <sys/epoll.h>
#include <sys/timerfd.h>
#endif

int sock_listen(const char *address, const char *port);
void *listen_threard(void *arg);
void *client_thread(void *arg);
int server_fd;
char const *address = NULL;
char const *port = "8000";

int main(int argc, char const *argv[]) {
  if (argc == 2) {
    port = argv[1];
  } else if (argc == 3) {
    port = argv[2];
    address = argv[1];
  }
  fprintf(stderr, "Test address: %s:%s\n", address ? address : "<null>", port);
  server_fd = sock_listen(address, port);
  /* code */
  pthread_t threads[4];

  for (size_t i = 0; i < 2; i++) {
    if (pthread_create(threads + i, NULL, listen_threard, (void *)i))
      perror("couldn't initiate server thread"), exit(-1);
  }
  for (size_t i = 2; i < 4; i++) {
    sleep(1);
    if (pthread_create(threads + i, NULL, client_thread, (void *)i))
      perror("couldn't initiate client thread"), exit(-1);
  }
  // join only server threads.
  for (size_t i = 0; i < 2; i++) {
    pthread_join(threads[i], NULL);
  }
  close(server_fd);
  sleep(1);
  return 0;
}

/**
Sets a socket to non blocking state.
*/
inline int sock_set_non_block(int fd) // Thanks to Bjorn Reese
{
/* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
  /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
  int flags;
  if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
    flags = 0;
  // printf("flags initial value was %d\n", flags);
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
  /* Otherwise, use the old way of doing it */
  static int flags = 1;
  return ioctl(fd, FIOBIO, &flags);
#endif
}

/* open a listenning socket */
int sock_listen(const char *address, const char *port) {
  int srvfd;
  // setup the address
  struct addrinfo hints;
  struct addrinfo *servinfo;       // will point to the results
  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
  if (getaddrinfo(address, port, &hints, &servinfo)) {
    perror("addr err");
    return -1;
  }
  // get the file descriptor
  srvfd =
      socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  if (srvfd <= 0) {
    perror("socket err");
    freeaddrinfo(servinfo);
    return -1;
  }
  // // keep the server socket blocking for the test.
  // // make sure the socket is non-blocking
  // if (sock_set_non_block(srvfd) < 0) {
  //   perror("couldn't set socket as non blocking! ");
  //   freeaddrinfo(servinfo);
  //   close(srvfd);
  //   return -1;
  // }

  // avoid the "address taken"
  {
    int optval = 1;
    setsockopt(srvfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  }
  // bind the address to the socket
  {
    int bound = 0;
    for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
      if (!bind(srvfd, p->ai_addr, p->ai_addrlen))
        bound = 1;
    }

    if (!bound) {
      // perror("bind err");
      freeaddrinfo(servinfo);
      close(srvfd);
      return -1;
    }
  }
  freeaddrinfo(servinfo);
  // listen in
  if (listen(srvfd, SOMAXCONN) < 0) {
    perror("couldn't start listening");
    close(srvfd);
    return -1;
  }
  return srvfd;
}
/* will start listenning, sleep for 5 seconds, then accept all the backlog and
 * finish */
void *listen_threard(void *arg) {

  int epoll_fd;
  ssize_t event_count;
#if reactor_epoll

#ifndef EPOLLEXCLUSIVE
#warning EPOLLEXCLUSIVE undeclared, test is futile
#define EPOLLEXCLUSIVE 0
#endif
  // create the epoll wait fd
  epoll_fd = epoll_create1(0);
  if (epoll_fd < 0)
    perror("couldn't create epoll fd"), exit(1);
  // add the server fd to the epoll watchlist
  {
    struct epoll_event chevent = {0};
    chevent.data.ptr = (void *)((uintptr_t)server_fd);
    chevent.events =
        EPOLLOUT | EPOLLIN | EPOLLERR | EPOLLEXCLUSIVE | ADD_EPOLL_OPTION;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &chevent);
  }
  // wait with epoll
  struct epoll_event events[10];
  event_count = epoll_wait(epoll_fd, events, 10, 5000);
#else
  // testing on BSD, use kqueue
  epoll_fd = kqueue();
  if (epoll_fd < 0)
    perror("couldn't create kqueue fd"), exit(1);
  // add the server fd to the kqueue watchlist
  {
    struct kevent chevent[2];
    EV_SET(chevent, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0,
           (void *)((uintptr_t)server_fd));
    EV_SET(chevent + 1, server_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0,
           (void *)((uintptr_t)server_fd));
    kevent(epoll_fd, chevent, 2, NULL, 0, NULL);
  }
  // wait with kqueue
  static struct timespec reactor_timeout = {.tv_sec = 5, .tv_nsec = 0};
  struct kevent events[10];
  event_count = kevent(epoll_fd, NULL, 0, events, 10, &reactor_timeout);
#endif

  close(epoll_fd);

  if (event_count <= 0) {
    fprintf(stderr, "Server thread %lu wakeup no events / error\n",
            (size_t)arg + 1);
    perror("errno ");
    return NULL;
  }

  fprintf(stderr, "Server thread %lu woke up with %lu events\n",
          (size_t)arg + 1, event_count);
  fprintf(stderr,
          "Server thread %lu will sleep for a second, to let things happen.\n",
          (size_t)arg + 1);
  sleep(1);
  int connfd;
  struct sockaddr_storage client_addr;
  socklen_t client_addrlen = sizeof client_addr;
  /* accept up all connections. we're non-blocking, -1 == no more connections */
  if ((connfd = accept(server_fd, (struct sockaddr *)&client_addr,
                       &client_addrlen)) >= 0) {
    fprintf(stderr,
            "Server thread %lu accepted a connection and saying hello.\n",
            (size_t)arg + 1);
    if (write(connfd,
              arg ? "Hello World - from server thread 2."
                  : "Hello World - from server thread 1.",
              35) < 35)
      perror("server write failed");
    close(connfd);
  } else {
    fprintf(stderr, "Server thread %lu failed to accept a connection",
            (size_t)arg + 1);
    perror(": ");
  }
  return NULL;
}

void *client_thread(void *arg) {

  int fd;
  // setup the address
  struct addrinfo hints;
  struct addrinfo *addrinfo;       // will point to the results
  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
  if (getaddrinfo(address, port, &hints, &addrinfo)) {
    perror("client couldn't initiate address");
    return NULL;
  }
  // get the file descriptor
  fd =
      socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
  if (fd <= 0) {
    perror("client couldn't create socket");
    freeaddrinfo(addrinfo);
    return NULL;
  }
  // // // Leave the socket blocking for the test.
  // // make sure the socket is non-blocking
  // if (sock_set_non_block(fd) < 0) {
  //   freeaddrinfo(addrinfo);
  //   close(fd);
  //   return -1;
  // }

  if (connect(fd, addrinfo->ai_addr, addrinfo->ai_addrlen) < 0 &&
      errno != EINPROGRESS) {
    fprintf(stderr, "client number %lu FAILED\n", (size_t)arg - 1);
    perror("client connect failure");
    close(fd);
    freeaddrinfo(addrinfo);
    return NULL;
  }
  freeaddrinfo(addrinfo);
  fprintf(stderr, "client number %lu connected\n", (size_t)arg - 1);
  char buffer[128];
  if (read(fd, buffer, 35) < 35) {
    perror("client: read error");
    close(fd);
  } else {
    buffer[35] = 0;
    fprintf(stderr, "client %lu: %s\n", (size_t)arg - 1, buffer);
    close(fd);
  }
  return NULL;
}