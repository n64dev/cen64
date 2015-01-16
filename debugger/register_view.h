//
// register_view.h: CEN64D register view (MVC).
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef REGISTER_VIEW_H
#define REGISTER_VIEW_H
#include <QAbstractScrollArea>
#include <QPaintEvent>
#include <QResizeEvent>

class RegisterView : public QAbstractScrollArea {
  Q_OBJECT

  const char **registers;
  unsigned numRegisters;

  unsigned fontWidth, fontHeight;
  unsigned longestMnemonic, octets;
  char formatstr[16];

public:
  explicit RegisterView(const char **regs, unsigned num_regs, unsigned octets);
  virtual ~RegisterView();

  unsigned getMaximumHeight() const;
  unsigned getMaximumWidth() const;

  void paintEvent(QPaintEvent* event);
  void resizeEvent(QResizeEvent *event);
};

#endif

