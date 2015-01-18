//
// network_handle.cpp: CEN64D network handle.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "network_handle.h"

NetworkHandle::NetworkHandle(QObject *parent) : socket(parent) {
  socket.connectToHost("localhost", 64646);
}

NetworkHandle::~NetworkHandle() {
}

