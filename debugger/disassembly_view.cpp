//
// disassembly_view.cpp: CEN64D disassembly view (MVC).
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "disassembly_view.h"
#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QSize>

DisassemblyView::DisassemblyView() {
  QFont monospacedFont("Courier New");
  monospacedFont.setFixedPitch(true);
  monospacedFont.setPointSize(10);
  setFont(monospacedFont);

  QFontMetrics metrics(monospacedFont);
  fontWidth = metrics.width('0');
  fontHeight = metrics.height();
}

DisassemblyView::~DisassemblyView() {
}

void DisassemblyView::paintEvent(QPaintEvent* event) {
  QSize area = viewport()->size();

  // TODO: Fonts don't seem to render exactly
  // where we want them, so add a fudge factor.
  float fudge = 2.0 / 3.0;
  unsigned start = fontHeight * fudge;

  QPainter painter(viewport());

  painter.fillRect(0, 0, area.width(), area.height(), QBrush(Qt::white));
  painter.drawText(1, start, "This is the disassembly view.");
}

