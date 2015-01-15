//
// disassembly_view.h: CEN64D disassembly view (MVC).
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef DISASSEMBLY_VIEW_H
#define DISASSEMBLY_VIEW_H
#include <QAbstractScrollArea>

class DisassemblyView : public QAbstractScrollArea {
  Q_OBJECT

public:
  explicit DisassemblyView();
  virtual ~DisassemblyView();

  void paintEvent(QPaintEvent* event);
};

#endif

