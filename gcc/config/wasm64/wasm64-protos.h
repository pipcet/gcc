/* Prototypes for exported functions defined in wasm64.c
   Copyright (C) 1999-2015 Free Software Foundation, Inc.
   Copyright (C) 2016 Pip Cet <pipcet@gmail.com

   This file is NOT part of GCC.

   GCC is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

extern void wasm64_cpu_cpp_builtins (struct cpp_reader * pfile);
extern enum reg_class wasm64_regno_reg_class(int regno);

extern void wasm64_print_operand(FILE *stream, rtx x, int code);
extern bool wasm64_print_operand_punct_valid_p(int code);
extern void wasm64_print_operand_address(FILE *stream, rtx x);

extern rtx wasm64_function_value(const_tree ret_type, const_tree fn_decl, bool outgoing);
extern rtx wasm64_libcall_value(machine_mode mode, const_rtx fun);
extern bool wasm64_function_value_regno_p(unsigned int regno);
extern bool wasm64_promote_prototypes(const_tree fntype);

extern void wasm64_file_start (void);
extern void wasm64_start_function (FILE *stream, const char *fnname,
				  tree decl ATTRIBUTE_UNUSED);
extern void wasm64_end_function (FILE *stream, const char *fnname,
				tree decl ATTRIBUTE_UNUSED);

extern enum unwind_info_type wasm64_debug_unwind_info (void);

extern void wasm64_output_ascii (FILE *stream, const void *ptr, size_t len);

extern void wasm64_output_label (FILE *stream, const char *name);
extern void wasm64_output_debug_label (FILE *stream, const char *name, int);
extern void wasm64_output_internal_label (FILE *stream, const char *name);
extern void wasm64_output_labelref (FILE *stream, const char *name);
extern void wasm64_output_label_ref (FILE *stream, const char *name);
extern void wasm64_output_symbol_ref (FILE *stream, rtx x);
extern void wasm64_output_common (FILE *stream, const char *name, size_t size, size_t rounded);
extern void wasm64_output_skip (FILE *stream, size_t bytes);
extern void wasm64_output_local (FILE *stream, const char *name, size_t size, size_t rounded);
extern void wasm64_weaken_label (FILE *stream, const char *name);
extern void wasm64_output_def (FILE *stream, const char *alias, const char *name);

extern int wasm64_hard_regno_nregs(unsigned int regno, machine_mode mode);
extern bool wasm64_hard_regno_mode_ok(unsigned int regno, machine_mode mode);
extern bool wasm64_hard_regno_rename_ok(unsigned int from, unsigned int to);
extern int wasm64_regmode_natural_size(machine_mode mode);
extern bool wasm64_modes_tieable_p(machine_mode mode1, machine_mode mode2);

extern bool wasm64_regno_ok_for_index_p(unsigned int regno);
extern bool wasm64_regno_ok_for_base_p(unsigned int regno);
extern unsigned int wasm64_function_arg_offset(machine_mode mode, const_tree type);
extern void wasm64_init_cumulative_args(CUMULATIVE_ARGS *cum_v, const_tree fntype,
				       rtx libname,
				       const_tree fndecl, unsigned int n_named_args);

extern bool wasm64_return_in_memory(const_tree type, const_tree fntype);

extern void wasm64_expand_builtin_va_start (tree valist, rtx nextarg);
extern rtx wasm64_struct_value_rtx(tree fndecl, int incoming);
extern bool wasm64_omit_struct_return_reg(void);
extern void wasm64_promote_mode(machine_mode *modep, int *unsignedp, const_tree type);
extern machine_mode wasm64_promote_function_mode(const_tree type, machine_mode modep, int *unsignedp, const_tree funtype, int for_return);

extern unsigned wasm64_reg_parm_stack_space(const_tree fndecl);
extern unsigned wasm64_incoming_reg_parm_stack_space(const_tree fndecl);
extern unsigned wasm64_outgoing_reg_parm_stack_space(const_tree fndecl);
extern unsigned wasm64_stack_parms_in_reg_parm_area(void);

extern rtx wasm64_dynamic_chain_address(rtx);
extern rtx wasm64_incoming_return_addr_rtx();
extern rtx wasm64_return_addr_rtx(int, rtx);
extern rtx wasm64_get_drap_rtx(void);

extern rtx wasm64_expand_call (rtx, rtx, rtx);
extern rtx wasm64_expand_prologue ();
extern rtx wasm64_expand_epilogue ();
extern const char *wasm64_expand_ret_insn ();

extern int wasm64_return_pops_args(tree fundecl, tree funtype, int size);
extern int wasm64_call_pops_args(CUMULATIVE_ARGS cum);

extern int wasm64_first_parm_offset(const_tree fundecl);

extern void wasm64_output_aligned_bss (FILE *stream, const_tree tree, const char *name,


				      unsigned HOST_WIDE_INT size,
				      unsigned HOST_WIDE_INT rounded);

extern void wasm64_output_aligned_decl_local (FILE *stream, tree decl, const char *name, size_t size ATTRIBUTE_UNUSED, size_t align ATTRIBUTE_UNUSED);

extern const char *wasm64_emit_binop_assign(rtx lval, rtx a, rtx b, rtx op);
extern void wasm64_asm_named_section (const char *name, unsigned int flags,
				     tree decl);

extern int wasm64_max_conditional_execute (void);
extern bool wasm64_can_eliminate(int a, int b);
extern int wasm64_initial_elimination_offset(int a, int b);
extern int wasm64_branch_cost(bool, bool);
extern int wasm64_register_move_cost(machine_mode, reg_class_t from, reg_class_t to);
extern int wasm64_memory_move_cost(machine_mode, reg_class_t, bool in);
extern bool wasm64_function_arg_regno_p(int);
extern bool wasm64_lra_p();

struct wasm64_jsexport_opts {
  const char *jsname;
  int recurse;
};

struct wasm64_jsexport_decl {
  tree decl;
  const char *jsname;
  const char *symbol;
  vec<const char *> fragments;
};


extern void wasm64_jsexport (tree, struct wasm64_jsexport_opts *,
			    vec<struct wasm64_jsexport_decl> *);

