//
// rdp_window.h: RDP view window.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef RDP_WINDOW_H
#define RDP_WINDOW_H

#include "toggle_window.h"

class RDPWindow : public ToggleWindow {
  Q_OBJECT

public:
  explicit RDPWindow(QAction *toggleAction, bool initiallyVisible);
  ~RDPWindow();
};

#endif

