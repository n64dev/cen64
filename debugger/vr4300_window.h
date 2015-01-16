//
// vr4300_window.h: VR4300 view window.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef VR4300_WINDOW_H
#define VR4300_WINDOW_H

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include "disassembly_view.h"
#include "memory_view.h"
#include "register_view.h"
#include "toggle_window.h"

class VR4300Window : public ToggleWindow {
  Q_OBJECT
  QGridLayout layout;
  QGridLayout addressLayout;

  DisassemblyView disassemblyView;
  MemoryView memoryView;
  RegisterView registerView;

  QWidget addressWidget;
  QLabel vaddressLabel;
  QLabel paddressLabel;
  QLineEdit vaddressLine;
  QLineEdit paddressLine;

public:
  explicit VR4300Window(QAction *toggleAction, bool initiallyVisible);
  ~VR4300Window();
};

#endif

