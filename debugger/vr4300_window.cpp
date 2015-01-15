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

VR4300Window::VR4300Window(QAction *toggleAction, bool initiallyVisible)
  : ToggleWindow(tr("CEN64D: VR4300"), toggleAction, initiallyVisible) {
  setLayout(&layout);

  layout.addWidget(&disassemblyView, 0, 1);
  layout.addWidget(&registerView, 0, 2);
  layout.addWidget(&memoryView, 1, 1, 1, 2);

  layout.setColumnStretch(1, 25);
  layout.setColumnStretch(2, 10);
  layout.setRowStretch(0, 20);
  layout.setRowStretch(1, 10);
}

VR4300Window::~VR4300Window() {
}

