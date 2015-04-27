//
// rsp_window.h: RSP view window.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef RSP_WINDOW_H
#define RSP_WINDOW_H

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include "disassembly_view.h"
#include "memory_view.h"
#include "register_view.h"
#include "toggle_window.h"

class RSPWindow : public ToggleWindow {
  Q_OBJECT
  QGridLayout layout;
  QGridLayout addressLayout;

  DisassemblyView disassemblyView;
  MemoryView memoryView;
  RegisterView registerView;

  QWidget addressWidget;
  QLabel addressLabel;
  QLineEdit addressLine;

public:
  explicit RSPWindow(QAction *toggleAction, bool initiallyVisible);
  ~RSPWindow();
};

#endif

