#ifndef WASM32_OPTS_H
#define WASM32_OPTS_H

enum wasm32_pc_type {
  PC_LAX = 0,
  PC_STRICT,
  PC_MEDIUM
};

enum wasm32_breakpoints_type {
  BP_MANY = 0,
  BP_NONE,
  BP_SINGLE,
  BP_PLETHORA
};

enum wasm32_bogotics_type {
  BOGOTICS_NONE = 0,
  BOGOTICS_ALL,
  BOGOTICS_BACKWARDS
};

#endif
