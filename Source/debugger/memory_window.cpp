//
// memory_window.cpp: Memory view window.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "memory_window.h"

MemoryWindow::MemoryWindow(QAction *toggleAction, bool initiallyVisible)
  : ToggleWindow(tr("CEN64D: Memory"), toggleAction, initiallyVisible),
    memoryView(8) {

  addressLabel.setText("Address: ");

  layout.addWidget(&memoryView, 0, 1, 1, 2);
  layout.addWidget(&addressLabel, 1, 1);
  layout.addWidget(&addressLine, 1, 2);

  setLayout(&layout);
}

MemoryWindow::~MemoryWindow() {
}

