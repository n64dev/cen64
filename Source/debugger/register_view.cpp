//
// register_view.cpp: CEN64D register view (MVC).
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "register_view.h"
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSize>
#include <cstdio>
#include <cstring>

RegisterView::RegisterView(const char **registers,
  unsigned numRegisters, unsigned octets) : model(numRegisters),
  registers(registers), numRegisters(numRegisters), octets(octets) {
  unsigned i;

  QFont monospacedFont("Courier New");
  monospacedFont.setFixedPitch(true);
  monospacedFont.setPointSize(10);
  setFont(monospacedFont);

  QFontMetrics metrics(monospacedFont);
  fontWidth = metrics.width('0');
  fontHeight = metrics.height();

  // Find the strlen of the longest mnemonic.
  for (longestMnemonic = 0, i = 0; i < numRegisters; i++) {
    size_t len = strlen(registers[i]);

    if (len > longestMnemonic)
      longestMnemonic = len;
  }

  // Create the format string.
  sprintf(formatstr, " 0x%%.%ullX", octets);
}

RegisterView::~RegisterView() {
}

unsigned RegisterView::getMaximumHeight() const {
  return fontHeight * numRegisters;
}

unsigned RegisterView::getMaximumWidth() const {
  return 1 + (3 + octets + longestMnemonic) * fontWidth;
}

RegisterModel& RegisterView::getModel() {
  return model;
}

void RegisterView::paintEvent(QPaintEvent* event) {
  QSize area = maximumViewportSize();
  char buf[32];
  int i;

  unsigned hs = horizontalScrollBar()->value();
  unsigned vs = verticalScrollBar()->value();
  unsigned hsDW = hs / fontWidth;
  unsigned hsMW = hs % fontWidth;
  unsigned vsDH = vs / fontHeight;
  unsigned vsMH = vs % fontHeight;

  // TODO: Fonts don't seem to render exactly
  // where we want them, so add a fudge factor.
  float fudge = 2.0 / 3.0;
  unsigned start = fontHeight * fudge;

  QPainter painter(viewport());
  QBrush clear = QBrush(Qt::white);
  QBrush shaded = QBrush(QColor(0xE8, 0xE8, 0xE8));

  // Shade ever other line.
  painter.fillRect(0, 0, area.width(), area.height(), clear);

  if (vsDH & 0x1)
    painter.fillRect(0, 0, area.width(), fontHeight - vsMH, shaded);

  for (i = (vsDH & 0x1 ? fontHeight : 0) + fontHeight - vsMH;
    i < area.height(); i+= fontHeight * 2) {
    painter.fillRect(0, i, area.width(), fontHeight, shaded);
  }

  // Draw any values in the range.
  unsigned rs = vsDH;
  unsigned rf = vs + fontHeight * (numRegisters - rs);

  if (rf > numRegisters)
    rf = numRegisters;

  for (i = -vsMH; i < area.height() && rs < rf; i += fontHeight) {
    size_t len = strlen(registers[rs]);
    unsigned j;

    // Write the mnemonic, pad with spaces to align.
    strcpy(buf, registers[rs]);
    for (j = len; j < longestMnemonic; j++)
      buf[j] = ' ';

    // Write the values, draw the string.
    sprintf(buf + longestMnemonic, formatstr, model.getIndex(rs++));
    painter.drawText(1 - hsMW, start + i, buf + hsDW);
  }
}

void RegisterView::resizeEvent(QResizeEvent *event) {
  QSize area = maximumViewportSize();

  horizontalScrollBar()->setRange(0, getMaximumWidth() - area.width());
  verticalScrollBar()->setRange(0, getMaximumHeight() - area.height());
}

