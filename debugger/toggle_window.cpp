//
// toggle_window.cpp: Toggle-able window.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "toggle_window.h"

ToggleWindow::ToggleWindow(const QString &windowTitle,
  QAction *toggleAction, bool initiallyVisible) : toggleAction(toggleAction) {
  setWindowTitle(windowTitle);

  // Create states and connect signals to slots.
  windowHiddenState.addTransition(toggleAction,
    SIGNAL(triggered()), &windowExposedState);

  windowExposedState.addTransition(toggleAction,
    SIGNAL(triggered()), &windowHiddenState);

  QObject::connect(&windowHiddenState, SIGNAL(entered()), this, SLOT(hide()));
  QObject::connect(&windowExposedState, SIGNAL(entered()), this, SLOT(show()));

  if (toggleAction->isCheckable())
    toggleAction->setChecked(initiallyVisible);

  // Setup the state machine.
  windowMachine.addState(&windowHiddenState);
  windowMachine.addState(&windowExposedState);
  windowMachine.setInitialState(initiallyVisible
    ? &windowExposedState
    : &windowHiddenState);

  windowMachine.start();
}

ToggleWindow::~ToggleWindow() {
}

void ToggleWindow::closeEvent(QCloseEvent *event) {
  toggleAction->trigger();
  event->accept();
}
