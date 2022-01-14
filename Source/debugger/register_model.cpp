//
// register_model.cpp: CEN64D register model (MVC).
//
// CEN64D: Cycle-Accurate Nintendo 64 Debugger
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "register_model.h"

RegisterModel::RegisterModel(unsigned num_regs) {
  data = new quint64[num_regs];
}

RegisterModel::~RegisterModel() {
  delete[] data;
}

quint64 RegisterModel::getIndex(unsigned i) {
  return data[i];
}

void RegisterModel::setIndex(unsigned i, quint64 d) {
  data[i] = d;
}

