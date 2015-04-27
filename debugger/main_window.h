//
// main_window.h: CEN64D main window.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "network_handle.h"
#include "memory_window.h"
#include "rdp_window.h"
#include "rsp_window.h"
#include "vr4300_window.h"
#include <QAction>
#include <QMainWindow>
#include <QMenuBar>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT
  Ui::MainWindow *ui;

  NetworkHandle *networkHandle;
  MemoryWindow *memoryWindow;
  RDPWindow *rdpWindow;
  RSPWindow *rspWindow;
  VR4300Window *vr4300Window;

  QMenu *fileMenu;
  QAction *fileQuitAction;

  QMenu *viewMenu;
  QAction *viewMemoryWindowAction;
  QAction *viewRDPWindowAction;
  QAction *viewRSPWindowAction;
  QAction *viewVR4300WindowAction;

  QMenu *helpMenu;
  QAction *helpAboutAction;

  void setupActions();
  void setupMenuBar(QMenuBar *menuBar);

private slots:
  void showAboutWindow();

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
};

#endif

