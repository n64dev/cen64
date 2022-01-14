//
// main_window.cpp: CEN64D main window.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "main_window.h"
#include "ui_main_window.h"
#include <QMessageBox>
#include <QKeySequence>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  setupActions();
  setupMenuBar(menuBar());

  memoryWindow = new MemoryWindow(viewMemoryWindowAction, false);
  rdpWindow = new RDPWindow(viewRDPWindowAction, false);
  rspWindow = new RSPWindow(viewRSPWindowAction, false);
  vr4300Window = new VR4300Window(viewVR4300WindowAction, true);

  // Prevent any of the non-critical windows from keeping us alive.
  memoryWindow->setAttribute(Qt::WA_QuitOnClose, false);
  rdpWindow->setAttribute(Qt::WA_QuitOnClose, false);
  rspWindow->setAttribute(Qt::WA_QuitOnClose, false);
  vr4300Window->setAttribute(Qt::WA_QuitOnClose, false);

  statusBar()->showMessage(tr("Connecting to host...."));
  networkHandle = new NetworkHandle(this);
  statusBar()->showMessage(tr("Debugger ready."));
}

void MainWindow::setupActions() {
  fileQuitAction = new QAction(tr("&Quit"), this);
  fileQuitAction->setShortcuts(QKeySequence::Quit);
  fileQuitAction->setStatusTip("Quit the application.");
  connect(fileQuitAction, SIGNAL(triggered()), this, SLOT(close()));

  viewMemoryWindowAction = new QAction(tr("&Memory"), this);
  viewMemoryWindowAction->setStatusTip("Toggle the memory view window.");
  viewMemoryWindowAction->setCheckable(true);

  viewRDPWindowAction = new QAction(tr("R&DP"), this);
  viewRDPWindowAction->setStatusTip("Toggle the RDP view window.");
  viewRDPWindowAction->setCheckable(true);

  viewRSPWindowAction = new QAction(tr("R&SP"), this);
  viewRSPWindowAction->setStatusTip("Toggle the RSP view window.");
  viewRSPWindowAction->setCheckable(true);

  viewVR4300WindowAction = new QAction(tr("&VR4300"), this);
  viewVR4300WindowAction->setStatusTip("Toggle the VR4300 view window.");
  viewVR4300WindowAction->setCheckable(true);

  helpAboutAction = new QAction(tr("&About"), this);
  helpAboutAction->setStatusTip("Show the about window.");
  connect(helpAboutAction, SIGNAL(triggered()), this, SLOT(showAboutWindow()));
}

void MainWindow::setupMenuBar(QMenuBar *menuBar) {
  fileMenu = menuBar->addMenu(tr("&File"));
  viewMenu = menuBar->addMenu(tr("&View"));
  helpMenu = menuBar->addMenu(tr("&Help"));

  fileMenu->addAction(fileQuitAction);
  viewMenu->addAction(viewMemoryWindowAction);
  viewMenu->addAction(viewRDPWindowAction);
  viewMenu->addAction(viewRSPWindowAction);
  viewMenu->addAction(viewVR4300WindowAction);
  helpMenu->addAction(helpAboutAction);
}

void MainWindow::showAboutWindow() {
  QMessageBox::about(this, tr("About CEN64 Debugger"),
    tr("This is <b>CEN64</b>'s official debugger."));
}

MainWindow::~MainWindow() {
  delete ui;
}

