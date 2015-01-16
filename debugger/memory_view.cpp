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
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QSize>
#include <cstdio>

MemoryView::MemoryView(unsigned addressOctets) : addressOctets(addressOctets) {
  QFont monospacedFont("Courier New");
  monospacedFont.setFixedPitch(true);
  monospacedFont.setPointSize(10);
  setFont(monospacedFont);

  QFontMetrics metrics(monospacedFont);
  fontWidth = metrics.width('0');
  fontHeight = metrics.height();

  // Create the format string.
  sprintf(formatstr, " 0x%%.%ullX", addressOctets);
}

MemoryView::~MemoryView() {
}

void MemoryView::paintEvent(QPaintEvent* event) {
  QSize area = viewport()->size();
  char buf[32];
  int i, j;

  // TODO: Fonts don't seem to render exactly
  // where we want them, so add a fudge factor.
  float fudge = 2.0 / 3.0;
  unsigned start = fontHeight * fudge;

  int byteStart = 1 + (3 + addressOctets) * fontWidth;
  int bytesPerRow = 1;

  // Determine how many bytes we can cram in the row without having
  // to resort to horizontal sliders. Make sure that we keep the byte
  // count to a power of two for simplicity's sake.
  for (i = 2; ; i *= 2) {
    unsigned check = byteStart + i * 3 * fontWidth + (i + 1) * fontWidth;

    if (check > area.width())
      break;

    bytesPerRow = i;
  }

  QPainter painter(viewport());
  QBrush clear = QBrush(Qt::white);
  QBrush shaded = QBrush(QColor(0xE8, 0xE8, 0xE8));

  painter.fillRect(0, 0, area.width(), area.height(), QBrush(Qt::white));

  // Shade ever other line.
  painter.fillRect(0, 0, area.width(), area.height(), clear);

  for (i = fontHeight; i < area.height(); i += fontHeight * 2)
    painter.fillRect(0, i, area.width(), fontHeight, shaded);

  // Draw any values in the range.
  for (i = 0; i < area.height(); i += fontHeight) {
    sprintf(buf, formatstr, (unsigned long long) 0 + (i * bytesPerRow));
    painter.drawText(1, start + i, buf);

    for (j = 0; j < bytesPerRow; j++) {
      sprintf(buf, " %02X ", j);
      painter.drawText(byteStart + j  * fontWidth * 3, start + i, buf);

      buf[1] = '\0';
      buf[0] = isprint(j) ? j : '.';
      painter.drawText(byteStart + bytesPerRow * 3 * fontWidth + (j + 1) * fontWidth, start + i, buf);
    }
  }
}

