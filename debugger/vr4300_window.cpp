//
// vr4300_window.cpp: VR4300 view window.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "vr4300_window.h"

static const char *regs[] = {
  "$zero",
  "$at",
  "$v0",
  "$v1",
  "$a0",
  "$a1",
  "$a2",
  "$a3",
  "$t0",
  "$t1",
  "$t2",
  "$t3",
  "$t4",
  "$t5",
  "$t6",
  "$t7",
  "$s0",
  "$s1",
  "$s2",
  "$s3",
  "$s4",
  "$s5",
  "$s6",
  "$s7",
  "$t8",
  "$t9",
  "$k0",
  "$k1",
  "$gp",
  "$sp",
  "$fp",
  "$ra"
};

VR4300Window::VR4300Window(QAction *toggleAction, bool initiallyVisible)
  : ToggleWindow(tr("CEN64D: VR4300"), toggleAction, initiallyVisible),
    memoryView(16), registerView(regs, sizeof(regs) / sizeof(*regs), 16) {
  setLayout(&layout);

  vaddressLabel.setText("Virtual address: ");
  paddressLabel.setText("Physical address: ");
  paddressLine.setReadOnly(true);

  addressWidget.setLayout(&addressLayout);
  addressLayout.addWidget(&vaddressLabel, 0, 1);
  addressLayout.addWidget(&vaddressLine, 0, 2);
  addressLayout.addWidget(&paddressLabel, 0, 3);
  addressLayout.addWidget(&paddressLine, 0, 4);

  layout.addWidget(&disassemblyView, 0, 1);
  layout.addWidget(&registerView, 0, 2);
  layout.addWidget(&memoryView, 1, 1, 1, 2);
  layout.addWidget(&addressWidget, 2, 1, 1, 2);

  layout.setColumnStretch(1, 25);
  layout.setColumnStretch(2, 10);
  layout.setRowStretch(0, 20);
  layout.setRowStretch(1, 10);
}

VR4300Window::~VR4300Window() {
}

