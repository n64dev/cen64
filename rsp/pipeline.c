//
// rsp/pipeline.c: RSP processor pipeline.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "rsp/cp0.h"
#include "rsp/cpu.h"

typedef void (*pipeline_function)(struct rsp *rsp);

// Instruction cache fetch stage.
static inline void rsp_if_stage(struct rsp *rsp) {

}

// Register fetch and decode stage.
static inline void rsp_rd_stage(struct rsp *rsp) {

}

// Execution stage.
static inline void rsp_ex_stage(struct rsp *rsp) {

}

// Data cache fetch stage.
static inline void rsp_df_stage(struct rsp *rsp) {

}

// Writeback stage.
static inline void rsp_wb_stage(struct rsp *rsp) {

}

// Advances the processor pipeline by one clock.
void rsp_cycle(struct rsp *rsp) {
  struct rsp_pipeline *pipeline = &rsp->pipeline;

  if (rsp->regs[RSP_CP0_REGISTER_SP_STATUS] & SP_STATUS_HALT)
    return;

  rsp_wb_stage(rsp);
  rsp_df_stage(rsp);
  rsp_ex_stage(rsp);
  rsp_rd_stage(rsp);
  rsp_if_stage(rsp);
}

// Initializes the pipeline with default values.
void rsp_pipeline_init(struct rsp_pipeline *pipeline) {

}

