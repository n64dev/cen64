//
// toggle_window.h: Toggle-able window.
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef TOGGLE_WINDOW_H
#define TOGGLE_WINDOW_H

#include <QAction>
#include <QCloseEvent>
#include <QState>
#include <QStateMachine>
#include <QWidget>

class ToggleWindow : public QWidget {
  Q_OBJECT

  QAction *toggleAction;

  QStateMachine windowMachine;
  QState windowHiddenState;
  QState windowExposedState;

  virtual void closeEvent(QCloseEvent *event);

public:
  explicit ToggleWindow(const QString &windowTitle,
    QAction *toggleAction, bool initiallyVisible);

  ~ToggleWindow();
};

#endif
