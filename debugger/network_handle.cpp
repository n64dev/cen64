//
// network_handle.cpp: CEN64D network handle.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "network_handle.h"
#include <QMessageBox>
#include <QObject>
#include <QTranslator>

#ifdef __WIN32__
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

#include <cstdio>

#define NETAPI_DEBUG_MAGIC 0x40544A53U // "@TJS"
#define NETAPI_DEBUG_VERSION 1U

enum netapi_debug_request_type {
  NETAPI_DEBUG_ERROR,
  NETAPI_DEBUG_GET_PROTOCOL_VERSION,
  NETAPI_DEBUG_GET_VR4300_REGS,
};

struct netapi_debug_request {
  quint32 magic;
  quint32 seq_id;
  quint32 length;

  enum netapi_debug_request_type type;
  quint8 data[];
};

NetworkHandle::NetworkHandle(QWidget *parent) : socket(parent) {
  socket.connectToHost("localhost", 64646);
  socket.waitForConnected();
}

NetworkHandle::~NetworkHandle() {
}

int NetworkHandle::getProtocolVersion() {
  return NETAPI_DEBUG_VERSION;
}

int NetworkHandle::getRemoteProtocolVersion() {
  struct netapi_debug_request req, resp;
  qint32 version;

  req.magic = htonl(NETAPI_DEBUG_MAGIC);
  req.length = htonl(sizeof(req));
  req.type = (enum netapi_debug_request_type) htonl(NETAPI_DEBUG_GET_PROTOCOL_VERSION);
  socket.write((char *) &req, sizeof(req));

  socket.waitForReadyRead();
  socket.read((char *) &resp, sizeof(resp));

  if (ntohl(resp.magic) != NETAPI_DEBUG_MAGIC ||
    ntohl(resp.type) != NETAPI_DEBUG_GET_PROTOCOL_VERSION ||
    ntohl(resp.length) != (sizeof(resp) + sizeof(quint32)))
    return -1;

  socket.read((char *) &version, sizeof(version));
  fprintf(stderr, "got version: %u\n", ntohl(version));
  return ntohl(version);
}

