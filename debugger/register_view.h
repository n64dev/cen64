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

class RegisterView : public QAbstractScrollArea {
  Q_OBJECT

public:
  explicit RegisterView();
  virtual ~RegisterView();

  void paintEvent(QPaintEvent* event);
};

#endif

