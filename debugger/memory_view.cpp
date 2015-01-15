//
// memory_view.cpp: CEN64D memory view (MVC).
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "memory_view.h"
#include <QBrush>
#include <QPainter>
#include <QSize>

MemoryView::MemoryView() {
}

MemoryView::~MemoryView() {
}

void MemoryView::paintEvent(QPaintEvent* event) {
  QPainter painter(viewport());
  QSize area = viewport()->size();

  painter.fillRect(0, 0, area.width(), area.height(), QBrush(Qt::green));
}

