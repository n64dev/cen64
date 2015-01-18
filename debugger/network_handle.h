//
// network_handle.h: CEN64D network handle.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef NETWORK_HANDLE_H
#define NETWORK_HANDLE_H
#include <QTcpSocket>

class NetworkHandle {
  QTcpSocket socket;

public:
  explicit NetworkHandle(QObject *parent);
  ~NetworkHandle();
};

#endif

