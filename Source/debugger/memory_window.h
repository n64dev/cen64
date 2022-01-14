//
// memory_window.h: Memory view window.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef MEMORY_WINDOW_H
#define MEMORY_WINDOW_H

#include "memory_view.h"
#include "toggle_window.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

class MemoryWindow : public ToggleWindow {
  Q_OBJECT
  QGridLayout layout;

  MemoryView memoryView;
  QLabel addressLabel;
  QLineEdit addressLine;

public:
  explicit MemoryWindow(QAction *toggleAction, bool initiallyVisible);
  ~MemoryWindow();
};

#endif

