#ifndef WASM64_OPTS_H
#define WASM64_OPTS_H

enum wasm64_pc_type {
  PC_LAX = 0,
  PC_STRICT,
  PC_MEDIUM
};

enum wasm64_breakpoints_type {
  BP_MANY = 0,
  BP_NONE,
  BP_SINGLE,
  BP_PLETHORA
};

enum wasm64_bogotics_type {
  BOGOTICS_NONE = 0,
  BOGOTICS_ALL,
  BOGOTICS_BACKWARDS
};

#endif
