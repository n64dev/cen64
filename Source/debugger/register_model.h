//
// register_model.h: CEN64D register model (MVC).
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef REGISTER_MODEL_H
#define REGISTER_MODEL_H
#include <QtGlobal>

class RegisterModel {
  quint64 *data;

public:
  explicit RegisterModel(unsigned num_regs);
  virtual ~RegisterModel();

  quint64 getIndex(unsigned i);
  void setIndex(unsigned i, quint64);
};

#endif

