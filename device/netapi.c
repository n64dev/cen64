//
// device/netapi.c: CEN64 device network API.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "device/netapi.h"

#ifdef __WIN32__
#include <ws2tcpip.h>
#include <winsock2.h>
#include <windows.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

// TODO: Really sloppy.
#ifdef __WIN32__
#define close(x) closesocket(x)
#endif

// Static functions.
static int bind_server_socket(int family, int type, const char *service);

static struct addrinfo *getaddrinfo_helper(int family,
  int type, const char *host, const char *service);

// Creates a socket of (family, type) and binds it portno.
int bind_server_socket(int family, int type, const char *service) {
  struct addrinfo *res, *i;
  int sfd, ret;

  if ((res = getaddrinfo_helper(family, type, NULL, service)) == NULL)
    return -1;

  // Walk the list; try to bind a socket.
  for (i = res; i != NULL; i = i->ai_next) {
    if ((sfd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) < 0)
      continue;

    // Disallow IPv4 on IPv6.
    if (i->ai_family == AF_INET6) {
#ifdef __WIN32__
      char on = 1;
#else
      int on = 1;
#endif

      if (setsockopt(sfd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)) != 0) {
        close(sfd);
        return -1;
      }
    }

    if ((ret = bind(sfd, i->ai_addr, i->ai_addrlen)) != 0) {
      close(sfd);
      continue;
    }

    freeaddrinfo(res);
    return sfd;
  }

  freeaddrinfo(res);
  return -2;
}

// Gets a list of address information for a given configuration.
struct addrinfo *getaddrinfo_helper(int family,
  int type, const char *host, const char *service) {
  struct addrinfo hints, *res;
  long int portno;
  char *endptr;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = family;
  hints.ai_socktype = type;

  // Port or IANA service?
  if (service != NULL) {
    portno = strtol(service, &endptr, 10);

    if (*endptr == '\0' && *service != '\0') {
      if (portno < 0 || portno >= 65536)
        return NULL;

#ifndef __WIN32__
      hints.ai_flags |= AI_NUMERICSERV;
#endif
    }
  }

  // Allow connects to localhost.
  if (host == NULL || !strcmp(host, "localhost")) {
    hints.ai_flags |= AI_PASSIVE;
    host = NULL;
  }

  return getaddrinfo(host, service, &hints, &res) != 0
    ? NULL
    : res;
}

// Closes a connection.
void netapi_close_connection(int csfd) {
  close(csfd);
}

// Attempts to open an IPv4 socket and binds it, listens.
// TODO: Respect the interface/port passed from the client.
int netapi_open_connection(void) {
  struct sockaddr address;
  socklen_t addrlen;
  int sfd, csfd;

  if ((sfd = bind_server_socket(AF_INET, SOCK_STREAM, "64646")) < 0)
    return -1;

  if (listen(sfd, 1)) {
    close(sfd);
    return -2;
  }

  if ((csfd = accept(sfd, &address, &addrlen)) < 0) {
    close(sfd);
    return -3;
  }

  // TODO: Might not want to toss this out yet.
  close(sfd);
  return csfd;
}

