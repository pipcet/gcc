#ifndef WASM_OPTS_H
#define WASM_OPTS_H

enum wasm_pc_type {
  PC_LAX = 0,
  PC_STRICT,
  PC_MEDIUM
};

enum wasm_breakpoints_type {
  BP_MANY = 0,
  BP_NONE,
  BP_SINGLE,
  BP_PLETHORA
};

enum wasm_bogotics_type {
  BOGOTICS_NONE = 0,
  BOGOTICS_ALL,
  BOGOTICS_BACKWARDS
};

#endif
