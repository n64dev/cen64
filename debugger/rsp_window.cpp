//
// rsp_window.cpp: RSP view window.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "rsp_window.h"

static const char *regs[] = {
  "$r0",
  "$r1",
  "$r2",
  "$r3",
  "$r4",
  "$r5",
  "$r6",
  "$r7",
  "$r8",
  "$r9",
  "$r10",
  "$r11",
  "$r12",
  "$r13",
  "$r14",
  "$r15",
  "$r16",
  "$r17",
  "$r18",
  "$r19",
  "$r20",
  "$r21",
  "$r22",
  "$r23",
  "$r24",
  "$r25",
  "$r26",
  "$r27",
  "$r28",
  "$r29",
  "$r30",
  "$r31"
};

RSPWindow::RSPWindow(QAction *toggleAction, bool initiallyVisible)
  : ToggleWindow(tr("CEN64D: RSP"), toggleAction, initiallyVisible),
    memoryView(4), registerView(regs, sizeof(regs) / sizeof(*regs), 8) {
  setLayout(&layout);

  addressLabel.setText("Address: ");

  addressWidget.setLayout(&addressLayout);
  addressLayout.addWidget(&memoryView, 0, 1, 1, 2);
  addressLayout.addWidget(&addressLabel, 1, 1);
  addressLayout.addWidget(&addressLine, 1, 2);

  layout.addWidget(&disassemblyView, 0, 1);
  layout.addWidget(&registerView, 0, 2);
  layout.addWidget(&addressWidget, 1, 1, 1, 2);

  layout.setColumnStretch(1, 25);
  layout.setColumnStretch(2, 10);
  layout.setRowStretch(0, 20);
  layout.setRowStretch(1, 10);
}

RSPWindow::~RSPWindow() {
}

