//
// memory_view.h: CEN64D memory view (MVC).
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef MEMORY_VIEW_H
#define MEMORY_VIEW_H
#include <QAbstractScrollArea>

class MemoryView : public QAbstractScrollArea {
  Q_OBJECT

  unsigned fontWidth, fontHeight;
  unsigned addressOctets;

  char formatstr[16];

public:
  explicit MemoryView(unsigned addressOctets);
  virtual ~MemoryView();

  void paintEvent(QPaintEvent* event);
};

#endif

