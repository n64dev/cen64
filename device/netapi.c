//
// device/netapi.c: CEN64 device network API.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"

#ifdef __WIN32__
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "device/device.h"
#include "device/netapi.h"
#include "vr4300/cpu.h"
#include "vr4300/pipeline.h"

// TODO: Really sloppy.
#ifdef __WIN32__
#define close(x) closesocket(x)
#endif

#define NETAPI_DEBUG_MAGIC 0x40544A53U // "@TJS"
#define NETAPI_DEBUG_VERSION 0U

// Static functions.
static int bind_server_socket(int family, int type, const char *service);

static struct addrinfo *getaddrinfo_helper(int family,
  int type, const char *host, const char *service);

static int netapi_debug_handle_request(int sfd, struct cen64_device *device,
  const struct netapi_debug_request *req, const uint8_t *req_data);

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

// Handles an incoming request from the client.
int netapi_debug_handle_request(int sfd, struct cen64_device *device,
  const struct netapi_debug_request *req, const uint8_t *req_data) {
  uint8_t buf[576];
  size_t length;
  unsigned i;

  struct netapi_debug_request resp;
  uint8_t *data = buf + sizeof(resp);
  uint32_t u32;
  uint64_t u64;

  switch(ntohl(req->type)) {

    // Get protocol version.
    case NETAPI_DEBUG_GET_PROTOCOL_VERSION:
      resp.type = req->type;
      length = sizeof(resp) + sizeof(u32);

      u32 = htonl(NETAPI_DEBUG_VERSION);
      memcpy(data, &u32, sizeof(u32));
      break;

    // Get VR4300 general purpose registers and $PC.
    case NETAPI_DEBUG_GET_VR4300_REGS:
      resp.type = req->type;
      length = sizeof(uint64_t) * 33;

      for (i = 0; i < 32; i++) {
        u64 = htonll(device->vr4300.regs[i]);
        memcpy(data + i * sizeof(u64), &u64, sizeof(u64));
      }

      u64 = htonll(device->vr4300.pipeline.dcwb_latch.common.pc);
      memcpy(data + 32 * sizeof(u64), &u64, sizeof(u64));
      break;

    // Unsupported command.
    default:
      resp.type = htonl(NETAPI_DEBUG_ERROR);
      length = sizeof(resp);
      break;
  }

  // Send the response.
  resp.magic = htonl(NETAPI_DEBUG_MAGIC);
  resp.length = htonl(length);

  memcpy(buf, &resp, sizeof(resp));
  send(sfd, buf, length, 0);
  return 0;
}

// Waits for an incoming request, attempts to handle it.
int netapi_debug_wait(int sfd, struct cen64_device *device) {
  struct netapi_debug_request req;
  uint8_t buf[576];

  // Pull and process the header.
  recv(sfd, &req, sizeof(req), 0);

  if (ntohl(req.magic) != NETAPI_DEBUG_MAGIC ||
    ntohl(req.length) > sizeof(buf)) {
    debug("net/debug: Got a bad request packet.\n");

    req.magic = htonl(NETAPI_DEBUG_MAGIC);
    req.length = htonl(sizeof(req));
    req.type = htonl(NETAPI_DEBUG_ERROR);

    send(sfd, &req, sizeof(req), 0);
    return -1;
  }

  // Pull the body of the request, if any.
  if (ntohl(req.length) > sizeof(req))
    recv(sfd, buf, ntohl(req.length) - sizeof(req), 0);

  return netapi_debug_handle_request(sfd, device, &req, buf);
}

