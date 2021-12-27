#ifndef ASMJS_OPTS_H
#define ASMJS_OPTS_H

enum asmjs_pc_type {
  PC_LAX = 0,
  PC_STRICT,
  PC_MEDIUM
};

enum asmjs_breakpoints_type {
  BP_MANY = 0,
  BP_NONE,
  BP_SINGLE,
  BP_PLETHORA
};

enum asmjs_bogotics_type {
  BOGOTICS_NONE = 0,
  BOGOTICS_ALL,
  BOGOTICS_BACKWARDS
};

#endif
