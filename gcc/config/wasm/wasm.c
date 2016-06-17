/* GCC backend for the asm.js target
   Copyright (C) 1991-2015 Free Software Foundation, Inc.
   Copyright (C) 2016 Pip Cet <pipcet@gmail.com>

   Based on the ARM port, which was:
   Contributed by Pieter `Tiggr' Schoenmakers (rcpieter@win.tue.nl)
   and Martin Simmons (@harleqn.co.uk).
   More major hacks by Richard Earnshaw (rearnsha@arm.com).

   This file is NOT part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

#define N_ARGREG_PASSED 2
#define N_ARGREG_GLOBAL 2

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tm_p.h"
#include "target.h"
#include "hash-set.h"
#include "machmode.h"
#include "vec.h"
#include "double-int.h"
#include "input.h"
#include "alias.h"
#include "symtab.h"
#include "wide-int.h"
#include "inchash.h"
#include "tree-core.h"
#include "stor-layout.h"
#include "tree.h"
#include "c-family/c-common.h"
#include "rtl.h"
#include "predict.h"
#include "dominance.h"
#include "cfg.h"
#include "cfgrtl.h"
#include "cfganal.h"
#include "lcm.h"
#include "cfgbuild.h"
#include "cfgcleanup.h"
#include "basic-block.h"
#include "hash-map.h"
#include "is-a.h"
#include "plugin-api.h"
#include "ipa-ref.h"
#include "function.h"
#include "cgraph.h"
#include "ggc.h"
#include "except.h"
#include "tm_p.h"
#include "target.h"
#include "sched-int.h"
#include "debug.h"
#include "langhooks.h"
#include "bitmap.h"
#include "df.h"
#include "intl.h"
#include "libfuncs.h"
#include "params.h"
#include "opts.h"
#include "dumpfile.h"
#include "gimple-expr.h"
#include "builtins.h"
#include "rtl-iter.h"
#include "sched-int.h"
#include "output.h"
#include "calls.h"
#include "config/linux-protos.h"
#include "real.h"
#include "fixed-value.h"
#include "expmed.h"
#include "dojump.h"
#include "explow.h"
#include "emit-rtl.h"
#include "stmt.h"
#include "expr.h"
#include "optabs.h"
#include "tree-chkp.h"
#include "rtl-chkp.h"
#include "print-tree.h"
#include "varasm.h"

extern void
debug_tree (tree node);

void
wasm_cpu_cpp_builtins (struct cpp_reader * pfile)
{
  cpp_assert(pfile, "cpu=wasm");
  cpp_assert(pfile, "machine=wasm");

  cpp_define(pfile, "__WASM__");
}

bool linux_libc_has_function(enum function_class cl ATTRIBUTE_UNUSED)
{
  return true;
}

void
wasm_globalize_label (FILE * stream, const char *name)
{
  asm_fprintf (stream, "\t.global %s\n", name + (name[0] == '*'));
}

void
wasm_weaken_label (FILE *stream, const char *name)
{
  asm_fprintf (stream, "\t.weak %s\n", name + (name[0] == '*'));
}

void
wasm_output_def (FILE *stream, const char *alias, const char *name)
{
  asm_fprintf (stream, "\t.set %s, %s\n", alias + (alias[0] == '*'), name + (name[0] == '*'));
}

enum reg_class wasm_regno_reg_class(int regno)
{
  if (regno < 3)
    return GENERAL_REGS;
  else if (regno < 8)
    return THREAD_REGS;
  else if (regno < 24 && (regno & 7) < max_regs_enabled)
    return GENERAL_REGS;
  else if (regno < 32 && (regno & 7) < max_regs_enabled)
    return FLOAT_REGS;
  else if (regno == 32 || regno == 34 || regno == 35)
    return GENERAL_REGS;
  else if (regno == 33)
    return THREAD_REGS;

  return NO_REGS;
}

static bool
wasm_legitimate_constant_p (machine_mode mode ATTRIBUTE_UNUSED, rtx x)
{
  return (CONST_INT_P (x)
	  || CONST_DOUBLE_P (x)
	  || CONSTANT_ADDRESS_P (x));
}

typedef struct {
  int count;
  int const_count;
  int indir;
} wasm_legitimate_address;

static bool
wasm_legitimate_address_p_rec (wasm_legitimate_address *r,
				machine_mode mode, rtx x, bool strict_p);

static bool
wasm_legitimate_address_p_rec (wasm_legitimate_address *r,
				machine_mode mode, rtx x, bool strict_p)
{
  enum rtx_code code = GET_CODE (x);

  if (MEM_P(x))
    {
      r->indir++;
      return wasm_legitimate_address_p_rec (r, mode, XEXP (x, 0), strict_p);
    }

  if (REG_P(x))
    {
      if (strict_p && REGNO (x) >= FIRST_PSEUDO_REGISTER)
	return false;

      r->count++;

      return true;
    }

  if (CONST_INT_P (x) || CONSTANT_ADDRESS_P (x))
    {
      r->const_count++;

      return true;
    }

  if (code == SYMBOL_REF)
    {
      r->const_count++;

      return true;
    }

  if (code == PLUS)
    {
      if (!wasm_legitimate_address_p_rec (r, mode, XEXP (x, 0), strict_p) ||
	  !wasm_legitimate_address_p_rec (r, mode, XEXP (x, 1), strict_p))
	return false;

      return true;
    }

  return false;
}

static bool
wasm_legitimate_address_p (machine_mode mode, rtx x, bool strict_p)
{
  wasm_legitimate_address r = { 0, 0, 0 };
  bool s;

  s = wasm_legitimate_address_p_rec (&r, mode, x, strict_p);

#if 0
  debug_rtx(x);
  fprintf(stderr, "%d %d %d %d\n",
	  s, r.count, r.const_count, r.indir);
#endif

  if (!s)
    return false;

  return (r.count <= MAX_REGS_PER_ADDRESS) && (r.count <= max_rpi) && (r.indir <= 0) && (r.const_count <= 1);
}

static int wasm_argument_count (const_tree fntype)
{
  function_args_iterator args_iter;
  tree n = NULL_TREE, t;
  int ret = 0;

  if (!fntype)
    return 0;

  FOREACH_FUNCTION_ARGS (fntype, t, args_iter)
    {
      n = t;
      ret++;
    }

  //fprintf (stderr, "tree for ret %d:\n", ret);
  //debug_tree(n);
  //fprintf (stderr, "\n\n");

  if (n && VOID_TYPE_P (n))
    ret--;

  /* This is a hack. We'd prefer to instruct gcc to pass an invisible
     structure return argument even for functions returning void, but
     that requires changes to the core code, and it frightens me a
     little to do that because it might cause obscure ICEs. */
  ret = ((n != NULL_TREE && n != void_type_node) ? -1 : 1) * ret;

  if (VOID_TYPE_P (TREE_TYPE (fntype)))
    ret ^= 0x40000000;

  return ret;
}

static bool
wasm_is_stackcall (const_tree type)
{
  if (!type)
    return false;

  tree attrs = TYPE_ATTRIBUTES (type);
  if (attrs != NULL_TREE)
    {
      if (lookup_attribute ("stackcall", attrs))
	return true;
    }

  if (wasm_argument_count (type) < 0)
    return true;

  return false;
}

static bool
wasm_is_real_stackcall (const_tree type)
{
  if (!type)
    return false;

  tree attrs = TYPE_ATTRIBUTES (type);
  if (attrs != NULL_TREE)
    {
      if (lookup_attribute ("stackcall", attrs))
	return true;
    }

  return false;
}

void wasm_init_cumulative_args(CUMULATIVE_ARGS *cum_p,
				const_tree fntype ATTRIBUTE_UNUSED,
				rtx libname ATTRIBUTE_UNUSED,
				const_tree fndecl,
				unsigned int n_named_args ATTRIBUTE_UNUSED)
{
  cum_p->n_areg = 0;
  cum_p->n_vreg = 0;
  cum_p->is_stackcall = wasm_is_stackcall (fndecl ? TREE_TYPE (fndecl) : NULL_TREE) || wasm_is_stackcall (fntype);
}

bool wasm_strict_argument_naming(cumulative_args_t cum ATTRIBUTE_UNUSED)
{
  return true;
}
bool wasm_function_arg_regno_p(int regno)
{
  /* XXX is this incoming or outgoing? */
  return ((regno >= R0_REG && regno < R0_REG + N_ARGREG_PASSED) ||
	  (regno >= A0_REG && regno < A0_REG + N_ARGREG_GLOBAL));
}

unsigned int wasm_function_arg_boundary(machine_mode mode, const_tree type)
{
  if (GET_MODE_ALIGNMENT (mode) > PARM_BOUNDARY ||
      (type && TYPE_ALIGN (type) > PARM_BOUNDARY)) {
    return PARM_BOUNDARY * 2;
  } else
    return PARM_BOUNDARY;
}
unsigned int wasm_function_arg_round_boundary(machine_mode mode, const_tree type)
{
  if (GET_MODE_ALIGNMENT (mode) > PARM_BOUNDARY ||
      (type && TYPE_ALIGN (type) > PARM_BOUNDARY)) {
    return PARM_BOUNDARY * 2;
  } else
    return PARM_BOUNDARY;
}

unsigned int wasm_function_arg_offset(machine_mode mode ATTRIBUTE_UNUSED,
				       const_tree type ATTRIBUTE_UNUSED)
{
  return 0;
}

static void
wasm_function_arg_advance(cumulative_args_t cum_v, machine_mode mode,
			   const_tree type ATTRIBUTE_UNUSED, bool named ATTRIBUTE_UNUSED)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (cum_v);

  switch (mode) {
  case QImode:
  case HImode:
  case SImode:
    cum->n_areg++;
  default:
    break;
  }
}

static rtx
wasm_function_incoming_arg (cumulative_args_t pcum_v, machine_mode mode,
			     const_tree type ATTRIBUTE_UNUSED,
			     bool named ATTRIBUTE_UNUSED)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (pcum_v);
  switch (mode) {
  case VOIDmode:
    return const0_rtx;
  case QImode:
  case HImode:
  case SImode:
    if (cum->n_areg < N_ARGREG_PASSED && !cum->is_stackcall)
      return gen_rtx_REG (mode, R0_REG + cum->n_areg);
    else if (cum->n_areg < N_ARGREG_PASSED + N_ARGREG_GLOBAL &&
	     !cum->is_stackcall)
      return gen_rtx_REG (mode, A0_REG + cum->n_areg - N_ARGREG_PASSED);
    return NULL_RTX;
  case SFmode:
  case DFmode:
  default:
    return NULL_RTX;
  }

  return NULL_RTX;
}

static rtx
wasm_function_arg (cumulative_args_t pcum_v, machine_mode mode,
		    const_tree type ATTRIBUTE_UNUSED,
		    bool named ATTRIBUTE_UNUSED)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (pcum_v);
  switch (mode) {
  case VOIDmode:
    return const0_rtx;
  case QImode:
  case HImode:
  case SImode:
    if (cum->n_areg < N_ARGREG_PASSED && !cum->is_stackcall)
      return gen_rtx_REG (mode, R0_REG + cum->n_areg);
    else if (cum->n_areg < N_ARGREG_PASSED + N_ARGREG_GLOBAL &&
	     !cum->is_stackcall)
      return gen_rtx_REG (mode, A0_REG + cum->n_areg - N_ARGREG_PASSED);
    return NULL_RTX;
  case SFmode:
  case DFmode:
  default:
    return NULL_RTX;
  }

  return NULL_RTX;
}

bool wasm_frame_pointer_required (void)
{
  return true;
}

enum unwind_info_type wasm_debug_unwind_info (void)
{
  return UI_DWARF2;
}

static void
wasm_call_args (rtx arg ATTRIBUTE_UNUSED, tree funtype)
{
  if (cfun->machine->funtype != funtype) {
    cfun->machine->funtype = funtype;
  }
}

static void
wasm_end_call_args (void)
{
  cfun->machine->funtype = NULL;
}

static struct machine_function *
wasm_init_machine_status (void)
{
  struct machine_function *p = ggc_cleared_alloc<machine_function> ();
  return p;
}

static void
wasm_option_override (void)
{
  init_machine_status = wasm_init_machine_status;
}

static tree
wasm_handle_cconv_attribute (tree *node ATTRIBUTE_UNUSED,
			      tree name ATTRIBUTE_UNUSED,
			      tree args ATTRIBUTE_UNUSED, int,
			      bool *no_add_attrs ATTRIBUTE_UNUSED)
{
  return NULL;
}

static tree
wasm_handle_bogotics_attribute (tree *node ATTRIBUTE_UNUSED,
				 tree name ATTRIBUTE_UNUSED,
				 tree args ATTRIBUTE_UNUSED, int,
				 bool *no_add_attrs ATTRIBUTE_UNUSED)
{
  return NULL;
}

static vec<struct wasm_jsexport_decl> wasm_jsexport_decls;

__attribute__((weak)) void
wasm_jsexport (tree node ATTRIBUTE_UNUSED,
		struct wasm_jsexport_opts *opts ATTRIBUTE_UNUSED,
		vec<struct wasm_jsexport_decl> *decls ATTRIBUTE_UNUSED)
{
  error("jsexport not defined for this frontend");
}

#include <print-tree.h>
#include <plugin.h>

static void wasm_jsexport_unit_callback (void *, void *)
{
  unsigned i,j;
  struct wasm_jsexport_decl *p;
  if (!wasm_jsexport_decls.is_empty())
    {
      switch_to_section (get_section (".jsexport", SECTION_MERGE|SECTION_STRINGS|1, NULL_TREE));

      FOR_EACH_VEC_ELT (wasm_jsexport_decls, i, p)
	{
	  const char **pstr;
	  const char *str;
	  int c;
	  FOR_EACH_VEC_ELT (p->fragments, j, pstr)
	    {
	      if (*pstr == NULL)
		{
		  fprintf (asm_out_file, "\t.reloc .+2,R_WASM_HEX16,%s\n",
			   p->symbol);
		  fprintf (asm_out_file, "\t.ascii \"0x0000000000000000\"\n");
		}
	      else
		{
		  fprintf (asm_out_file, "\t.ascii \"");
		  str = *pstr;
		  while ((c = *str++))
		    {
		      switch (c)
			{
			case '\"':
			  fprintf (asm_out_file, "\\\"");
			  break;

			default:
			  fprintf (asm_out_file, "%c", c);
			  break;
			}
		    }
		  fprintf (asm_out_file, "\"\n");
		}
	    }

	  fprintf (asm_out_file, "\t.asciz \"\"\n");
	}
    }

  wasm_jsexport_decls = vec<struct wasm_jsexport_decl>();
}

static void wasm_jsexport_parse_args (tree args, struct wasm_jsexport_opts *opts)
{
  while (args)
    {
      if (TREE_CODE (args) == STRING_CST)
	opts->jsname = TREE_STRING_POINTER (args);
      else if (TREE_CODE (args) == INTEGER_CST)
	opts->recurse = !compare_tree_int (args, 0);
      args = TREE_CHAIN (args);
    }
}

static void wasm_jsexport_decl_callback (void *gcc_data, void *)
{
  tree decl = (tree)gcc_data;
  bool found = false;
  struct wasm_jsexport_opts opts;

  opts.jsname = NULL;
  opts.recurse = 0;

  if (DECL_P (decl))
    {
      tree attrs = DECL_ATTRIBUTES (decl);
      tree attr;
      if (attrs != NULL_TREE)
	{
	  if ((attr = lookup_attribute ("jsexport", attrs)))
	    {
	      found = true;
	      wasm_jsexport_parse_args (TREE_VALUE (attr), &opts);
	    }
	}
    }
  else if (TYPE_P (decl))
    {
      tree attrs = TYPE_ATTRIBUTES (decl);
      tree attr;
      if (attrs != NULL_TREE)
	{
	  if ((attr = lookup_attribute ("jsexport", attrs)))
	    {
	      found = true;
	      wasm_jsexport_parse_args (TREE_VALUE (attr), &opts);
	    }
	}
    }

  if (!found)
    {
      return;
    }

  if (found)
    wasm_jsexport(decl, &opts, &wasm_jsexport_decls);
}

static bool wasm_jsexport_plugin_inited = false;

static void wasm_jsexport_plugin_init (void)
{
  register_callback("jsexport", PLUGIN_FINISH_DECL,
		    wasm_jsexport_decl_callback, NULL);
  register_callback("jsexport", PLUGIN_FINISH_TYPE,
		    wasm_jsexport_decl_callback, NULL);
  register_callback("jsexport", PLUGIN_FINISH_UNIT,
		    wasm_jsexport_unit_callback, NULL);
  flag_plugin_added = true;

  wasm_jsexport_plugin_inited = true;
}

static tree
wasm_handle_jsexport_attribute (tree * node, tree attr_name ATTRIBUTE_UNUSED,
				 tree args, int,
				 bool *no_add_attrs ATTRIBUTE_UNUSED)
{
  struct wasm_jsexport_opts opts;

  opts.jsname = NULL;
  opts.recurse = 0;

  if (!wasm_jsexport_plugin_inited)
    wasm_jsexport_plugin_init ();

  if (TREE_CODE (*node) == TYPE_DECL)
    {
      wasm_jsexport_parse_args (args, &opts);
      wasm_jsexport (*node, &opts, &wasm_jsexport_decls);
    }

  return NULL_TREE;
}

static const struct attribute_spec wasm_attribute_table[] =
{
  /* { name, min_len, max_len, decl_req, type_req, fn_type_req, handler,
       affects_type_identity } */
  /* stackcall, no regparms but argument count on the stack */
  { "stackcall", 0, 0, false, true, true, wasm_handle_cconv_attribute, true },
  { "regparm", 1, 1, false, true, true, wasm_handle_cconv_attribute, true },

  { "nobogotics", 0, 0, false, true, true, wasm_handle_bogotics_attribute, false },
  { "bogotics_backwards", 0, 0, false, true, true, wasm_handle_bogotics_attribute, false },
  { "bogotics", 0, 0, false, true, true, wasm_handle_bogotics_attribute, false },

  { "nobreak", 0, 0, false, true, true, wasm_handle_bogotics_attribute, false },
  { "breakonenter", 0, 0, false, true, true, wasm_handle_bogotics_attribute, false },
  { "fullbreak", 0, 0, false, true, true, wasm_handle_bogotics_attribute, false },
  { "jsexport", 0, 2, false, false, false, wasm_handle_jsexport_attribute, false },
  { NULL, 0, 0, false, false, false, NULL, false }
};

void wasm_print_operand_address(FILE *stream, rtx x)
{
  switch (GET_CODE (x)) {
  case MEM: {
    rtx addr = XEXP (x, 0);

    wasm_print_operand (stream, addr, 0);

    return;
  }
  default: {
    print_rtl(stream, x);
  }
  }
}

void wasm_print_op(FILE *stream, rtx x);

void wasm_print_assignment(FILE *stream, rtx x);
void wasm_print_label(FILE *stream, rtx x)
{
  switch (GET_CODE (x)) {
  case SYMBOL_REF: {
    const char *name = XSTR (x, 0);
    if (!SYMBOL_REF_FUNCTION_P (x)) {
      asm_fprintf (stream, "$\n\t.datatextlabel ");
      asm_fprintf (stream, "%s", name + (name[0] == '*'));
      asm_fprintf (stream, "\n\t");
    } else if (in_section->common.flags & SECTION_CODE) {
      asm_fprintf (stream, "$\n\t.codetextlabel ");
      asm_fprintf (stream, "%s", name + (name[0] == '*'));
      asm_fprintf (stream, "\n\t");
    } else {
      asm_fprintf (stream, "BROKEN");
      asm_fprintf (stream, "$\n\t.codetextlabel ");
      asm_fprintf (stream, "%s", name + (name[0] == '*'));
      asm_fprintf (stream, "\n\t");
    }
    break;
  }
  case LABEL_REF: {
    char buf[256];
    x = LABEL_REF_LABEL (x);
    asm_fprintf (stream, "$\n\t.codetextlabel ");
    ASM_GENERATE_INTERNAL_LABEL (buf, "L", CODE_LABEL_NUMBER (x));
    ASM_OUTPUT_LABEL_REF (stream, buf);
    asm_fprintf (stream, "\n\t");
    break;
  }
  default:
    gcc_unreachable ();
  }
}

const char *wasm_mode (machine_mode mode)
{
  switch (mode) {
  case QImode:
    return "i8";
  case HImode:
    return "i16";
  case SImode:
    return "i32";
  case DImode:
    return "i64";
  case SFmode:
    return "f32";
  case DFmode:
    return "f64";
  default:
    return "i32"; /* why is this happening? */
  }
}

const char *wasm_load (machine_mode outmode, machine_mode inmode, bool sign)
{
  switch (outmode) {
  case SImode:
    switch (inmode) {
    case SImode:
      return "i32.load";
    case HImode:
      return sign ? "i32.load16_s" : "i32.load16_u";
    case QImode:
      return sign ? "i32.load8_s" : "i32.load8_u";
    default: ;
    }
    gcc_unreachable ();
  case DImode:
    switch (inmode) {
    case DImode:
      return "i64.load";
    case SImode:
      return sign ? "i64.load32_s" : "i64.load32_u";
    case HImode:
      return sign ? "i64.load16_s" : "i64.load16_u";
    case QImode:
      return sign ? "i64.load8_s" : "i64.load8_u";
    default: ;
    }
    gcc_unreachable ();
  case SFmode:
    switch (inmode) {
    case SFmode:
      return "f32.load";
    default: ;
    }
    gcc_unreachable ();
  case DFmode:
    switch (inmode) {
    case DFmode:
      return "f64.load";
    default: ;
    }
    gcc_unreachable ();
  default: ;
  }
  gcc_unreachable ();
}

const char *wasm_store (machine_mode outmode, machine_mode inmode)
{
  switch (outmode) {
  case SImode:
    switch (inmode) {
    case SImode:
      return "i32.store";
    case HImode:
      return "i32.store16";
    case QImode:
      return "i32.store8";
    default: ;
    }
    gcc_unreachable ();
  case DImode:
    switch (inmode) {
    case DImode:
      return "i64.store";
    case SImode:
      return "i64.store32";
    case HImode:
      return "i64.store16";
    case QImode:
      return "i64.store8";
    default: ;
    }
    gcc_unreachable ();
  case SFmode:
    switch (inmode) {
    case SFmode:
      return "f32.store";
    default: ;
    }
    gcc_unreachable ();
  case DFmode:
    switch (inmode) {
    case DFmode:
      return "f64.store";
    default: ;
    }
    gcc_unreachable ();
  default: ;
  }
  gcc_unreachable ();
}

#include <print-tree.h>

void wasm_print_operation(FILE *stream, rtx x, bool want_lval)
{
  switch (GET_CODE (x)) {
  case PC: {
    asm_fprintf (stream, "pc");
    break;
  }
  case REG: {
    if (want_lval) {
      if (REGNO (x) == RV_REG)
	asm_fprintf (stream, "i32.store (i32.const 4096)");
      else if (REGNO (x) >= A0_REG && REGNO (x) <= A3_REG)
	asm_fprintf (stream, "i32.store (i32.const %d)", 4104 + 8*(REGNO (x)-A0_REG));
      else
	asm_fprintf (stream, "set_local $%s", reg_names[REGNO (x)]);
    } else {
      if (REGNO (x) == RV_REG)
	asm_fprintf (stream, "(i32.load (i32.const 4096))");
      else if (REGNO (x) >= A0_REG && REGNO (x) <= A3_REG)
	asm_fprintf (stream, "(i32.load (i32.const %d))", 4104 + 8*(REGNO (x)-A0_REG));
      else
	asm_fprintf (stream, "(get_local $%s)", reg_names[REGNO (x)]);
    }
    break;
  }
  case ZERO_EXTEND: {
    rtx mem = XEXP (x, 0);
    if (GET_CODE (mem) == MEM)
      {
	rtx addr = XEXP (mem, 0);
	if (want_lval)
	  gcc_unreachable ();

	asm_fprintf (stream, "(%s ", wasm_load (GET_MODE (x), GET_MODE (mem), false));
	wasm_print_operation (stream, addr, false);
	asm_fprintf (stream, ")");
      }
    else if (GET_CODE (mem) == SUBREG)
      {
	rtx reg = XEXP (mem, 0);

	asm_fprintf (stream, "(i32.and ");
	wasm_print_operation (stream, reg, false);
	asm_fprintf (stream, " %d)", GET_MODE (mem) == HImode ? 65535 : 255);
      }
    else if (GET_CODE (mem) == REG)
      {
	asm_fprintf (stream, "(i32.and ");
	wasm_print_operation (stream, mem, false);
	asm_fprintf (stream, " %d)", GET_MODE (mem) == HImode ? 65535 : 255);
      }
    else
      {
	print_rtl (stderr, x);
	gcc_unreachable ();
      }
    break;
  }
  case MEM: {
    rtx addr = XEXP (x, 0);

    if (want_lval) {
      machine_mode outmode = GET_MODE (x);
      if (outmode == QImode || outmode == HImode)
	outmode = SImode;
      asm_fprintf (stream, "%s ", wasm_store (outmode, GET_MODE (x)));
    } else {
      machine_mode outmode = GET_MODE (x);
      if (outmode == QImode || outmode == HImode)
	outmode = SImode;
      asm_fprintf (stream, "(%s ", wasm_load (outmode, GET_MODE (x), true));
    }
    wasm_print_operation (stream, addr, false);
    if (!want_lval)
      asm_fprintf (stream, ")");

    break;
  }
  case CONST_INT: {
    asm_fprintf (stream, "(i32.const %d)", (int) XWINT (x, 0));
    break;
  }
  case CONST_DOUBLE: {
    REAL_VALUE_TYPE r;
    char buf[512];
    long l[2];

    r = *CONST_DOUBLE_REAL_VALUE (x);
    REAL_VALUE_TO_TARGET_DOUBLE (r, l);

    real_to_decimal_for_mode (buf, &r, 510, 0, 1, DFmode);

    if (GET_MODE (x) == DFmode) {
      if (strcmp(buf, "+Inf") == 0) {
	asm_fprintf (stream, "(f64.div (f64.const 1.0) (f64.const 0.0))");
      } else if (strcmp (buf, "-Inf") == 0) {
	asm_fprintf (stream, "(f64.div (f64.const -1.0) (f64.const 0.0))");
      } else if (strcmp (buf, "+QNaN") == 0) {
	asm_fprintf (stream, "(f64.div (f64.const 0.0) (f64.const 0.0))");
      } else if (strcmp (buf, "+SNaN") == 0) {
	asm_fprintf (stream, "(f64.div (f64.const -0.0) (f64.const 0.0))");
      } else if (buf[0] == '+')
	asm_fprintf (stream, "(f64.const %s)", buf+1);
      else
	asm_fprintf (stream, "(f64.const %s)", buf);
    } else {
      if (strcmp(buf, "+Inf") == 0) {
	asm_fprintf (stream, "(f32.div (f32.const 1.0) (f32.const 0.0))");
      } else if (strcmp (buf, "-Inf") == 0) {
	asm_fprintf (stream, "(f32.div (f32.const -1.0) (f32.const 0.0))");
      } else if (strcmp (buf, "+QNaN") == 0) {
	asm_fprintf (stream, "(f32.div (f32.const 0.0) (f32.const 0.0))");
      } else if (strcmp (buf, "+SNaN") == 0) {
	asm_fprintf (stream, "(f32.div (f32.const -0.0) (f32.const 0.0))");
      } else if (buf[0] == '+')
	asm_fprintf (stream, "(f32.const %s)", buf+1);
      else
	asm_fprintf (stream, "(f32.const %s)", buf);
    }
    break;
  }
  case SYMBOL_REF: {
    const char *name = XSTR (x, 0);
    if (!SYMBOL_REF_FUNCTION_P (x)) {
      asm_fprintf (stream, "$\n\t.ndatatextlabel ");
      asm_fprintf (stream, "%s", name + (name[0] == '*'));
      asm_fprintf (stream, "\n\t");
    } else if (in_section->common.flags & SECTION_CODE) {
      asm_fprintf (stream, "$\n\t.ncodetextlabel ");
      asm_fprintf (stream, "%s", name + (name[0] == '*'));
      asm_fprintf (stream, "\n\t");
    } else {
      asm_fprintf (stream, "BROKEN");
      asm_fprintf (stream, "$\n\t.ncodetextlabel ");
      asm_fprintf (stream, "%s", name + (name[0] == '*'));
      asm_fprintf (stream, "\n\t");
    }
    break;
  }
  case LABEL_REF: {
    char buf[256];
    x = LABEL_REF_LABEL (x);
    asm_fprintf (stream, "$\n\t.codetextlabel ");
    ASM_GENERATE_INTERNAL_LABEL (buf, "L", CODE_LABEL_NUMBER (x));
    ASM_OUTPUT_LABEL_REF (stream, buf);
    asm_fprintf (stream, "\n\t");
    break;
  }
  case PLUS:
  case MINUS:
  case ASHIFT:
  case ASHIFTRT:
  case LSHIFTRT:
  case MULT:
  case DIV:
  case MOD:
  case UMOD:
  case UDIV:
  case AND:
  case IOR:
  case XOR: {
    wasm_print_op (stream, x);
    break;
  }
  case SET: {
    wasm_print_assignment (stream, x);
    break;
  }
  case EQ:
  case NE:
  case GT:
  case LT:
  case GE:
  case LE: {
    wasm_print_op (stream, x);
    break;
  }
  case GTU:
  case LTU:
  case GEU:
  case LEU: {
    wasm_print_op (stream, x);
    break;
  }
  case FLOAT:
  case FLOAT_TRUNCATE:
  case FLOAT_EXTEND:
  case FIX:
  case NEG:
  case NOT: {
    wasm_print_op (stream, x);
    break;
  }
  case CONST: {
    wasm_print_operation (stream, XEXP (x, 0), false);
    break;
  }
  default:
    asm_fprintf (stream, "BROKEN (unknown code:746): ");
    print_rtl (stream, x);
    return;
  }
}

static unsigned wasm_function_regsize(tree decl ATTRIBUTE_UNUSED);

void wasm_print_operand(FILE *stream, rtx x, int code)
{
  //print_rtl (stderr, x);
  if (code == 'O' || code == 0) {
    wasm_print_operation (stream, x, false);
    //asm_fprintf (stream, "\n/* RTL: ");
    //print_rtl(stream, x);
    //asm_fprintf (stream, "*/");
    return;
  } else if (code == 'S') {
    wasm_print_operation (stream, x, true);
    return;
  } else if (code == '/') {
    asm_fprintf (stream, "%d", wasm_function_regsize (NULL_TREE));
    return;
  } else if (code == 'L') {
    wasm_print_label (stream, x);
    return;
  }

  asm_fprintf (stream, "BROKEN: should be using wasm_print_operation");
  print_rtl (stream, x);

  return;
}

bool wasm_print_operand_punct_valid_p(int code)
{
  switch (code) {
  case 'F':
  case 'U':
  case 'L':
  case 'O':
  case 'C':
  case 'S':
    return true;
  case '!':
    return true;
  case ';':
    return true;
  case '/':
    return true;
  default:
    return false;
  }
}

struct wasm_operator {
  int code;
  machine_mode inmode;
  machine_mode outmode;

  const char *str;

  int nargs;
};

struct wasm_operator wasm_operators[] = {
  {
    PLUS, SImode, SImode,
    "add",
    2
  },
  {
    MINUS, SImode, SImode,
    "sub",
    2
  },
  {
    MULT, SImode, SImode,
    "mul",
    2
  },
  {
    DIV, SImode, SImode,
    "div_s",
    2
  },
  {
    MOD, SImode, SImode,
    "rem_s",
    2
  },
  {
    UDIV, SImode, SImode,
    "div_u",
    2
  },
  {
    UMOD, SImode, SImode,
    "rem_u",
    2
  },
  {
    XOR, SImode, SImode,
    "xor",
    2
  },
  {
    IOR, SImode, SImode,
    "or",
    2
  },
  {
    AND, SImode, SImode,
    "and",
    2
  },
  {
    ASHIFT, SImode, SImode,
    "call $shl",
    2
  },
  {
    ASHIFTRT, SImode, SImode,
    "call $shr_s",
    2
  },
  {
    LSHIFTRT, SImode, SImode,
    "call $shr_u",
    2
  },
  {
    EQ, SImode, VOIDmode,
    "eq",
    2
  },
  {
    NE, SImode, VOIDmode,
    "ne",
    2
  },
  {
    LT, SImode, VOIDmode,
    "lt_s",
    2
  },
  {
    LE, SImode, VOIDmode,
    "le_s",
    2
  },
  {
    GT, SImode, VOIDmode,
    "gt_s",
    2
  },
  {
    GE, SImode, VOIDmode,
    "ge_s",
    2
  },
  {
    LTU, SImode, VOIDmode,
    "lt_u",
    2
  },
  {
    LEU, SImode, VOIDmode,
    "le_u",
    2
  },
  {
    GTU, SImode, VOIDmode,
    "gt_u",
    2
  },
  {
    GEU, SImode, VOIDmode,
    "ge_u",
    2
  },
  {
    EQ, SImode, SImode,
    "eq",
    2
  },
  {
    NE, SImode, SImode,
    "ne",
    2
  },
  {
    LT, SImode, SImode,
    "lt_s",
    2
  },
  {
    LE, SImode, SImode,
    "le_s",
    2
  },
  {
    GT, SImode, SImode,
    "gt_s",
    2
  },
  {
    GE, SImode, SImode,
    "ge_s",
    2
  },
  {
    LTU, SImode, SImode,
    "lt_u",
    2
  },
  {
    LEU, SImode, SImode,
    "le_u",
    2
  },
  {
    GTU, SImode, SImode,
    "gt_u",
    2
  },
  {
    GEU, SImode, SImode,
    "ge_u",
    2
  },
  {
    PLUS, DFmode, DFmode,
    "add",
    2
  },
  {
    MINUS, DFmode, DFmode,
    "sub",
    2
  },
  {
    MULT, DFmode, DFmode,
    "mul",
    2
  },
  {
    DIV, DFmode, DFmode,
    "div",
    2
  },
  {
    NEG, DFmode, DFmode,
    "neg",
    1
  },
  {
    EQ, DFmode, VOIDmode,
    "eq",
    2
  },
  {
    NE, DFmode, VOIDmode,
    "ne",
    2
  },
  {
    LT, DFmode, VOIDmode,
    "lt",
    2
  },
  {
    LE, DFmode, VOIDmode,
    "le",
    2
  },
  {
    GT, DFmode, VOIDmode,
    "gt",
    2
  },
  {
    GE, DFmode, VOIDmode,
    "ge",
    2
  },
  {
    NEG, SImode, SImode,
    "sub (i32.const 0)",
    1
  },
  {
    NOT, SImode, SImode,
    "xor (i32.const -1)",
    1
  },
  {
    FIX, DFmode, SImode,
    "trunc_s_f64",
    1
  },
  {
    FLOAT, SImode, DFmode,
    "convert_s_i32",
    1
  },
  {
    FLOAT, DImode, DFmode,
    "convert_s_i32",
    1
  },
  {
    FLOAT_EXTEND, SFmode, DFmode,
    "promote_f32",
    1
  },
  {
    FLOAT_TRUNCATE, DFmode, SFmode,
    "demote_f64",
    1
  },
  {
    0, SImode, SImode,
    NULL,
    0
  }
};

bool modes_compatible (machine_mode mode1, machine_mode mode2)
{
  return mode1 == mode2 || mode1 == VOIDmode || mode2 == VOIDmode;
}

void wasm_print_op(FILE *stream, rtx x)
{
  struct wasm_operator *oper;
  rtx a = XEXP (x, 0);
  rtx b;

  for (oper = wasm_operators; oper->code; oper++) {
    if (oper->code == GET_CODE (x) &&
	oper->outmode == GET_MODE (x) &&
	modes_compatible(GET_MODE (a), oper->inmode) &&
	(oper->nargs == 1 || modes_compatible (GET_MODE (XEXP (x,1)), oper->inmode)))
      break;
  }

  if (oper->code == 0) {
    gcc_unreachable ();
  }

  if (strncmp (oper->str, "call ", 5))
    asm_fprintf (stream, "(%s.%s ", wasm_mode (GET_MODE (x)), oper->str);
  else
    asm_fprintf (stream, "(%s ", oper->str);
  wasm_print_operation(stream, a, false);
  if (oper->nargs == 2) {
    b = XEXP (x, 1);

    asm_fprintf (stream, " ");
    wasm_print_operation(stream, b, false);
  }
  asm_fprintf (stream, ")");
}

void wasm_print_assignment(FILE *stream, rtx x)
{
  asm_fprintf (stream, "(");
  wasm_print_operation (stream, XEXP (x, 0), true);
  asm_fprintf (stream, " ");
  wasm_print_operation (stream, XEXP (x, 1), false);
  asm_fprintf (stream, ")");
}

rtx wasm_function_value(const_tree ret_type, const_tree fn_decl ATTRIBUTE_UNUSED,
			 bool outgoing ATTRIBUTE_UNUSED)
{
  if (!use_rv_register)
    return NULL_RTX;

  /* XXX does this work for DImode? */
  switch (TYPE_MODE (ret_type)) {
  case SFmode:
  case DFmode:
  default:
    return NULL_RTX;
  case QImode:
  case HImode:
  case SImode:
    return gen_rtx_REG (SImode, RV_REG);
  }
}

rtx wasm_struct_value_rtx(tree fndecl ATTRIBUTE_UNUSED,
			   int incoming ATTRIBUTE_UNUSED)
{
  return NULL_RTX;
}

bool wasm_return_in_memory(const_tree type,
			    const_tree fntype ATTRIBUTE_UNUSED)
{
  if (use_rv_register)
    switch (TYPE_MODE (type)) {
    case QImode:
    case HImode:
    case SImode:
      return false;
    default:
      return true;
    }
  else
    return true;
}

bool wasm_omit_struct_return_reg(void)
{
  if (use_rv_register)
    return false;
  else
    return true;
}

rtx wasm_get_drap_rtx(void)
{
  return NULL_RTX;
}

rtx wasm_libcall_value(machine_mode mode, const_rtx fun ATTRIBUTE_UNUSED)
{
  if (!use_rv_register)
    return NULL_RTX;

  /* XXX does this work for DImode? */
  switch (mode) {
  case SFmode:
  case DFmode:
    return NULL_RTX;
  default:
    return gen_rtx_REG (mode, RV_REG);
  }
}

bool wasm_function_value_regno_p(unsigned int regno)
{
  if (use_rv_register)
    return regno == RV_REG;
  else
    return false;
}

bool wasm_promote_prototypes(const_tree fntype ATTRIBUTE_UNUSED)
{
  return true;
}

void wasm_promote_mode(machine_mode *modep,
			int *punsignedp ATTRIBUTE_UNUSED,
			const_tree type ATTRIBUTE_UNUSED)
{
  switch (*modep) {
  case QImode:
  case HImode:
  case SImode:
    *modep = SImode;
    break;
  default:
    break;
  }
}

machine_mode wasm_promote_function_mode(const_tree type ATTRIBUTE_UNUSED,
					 machine_mode mode,
					 int *punsignedp ATTRIBUTE_UNUSED,
					 const_tree funtype ATTRIBUTE_UNUSED,
					 int for_return)
{
  if (for_return == 1) {
    switch (mode) {
    case VOIDmode:
    case QImode:
    case HImode:
    case SImode:
      return SImode;
    default:
      return mode;
    }
  } else if (for_return == 2) {
    switch (mode) {
    case QImode:
    case HImode:
    case SImode:
      return SImode;
    default:
      return mode;
    }
  }

  return mode;
}

unsigned wasm_reg_parm_stack_space(const_tree fndecl ATTRIBUTE_UNUSED)
{
  return 0;
}

unsigned wasm_incoming_reg_parm_stack_space(const_tree fndecl ATTRIBUTE_UNUSED)
{
  return 0;
}

unsigned wasm_outgoing_reg_parm_stack_space(const_tree fndecl ATTRIBUTE_UNUSED)
{
  return 0;
}

void wasm_file_start(void)
{
  fputs ("#NO_APP\n", asm_out_file);
  switch (pc_type) {
  case PC_STRICT:
    fputs ("\t.set __gcc_pc_update, 2\n", asm_out_file);
    break;
  case PC_MEDIUM:
    fputs ("\t.set __gcc_pc_update, 1\n", asm_out_file);
    break;
  case PC_LAX:
  default:
    fputs ("\t.set __gcc_pc_update, 0\n", asm_out_file);
    break;
  }
  fputs ("\t.set __gcc_bogotics_backwards, 0\n", asm_out_file);
  fputs ("\t.include \"wasm-macros.s\"\n", asm_out_file);
  default_file_start ();
}

static unsigned wasm_function_regmask(tree decl ATTRIBUTE_UNUSED)
{
  unsigned ret = 0;

  int i;
  /* XXX work out why calls_eh_return functions use additional
   * registers. */
  if (crtl->calls_eh_return)
    ret |= 0xfffffff;
  for (i = 8 /* R0_REG */; i <= 31 /* F7_REG */; i++)
    {
      if (df_regs_ever_live_p (i))
	ret |= (1<<(i-4));
    }

  return ret;
}

static unsigned wasm_function_regsize(tree decl ATTRIBUTE_UNUSED)
{
  unsigned mask = wasm_function_regmask (decl);
  unsigned size = 0;

  size += 4;
  size += 4;
  size += 4;
  size += 4;
  size += 4;
  size += 4;

  int i;
  for (i = 4 /* A0_REG */; i <= 31 /* F7_REG */;  i++)
    {
      if (mask & (1<<(i-4)))
	{
	  if (wasm_regno_reg_class (i) == FLOAT_REGS)
	    {
	      size += size&4;
	      size += 8;
	    }
	  else
	    {
	      size += 4;
	    }
	}
    }

  size += size&4;

  return size;
}

static unsigned wasm_function_regstore(FILE *stream,
					tree decl ATTRIBUTE_UNUSED)
{
  unsigned mask = wasm_function_regmask (decl);
  unsigned size = 0;
  unsigned total_size = wasm_function_regsize (decl);

  asm_fprintf (stream, "\t(block\n");
  asm_fprintf (stream, "\t\t(if (i32.ne (i32.and (get_local $rp) (i32.const 3)) (i32.const 1)) (return (get_local $rp)))\n");
  asm_fprintf (stream, "\t\t(i32.store (i32.add (get_local $fp) (i32.const %d)) (i32.add (get_local $fp) (i32.const %d)))\n", size, total_size);
  size += 4;

  asm_fprintf (stream, "\t\t(i32.store (i32.add (get_local $fp) (i32.const %d)) (i32.shl (get_local $pc0) (i32.const 4)))\n", size);
  size += 4;

  asm_fprintf (stream, "\t\t(i32.store (i32.add (get_local $fp) (i32.const %d)) (i32.shl (i32.add (get_local $pc0) (get_local $dpc)) (i32.const 4)))\n", size);
  size += 4;

  asm_fprintf (stream, "\t\t(i32.store (i32.add (get_local $fp) (i32.const %d)) (get_local $rpc))\n", size);
  size += 4;

  asm_fprintf (stream, "\t\t(i32.store (i32.add (get_local $fp) (i32.const %d)) (get_local $sp))\n", size);
  size += 4;

  asm_fprintf (stream, "\t\t(i32.store (i32.add (get_local $fp) (i32.const %d)) (i32.const %d))\n", size, mask);
  size += 4;

  int i;
  for(i = 4 /* R0_REG */; i <= 31 /* F7_REG */; i++)
    {
      if (mask & (1<<(i-4)))
	{
	  if (wasm_regno_reg_class (i) == FLOAT_REGS)
	    {
	      size += size&4;
	      asm_fprintf(stream, "\t\t(f64.store (i32.add (get_local $fp) (i32.const %d)) (get_local $%s))\n",
			  size, reg_names[i]);
	      size += 8;
	    }
	  else
	    {
	      if (i >= 8)
		asm_fprintf(stream, "\t\t(i32.store (i32.add (get_local $fp) (i32.const %d)) (get_local $%s))\n",
			    size, reg_names[i]);
	      size += 4;
	    }
	}
    }

  asm_fprintf (stream, "\t\t(return (get_local $rp))\n");
  asm_fprintf (stream, "\t)\n");
  size += size&4;
  return size;
}

static unsigned wasm_function_regload(FILE *stream,
					tree decl ATTRIBUTE_UNUSED)
{
  unsigned mask = wasm_function_regmask (decl);
  unsigned size = 0;
  unsigned total_size = wasm_function_regsize (decl);

  size += 4;

  asm_fprintf (stream, "\t\t(set_local $pc0 (i32.shr_u (i32.load (i32.add (get_local $rp) (i32.const %d))) (i32.const 4)))\n", size);
  size += 4;

  asm_fprintf (stream, "\t\t(set_local $dpc (i32.sub (i32.shr_u (i32.load (i32.add (get_local $rp) (i32.const %d))) (i32.const 4)) (get_local $pc0)))\n", size);
  size += 4;

  asm_fprintf (stream, "\t\t(set_local $rpc (i32.load (i32.add (get_local $rp) (i32.const %d))))\n", size);
  size += 4;

  asm_fprintf (stream, "\t\t(set_local $sp (i32.load (i32.add (get_local $rp) (i32.const %d))))\n", size);
  size += 4;

  size += 4;

  int i;
  for(i = 4 /* R0_REG */; i <= 31 /* F7_REG */; i++)
    {
      if (mask & (1<<(i-4)))
	{
	  if (wasm_regno_reg_class (i) == FLOAT_REGS)
	    {
	      size += size&4;
	      asm_fprintf (stream, "\t\t(set_local $%s (f64.load (i32.add (get_local $rp) (i32.const %d))))\n",
			   reg_names[i], size);
	      size += 8;
	    }
	  else
	    {
	      if (i >= 8)
		asm_fprintf (stream, "\t\t(set_local $%s (i32.load (i32.add (get_local $rp) (i32.const %d))))\n",
			     reg_names[i], size);
	      size += 4;
	    }
	}
    }

  size += size&4;
  if (size != total_size)
    gcc_unreachable();
  return size;
}

void wasm_start_function(FILE *f, const char *name, tree decl)
{
  char *cooked_name = (char *)alloca(strlen(name)+1);
  const char *p = name;
  char *q = cooked_name;
  tree type = decl ? TREE_TYPE (decl) : NULL;
  enum wasm_bogotics_type bt_type = bogotics_type;

  (void) bt_type;

  if (type)
    {
      tree attrs = TYPE_ATTRIBUTES (type);

      if (attrs)
	{
	  if (lookup_attribute("nobogotics", attrs))
	    bt_type = BOGOTICS_NONE;

	  if (lookup_attribute("bogotics", attrs))
	    bt_type = BOGOTICS_ALL;

	  if (lookup_attribute("bogotics_backwards", attrs))
	    bt_type = BOGOTICS_BACKWARDS;
	}
    }

  for (p = name; *p; p++)
      *q++ = *p;

  *q = 0;

  while (cooked_name[0] == '*')
    cooked_name++;

  asm_fprintf(f, "\t.popsection\n");
  asm_fprintf(f, "\t.p2align 4+8\n");
  asm_fprintf(f, "\t.pushsection .wasm_pwas%%S,\"a\"\n");
  asm_fprintf(f, "(\n");
  asm_fprintf(f, "\t.wasmtextlabeldeffirst %s\n", cooked_name);
  asm_fprintf(f, "\t\"f_$\n");
  asm_fprintf(f, "\t.textlabel %s\n", cooked_name);
  asm_fprintf(f, "\"\n");
  wasm_function_regstore(f, decl);
  asm_fprintf(f, "\t(call_import $cp $\n\t.ncodetextlabel %s\n\t)\n", cooked_name);
  asm_fprintf(f, "\t(set_local $sp (i32.add (get_local $sp1) (i32.const -16)))\n");
}

static void
wasm_define_function(FILE *f, const char *name, tree decl ATTRIBUTE_UNUSED)
{
  char *cooked_name = (char *)alloca(strlen(name)+1);
  const char *p = name;
  char *q = cooked_name;

  for (p = name; *p; p++)
    *q++ = *p;

  *q = 0;

  while (cooked_name[0] == '*')
    cooked_name++;

  asm_fprintf(f, "\t.section .special.define,\"a\"\n");
  asm_fprintf(f, "\t.pushsection .javascript%%S,\"a\"\n");
  asm_fprintf(f, "\t.ascii \"\\tdeffun({name: \\\"f_\"\n\t.codetextlabel .L.%s\n\t.ascii \"\\\", symbol: \\\"%s\\\", pc0: \"\n\t.codetextlabel .L.%s\n\t.ascii \", pc1: \"\n\t.codetextlabel .L.ends.%s\n\t.ascii \", regsize: %d, regmask: 0x%x});\\n\"\n", cooked_name, cooked_name, cooked_name, cooked_name, wasm_function_regsize(decl), wasm_function_regmask(decl));
  asm_fprintf(f, "\t.popsection\n");
}

static void
wasm_fpswitch_function(FILE *f, const char *name, tree decl ATTRIBUTE_UNUSED)
{
  char *cooked_name = (char *)alloca(strlen(name)+1);
  const char *p = name;
  char *q = cooked_name;

  for (p = name; *p; p++)
    *q++ = *p;

  *q = 0;

  while (cooked_name[0] == '*')
    cooked_name++;

  asm_fprintf(f, "\t.section .special.fpswitch,\"a\"\n");
  asm_fprintf(f, "\t.pushsection .wasm_pwas%%S,\"a\"\n");
  asm_fprintf(f, "\t.rept (.L.ends.%s-.L.%s+4096)/4096\n", cooked_name, cooked_name);
  asm_fprintf(f, "\t(return (call $f_$\n\t.codetextlabel .L.%s\n\t (get_local $dpc) (get_local $sp1) (get_local $r0) (get_local $r1) (get_local $rpc) (get_local $pc0)))\n", cooked_name);
  asm_fprintf(f, "\t.endr\n");
  asm_fprintf(f, "\t.popsection\n");
}
void wasm_end_function(FILE *f, const char *name, tree decl ATTRIBUTE_UNUSED)
{
  char *cooked_name = (char *)alloca(strlen(name)+1);
  const char *p = name;
  char *q = cooked_name;
  enum wasm_bogotics_type bt_type = bogotics_type;

  for (p = name; *p; p++)
    *q++ = *p;

  *q = 0;

  while (cooked_name[0] == '*')
    cooked_name++;

  asm_fprintf(f, "\t.wasmtextlabeldeflast .L.ends.%s\n", cooked_name);
  //asm_fprintf(f, "(if (i32.ne (i32.add $dpc $pc0) (i32.const 0)) (call_import $abort (i32.const 0) (i32.const 0) (i32.const 0) (i32.const 0)))\n");
  asm_fprintf(f, "\t(set_local $rp (i32.add (get_local $sp1) (i32.const -16)))\n");
  wasm_function_regload (f, decl);
  asm_fprintf(f, "\t(set_local $fp (get_local $rp))\n");
  asm_fprintf(f, ")\n");

  wasm_define_function(f, name, decl);
  wasm_fpswitch_function(f, name, decl);
}

void wasm_output_ascii (FILE *f, const void *ptr, size_t len)
{
  const unsigned char *bytes = (const unsigned char *)ptr;

  if (len) {
    size_t i;

    asm_fprintf(f, "\t.QI ");

    for (i=0; i<len; i++) {
      asm_fprintf(f, "%d%c", bytes[i], i == len-1 ? '\n' : ',');
    }
  }
}

void wasm_output_label (FILE *stream, const char *name)
{
  fprintf(stream, "%s:\n", name + (name[0] == '*'));
}

void wasm_output_debug_label (FILE *stream, const char *prefix, int num)
{
  fprintf(stream, "\t.labeldef_debug .%s%d\n", prefix, num);
}

void wasm_output_internal_label (FILE *stream, const char *name)
{
  if (in_section && in_section->common.flags & SECTION_CODE) {
    if (strncmp(name, ".Letext", strlen(".Letext")) == 0)
      fprintf(stream, "%s:\n", name + (name[0] == '*'));
    else
      fprintf(stream, "\t.labeldef_internal %s\n", name + (name[0] == '*'));
  } else if (strncmp(name, ".LSFDE", 6) == 0)
    fprintf(stream, "%s:\n", name + (name[0] == '*'));
  else
    fprintf(stream, "%s:\n", name + (name[0] == '*'));
}

void wasm_output_labelref (FILE *stream, const char *name)
{
  fprintf(stream, "%s", name + (name[0] == '*'));
}

void wasm_output_label_ref (FILE *stream, const char *name)
{
  fprintf(stream, "%s", name + (name[0] == '*'));
}

void wasm_output_symbol_ref (FILE *stream, rtx x)
{
  const char *name = XSTR (x, 0);
  fprintf(stream, "%s", name + (name[0] == '*'));
}

void
wasm_asm_named_section (const char *name, unsigned int flags,
			 tree decl)
{
  char flagchars[10], *f = flagchars;

  /* If we have already declared this section, we can use an
     abbreviated form to switch back to it -- unless this section is
     part of a COMDAT groups, in which case GAS requires the full
     declaration every time.  */
  if (!(HAVE_COMDAT_GROUP && (flags & SECTION_LINKONCE))
      && (flags & SECTION_DECLARED))
    {
      fprintf (asm_out_file, "\t.section\t%s\n", name);
      return;
    }

  if (!(flags & SECTION_DEBUG))
    *f++ = 'a';
#if defined (HAVE_GAS_SECTION_EXCLUDE) && HAVE_GAS_SECTION_EXCLUDE == 1
  if (flags & SECTION_EXCLUDE)
    *f++ = 'e';
#endif
  if (flags & SECTION_WRITE)
    *f++ = 'w';
  if (flags & SECTION_CODE)
    *f++ = 'x';
  if (flags & SECTION_SMALL)
    *f++ = 's';
  if (flags & SECTION_MERGE && (!(flags&SECTION_CODE)))
    *f++ = 'M';
  if (flags & SECTION_STRINGS)
    *f++ = 'S';
#ifdef TLS_SECTION_ASM_FLAG
  if (flags & SECTION_TLS)
    *f++ = TLS_SECTION_ASM_FLAG;
#endif
  if (HAVE_COMDAT_GROUP && (flags & SECTION_LINKONCE) && (!(flags &SECTION_CODE)))
    *f++ = 'G';
  *f = '\0';

  fprintf (asm_out_file, "\t.section\t%s,\"%s\"", name, flagchars);

  if (!(flags & SECTION_NOTYPE))
    {
      const char *type;
      const char *format;

      if (flags & SECTION_BSS)
	type = "nobits";
      else
	type = "progbits";

      format = ",@%s";
      /* On platforms that use "@" as the assembly comment character,
	 use "%" instead.  */
      if (strcmp (ASM_COMMENT_START, "@") == 0)
	format = ",%%%s";
      fprintf (asm_out_file, format, type);

      if (flags & SECTION_ENTSIZE)
	fprintf (asm_out_file, ",%d", flags & SECTION_ENTSIZE);
      if (HAVE_COMDAT_GROUP && (flags & SECTION_LINKONCE) && (!(flags&SECTION_CODE)))
	{
	  if (TREE_CODE (decl) == IDENTIFIER_NODE)
	    fprintf (asm_out_file, ",%s,comdat", IDENTIFIER_POINTER (decl));
	  else
	    fprintf (asm_out_file, ",%s,comdat",
		     IDENTIFIER_POINTER (DECL_COMDAT_GROUP (decl)));
	}
    }

  putc ('\n', asm_out_file);

  if (flags & SECTION_CODE)
    {
      fprintf (asm_out_file, "\t.pushsection .wasm_pwas%%S,\"a\"\n");
    }
}

void wasm_output_common (FILE *stream, const char *name, size_t size ATTRIBUTE_UNUSED, size_t rounded)
{
  fprintf (stream, "\t.comm ");
  assemble_name(stream, name);
  fprintf (stream, ", %d", (int)rounded);
  fprintf (stream, "\n");
}

void wasm_output_aligned_decl_local (FILE *stream, tree decl ATTRIBUTE_UNUSED, const char *name, size_t size ATTRIBUTE_UNUSED, size_t align ATTRIBUTE_UNUSED)
{
  fprintf (stream, "\t.local %s\n", name);
}

void wasm_output_local (FILE *stream, const char *name, size_t size ATTRIBUTE_UNUSED, size_t rounded ATTRIBUTE_UNUSED)
{
  fprintf (stream, "\t.local ");
  assemble_name(stream, name);
  fprintf (stream, "\n");
  fprintf (stream, "\t.comm ");
  assemble_name(stream, name);
  fprintf (stream, ", %d", (int)rounded);
  fprintf (stream, "\n");
}

void wasm_output_skip (FILE *stream, size_t bytes)
{
  size_t n;
  for (n = 0; n < bytes; n++) {
    fprintf (stream, "\t.QI 0\n");
  }
}

void wasm_output_aligned_bss (FILE *stream, const_tree tree ATTRIBUTE_UNUSED, const char *name,
			       unsigned HOST_WIDE_INT size,
			       unsigned HOST_WIDE_INT rounded ATTRIBUTE_UNUSED)
{
  fprintf (stream, ".aligned-bss %s %d %d\n", name,
	   (int) size, (int) rounded);
}

bool wasm_hard_regno_mode_ok(unsigned int regno, machine_mode mode)
{
  if (regno >= FIRST_PSEUDO_REGISTER)
    return false;

  if (mode == DImode)
    return false;

  if (regno >= 24 && regno <= 31)
    return FLOAT_MODE_P (mode);
  else
    return !FLOAT_MODE_P (mode);

  return true;
}

bool wasm_hard_regno_rename_ok(unsigned int from, unsigned int to)
{
  return wasm_regno_reg_class(from) == wasm_regno_reg_class(to);
}

bool wasm_modes_tieable_p(machine_mode mode1, machine_mode mode2)
{
  return ((mode1 == DFmode) ==
	  (mode2 == DFmode));
}

bool wasm_regno_ok_for_index_p(unsigned int regno)
{
  if (regno >= FIRST_PSEUDO_REGISTER)
    return false;

  return wasm_regno_reg_class(regno) == GENERAL_REGS;
}

bool wasm_regno_ok_for_base_p(unsigned int regno)
{
  if (regno >= FIRST_PSEUDO_REGISTER)
    return false;

  return wasm_regno_reg_class(regno) == GENERAL_REGS;
}

/* Uh, I don't understand the documentation for this. Using anything
 * but 4 breaks df_read_modify_subreg. */
int wasm_regmode_natural_size(machine_mode ATTRIBUTE_UNUSED mode)
{
  return 4;
}

int wasm_hard_regno_nregs(unsigned int regno, machine_mode mode)
{
  if (wasm_regno_reg_class(regno) == FLOAT_REGS &&
      mode == DFmode)
    return 1;
  else
    return (GET_MODE_SIZE(mode) + UNITS_PER_WORD - 1)/UNITS_PER_WORD;
}

static int wasm_register_priority(int regno)
{
  switch (wasm_regno_reg_class (regno)) {
  case GENERAL_REGS:
    if (regno >= R0_REG || regno < R0_REG + N_ARGREG_PASSED)
      return 0;
    return 8;
  case FLOAT_REGS:
    return 8;
  case THREAD_REGS:
    return 16;
  default:
    return 8;
  }
}


void wasm_expand_builtin_va_start (tree valist, rtx nextarg)
{
  rtx va_r = expand_expr (valist, NULL_RTX, VOIDmode, EXPAND_WRITE);
  convert_move (va_r, gen_rtx_PLUS (SImode, nextarg, gen_rtx_CONST_INT (SImode, 0)), 0);

  /* We do not have any valid bounds for the pointer, so
     just store zero bounds for it.  */
  if (chkp_function_instrumented_p (current_function_decl))
    chkp_expand_bounds_reset_for_mem (valist,
				      make_tree (TREE_TYPE (valist),
						 nextarg));
}

int wasm_return_pops_args(tree fundecl ATTRIBUTE_UNUSED,
			   tree funtype ATTRIBUTE_UNUSED,
			   int size ATTRIBUTE_UNUSED)
{
  return 0;
}

int wasm_call_pops_args(CUMULATIVE_ARGS size ATTRIBUTE_UNUSED)
{
  return 0;
}

rtx wasm_dynamic_chain_address(rtx frameaddr)
{
  return gen_rtx_MEM (SImode, frameaddr);
}

rtx wasm_incoming_return_addr_rtx(void)
{
  return gen_rtx_REG (SImode, RPC_REG);
  //return gen_rtx_MEM (SImode, plus_constant (SImode, gen_rtx_MEM (SImode, frame_pointer_rtx), 8));
}

rtx wasm_return_addr_rtx(int count ATTRIBUTE_UNUSED, rtx frameaddr)
{
  if (count == 0)
    return gen_rtx_ASHIFT (SImode, gen_rtx_REG (SImode, RPC_REG), gen_rtx_CONST_INT (SImode, 4));
  else
    {
      return gen_rtx_MEM (SImode, gen_rtx_PLUS (SImode, gen_rtx_MEM (SImode, wasm_dynamic_chain_address(frameaddr)), gen_rtx_CONST_INT (SImode, 8)));
    }
}

int wasm_first_parm_offset(const_tree fntype ATTRIBUTE_UNUSED)
{
  return 0;
}

rtx wasm_expand_call (rtx retval, rtx address, rtx callarg1)
{
  int argcount;
  rtx use = NULL, call;
  rtx_insn *call_insn;
  rtx sp = gen_rtx_REG (SImode, SP_REG);
  tree funtype = cfun->machine->funtype;

  argcount = wasm_argument_count (funtype);

  if (wasm_is_real_stackcall (funtype))
    {
      rtx loc = gen_rtx_MEM (SImode,
			     gen_rtx_PLUS (SImode,
					   sp,
					   gen_rtx_CONST_INT (SImode,
							      -4)));

      emit_move_insn (loc, gen_rtx_CONST_INT (SImode, argcount));
    }

  if (wasm_is_real_stackcall (funtype))
    {
      rtx loc = gen_rtx_MEM (SImode,
			     gen_rtx_PLUS (SImode,
					   sp,
					   gen_rtx_CONST_INT (SImode,
							      -8)));
      emit_move_insn (loc, const0_rtx);
    }

  call = gen_rtx_CALL (retval ? SImode : VOIDmode, address, callarg1);

  if (retval)
    call = gen_rtx_SET (gen_rtx_REG (SImode, RV_REG), call);
  else
    clobber_reg (&use, gen_rtx_REG (SImode, RV_REG));

  clobber_reg (&use, gen_rtx_REG (SImode, RV_REG)); // XXX
  use_reg (&use, gen_rtx_REG (SImode, SP_REG)); // XXX
  call_insn = emit_call_insn (call);

  if (use)
    CALL_INSN_FUNCTION_USAGE (call_insn) = use;

  return call_insn;
}

/*
 * Stack layout
 *              stack args
 * AP,CFA   ->
 *              unused
 *              unused
 *              unused
 *              old FP
 *              registers as specified
 *              size of regblock
 *              current SP
 *              current PC
 *              bitmask
 * FP,SP    ->
 */

rtx wasm_expand_prologue()
{
  HOST_WIDE_INT size = get_frame_size () + crtl->outgoing_args_size;
  rtx sp = gen_rtx_REG (SImode, SP_REG);
  size = (size + 7) & -8;
  int regsize = wasm_function_regsize (NULL_TREE);
  rtx insn;

  RTX_FRAME_RELATED_P (insn = emit_move_insn (sp, plus_constant (Pmode, sp, -regsize))) = 0;
  RTX_FRAME_RELATED_P (insn = emit_move_insn (frame_pointer_rtx, sp)) = 1;
  add_reg_note (insn, REG_CFA_DEF_CFA,
		plus_constant (Pmode, frame_pointer_rtx, 0));
  add_reg_note (insn, REG_CFA_EXPRESSION, gen_rtx_SET (gen_rtx_MEM (Pmode, gen_rtx_PLUS (Pmode, sp, gen_rtx_MINUS (Pmode, gen_rtx_MEM (Pmode, sp), sp))), frame_pointer_rtx));
  add_reg_note (insn, REG_CFA_OFFSET, gen_rtx_SET (gen_rtx_MEM (Pmode, plus_constant (Pmode, sp, 8)), pc_rtx));

  if (crtl->calls_eh_return)
    {
      add_reg_note (insn, REG_CFA_OFFSET, gen_rtx_SET (gen_rtx_MEM (SImode, plus_constant (Pmode, sp, 24)), gen_rtx_REG (SImode, A0_REG)));
      add_reg_note (insn, REG_CFA_OFFSET, gen_rtx_SET (gen_rtx_MEM (SImode, plus_constant (Pmode, sp, 28)), gen_rtx_REG (SImode, A1_REG)));
      add_reg_note (insn, REG_CFA_OFFSET, gen_rtx_SET (gen_rtx_MEM (SImode, plus_constant (Pmode, sp, 32)), gen_rtx_REG (SImode, A2_REG)));
      add_reg_note (insn, REG_CFA_OFFSET, gen_rtx_SET (gen_rtx_MEM (SImode, plus_constant (Pmode, sp, 36)), gen_rtx_REG (SImode, A3_REG)));
    }

  if (size != 0)
    RTX_FRAME_RELATED_P (emit_move_insn (sp, gen_rtx_PLUS (SImode, sp, gen_rtx_CONST_INT (SImode, -size)))) = 0;

  return NULL;
}

rtx wasm_expand_epilogue()
{
  if (crtl->calls_eh_return)
    {
      HOST_WIDE_INT size = 16;

      for (int i = A0_REG; i <= A3_REG; i++)
	{
	  emit_insn(gen_rtx_SET (gen_rtx_REG (SImode, 8), plus_constant (SImode, frame_pointer_rtx, size)));
	  emit_insn(gen_rtx_SET (gen_rtx_REG (SImode, i), gen_rtx_MEM (SImode,gen_rtx_REG (SImode, 8))));
	  size += 4;
	}
    }

  emit_jump_insn (ret_rtx);

  return NULL;
}

const char *wasm_expand_ret_insn()
{
  char buf[1024];
  snprintf (buf, 1022, "(return (i32.add (get_local $fp) (i32.const %d)))\n\t.set __wasm_fallthrough, 0",
	    wasm_function_regsize (NULL_TREE));

  return ggc_strdup (buf);
}

int wasm_max_conditional_execute()
{
  return max_conditional_insns;
}

bool wasm_can_eliminate(int reg0, int reg1)
{
  if (reg0 == AP_REG && reg1 == FP_REG)
    return true;
  if (reg0 == AP_REG && reg1 == SP_REG)
    return true;

  return false;
}

int wasm_initial_elimination_offset(int reg0, int reg1)
{
  if (reg0 == AP_REG && reg1 == FP_REG)
    return 16 + wasm_function_regsize (NULL_TREE);
  else if (reg0 == FP_REG && reg1 == AP_REG)
    return -16 - wasm_function_regsize (NULL_TREE);
  if (reg0 == FP_REG && reg1 == SP_REG)
    return 0;
  else if (reg0 == SP_REG && reg1 == FP_REG)
    return 0;
  if (reg0 == AP_REG && reg1 == SP_REG)
    return 16 + wasm_function_regsize (NULL_TREE);
  else if (reg0 == SP_REG && reg1 == AP_REG)
    return -16 - wasm_function_regsize (NULL_TREE);
  else
    gcc_unreachable ();
}

int wasm_branch_cost(bool speed_p ATTRIBUTE_UNUSED,
		      bool predictable_p ATTRIBUTE_UNUSED)
{
  return my_branch_cost;
}

int wasm_register_move_cost(machine_mode mode ATTRIBUTE_UNUSED,
			     reg_class_t from,
			     reg_class_t to)
{
  if (from == GENERAL_REGS && to == GENERAL_REGS)
    return my_register_cost;
  if ((from == GENERAL_REGS && to == THREAD_REGS) ||
      (from == THREAD_REGS && to == GENERAL_REGS))
    return 4 * my_register_cost;
  if (from == THREAD_REGS && to == THREAD_REGS)
    return 7 * my_register_cost;
  return my_register_cost;
}

int wasm_memory_move_cost(machine_mode mode ATTRIBUTE_UNUSED,
			   reg_class_t from ATTRIBUTE_UNUSED,
			   bool in ATTRIBUTE_UNUSED)
{
  return my_memory_cost;
}

bool wasm_rtx_costs (rtx x, machine_mode mode ATTRIBUTE_UNUSED,
		      int outer_code ATTRIBUTE_UNUSED,
		      int opno ATTRIBUTE_UNUSED, int *total,
		      bool speed ATTRIBUTE_UNUSED)
{
  switch (GET_CODE (x)) {
  case MULT:
    // (*total == COSTS_N_INSNS (5))
    //*total = COSTS_N_INSNS (1);
    break;
  case DIV:
  case UDIV:
  case MOD:
  case UMOD:
    if (*total == COSTS_N_INSNS (7))
      *total = COSTS_N_INSNS (1);
    break;
  default:
    break;
  }

  return true;
}

/* Always use LRA, as is now recommended. */

bool wasm_lra_p ()
{
  return true;
}


bool wasm_cxx_library_rtti_comdat(void)
{
  return true;
}

bool wasm_cxx_class_data_always_comdat(void)
{
  return true;
}


static rtx
wasm_trampoline_adjust_address (rtx addr)
{
  return addr;
}

/* Trampolines are merely three-word blocks of data aligned to a
 * 16-byte boundary; "TRAM" followed by a function pointer followed by
 * a value to load in the static chain register.  The JavaScript code
 * handles the recognition and evaluation of trampolines, so it's
 * quite slow. */

static void
wasm_trampoline_init (rtx m_tramp, tree fndecl, rtx static_chain)
{
  rtx fnaddr = force_reg (Pmode, XEXP (DECL_RTL (fndecl), 0));
  m_tramp = wasm_trampoline_adjust_address (XEXP (m_tramp, 0));
  m_tramp = force_reg (Pmode, m_tramp);
  emit_insn (gen_rtx_SET (gen_rtx_MEM (SImode, m_tramp), gen_rtx_CONST_INT(SImode, 0x4d4a5254)));
  emit_insn (gen_rtx_SET (m_tramp, plus_constant (SImode, m_tramp, 4)));
  emit_insn (gen_rtx_SET (gen_rtx_MEM (SImode, m_tramp), fnaddr));
  emit_insn (gen_rtx_SET (m_tramp, plus_constant (SImode, m_tramp, 4)));
  emit_insn (gen_rtx_SET (gen_rtx_MEM (SImode, m_tramp), static_chain));
  emit_insn (gen_rtx_SET (m_tramp, plus_constant (SImode, m_tramp, -8)));
}

static bool
wasm_asm_can_output_mi_thunk (const_tree, HOST_WIDE_INT, HOST_WIDE_INT,
			       const_tree)
{
  return true;
}

#if 0 /* unused */
/* Return RTX for the first (implicit this) parameter passed to a method. */

static rtx
wasm_this_parameter (tree function)
{
  if (wasm_is_stackcall (TREE_TYPE (function)))
    return gen_rtx_MEM (SImode, plus_constant (SImode,
					       gen_rtx_REG (SImode, SP_REG),
					       16));
  else
    return gen_rtx_REG (SImode, R0_REG);
}
#endif

/* Adjust the "this" parameter and jump to function. We can't actually force
 * a tail call to happen on the JS stack, but we can force one on the VM
 * stack. Do so.
 *
 * A slight complication here: FUNCTION might be a stack-call function
 * with an aggregate return type, so we need to check for that before
 * deciding where on the stack the this argument lives.
 */
static void
wasm_asm_output_mi_thunk (FILE *f, tree thunk, HOST_WIDE_INT delta,
			   HOST_WIDE_INT vcall_offset, tree function)
{
  const char *tname = XSTR (XEXP (DECL_RTL (function), 0), 0);
  const char *name = XSTR (XEXP (DECL_RTL (thunk), 0), 0);
  bool stackcall = wasm_is_stackcall (TREE_TYPE (function));
  bool structret = aggregate_value_p (TREE_TYPE (TREE_TYPE (function)), TREE_TYPE (function));
  const char *r = (structret && !stackcall) ? "$r1" : "$r0";

  tname += tname[0] == '*';
  name += name[0] == '*';

  if (stackcall)
    {
      asm_fprintf (f, "\t(set_local %s (i32.add (get_local $sp) (i32.const %d)))\n", r, structret ? 20 : 16);
      asm_fprintf (f, "\t(set_local %s (i32.load (get_local %s)))\n", r, r);
    }

  asm_fprintf (f, "\t(set_local %s (i32.add (get_local %s) (i32.const %d)))\n",
	       r, r, (int) delta);
  if (vcall_offset)
    {
      asm_fprintf (f, "\t(i32.store (i32.const 4096) (get_local %s))\n", r);
      asm_fprintf (f, "\t(i32.store (i32.const 4096) (i32.add (i32.load (i32.const 4096)) (i32.const %d)))\n", (int)vcall_offset);
      asm_fprintf (f, "\t(i32.store (i32.const 4096) (i32.load (i32.load (i32.const 4096))))\n");
      asm_fprintf (f, "\t(set_local %s (i32.add (get_local %s) (i32.load (i32.const 4096))))\n",
		   r, r);
    }

  if (stackcall)
    asm_fprintf (f, "\t(i32.store (i32.add (get_local $sp) (i32.const %d)) (get_local %s))\n", structret ? 20 : 16, r);

  asm_fprintf (f, "\t(return (call $f_$\n\t.codetextlabel %s\n\t(i32.const 0) (i32.add (get_local $sp) (i32.const 16)) (get_local $r0) (get_local $r1) (i32.add (get_local $dpc) (get_local $pc0))\n\t.ncodetextlabel %s\n\t))\n",
	       tname, tname);
}

#undef TARGET_ASM_GLOBALIZE_LABEL
#define TARGET_ASM_GLOBALIZE_LABEL wasm_globalize_label

#undef TARGET_LEGITIMATE_CONSTANT_P
#define TARGET_LEGITIMATE_CONSTANT_P wasm_legitimate_constant_p

#undef TARGET_LEGITIMATE_ADDRESS_P
#define TARGET_LEGITIMATE_ADDRESS_P wasm_legitimate_address_p

#undef TARGET_STRICT_ARGUMENT_NAMING
#define TARGET_STRICT_ARGUMENT_NAMING wasm_strict_argument_naming

#undef TARGET_FUNCTION_ARG_BOUNDARY
#define TARGET_FUNCTION_ARG_BOUNDARY wasm_function_arg_boundary

#undef TARGET_FUNCTION_ARG_ROUND_BOUNDARY
#define TARGET_FUNCTION_ARG_ROUND_BOUNDARY wasm_function_arg_round_boundary

#undef TARGET_FUNCTION_ARG_ADVANCE
#define TARGET_FUNCTION_ARG_ADVANCE wasm_function_arg_advance

#undef TARGET_FUNCTION_INCOMING_ARG
#define TARGET_FUNCTION_INCOMING_ARG wasm_function_incoming_arg

#undef TARGET_FUNCTION_ARG
#define TARGET_FUNCTION_ARG wasm_function_arg

#undef TARGET_FRAME_POINTER_REQUIRED
#define TARGET_FRAME_POINTER_REQUIRED wasm_frame_pointer_required

#undef TARGET_DEBUG_UNWIND_INFO
#define TARGET_DEBUG_UNWIND_INFO  wasm_debug_unwind_info

#undef TARGET_CALL_ARGS
#define TARGET_CALL_ARGS wasm_call_args

#undef TARGET_END_CALL_ARGS
#define TARGET_END_CALL_ARGS wasm_end_call_args

#undef TARGET_OPTION_OVERRIDE
#define TARGET_OPTION_OVERRIDE wasm_option_override

#undef TARGET_ATTRIBUTE_TABLE
#define TARGET_ATTRIBUTE_TABLE wasm_attribute_table

#undef TARGET_REGISTER_PRIORITY
#define TARGET_REGISTER_PRIORITY wasm_register_priority

#undef TARGET_RTX_COSTS
#define TARGET_RTX_COSTS wasm_rtx_costs

#undef TARGET_CXX_LIBRARY_RTTI_COMDAT
#define TARGET_CXX_LIBRARY_RTTI_COMDAT wasm_cxx_library_rtti_comdat

#undef TARGET_CXX_CLASS_DATA_ALWAYS_COMDAT
#define TARGET_CXX_CLASS_DATA_ALWAYS_COMDAT wasm_cxx_class_data_always_comdat

#undef TARGET_TRAMPOLINE_ADJUST_ADDRESS
#define TARGET_TRAMPOLINE_ADJUST_ADDRESS wasm_trampoline_adjust_address

#undef TARGET_TRAMPOLINE_INIT
#define TARGET_TRAMPOLINE_INIT wasm_trampoline_init

#undef TARGET_ABSOLUTE_BIGGEST_ALIGNMENT
#define TARGET_ABSOLUTE_BIGGEST_ALIGNMENT 1024

#undef TARGET_ASM_OUTPUT_MI_THUNK
#define TARGET_ASM_OUTPUT_MI_THUNK wasm_asm_output_mi_thunk

#undef TARGET_ASM_CAN_OUTPUT_MI_THUNK
#define TARGET_ASM_CAN_OUTPUT_MI_THUNK wasm_asm_can_output_mi_thunk

#define TARGET_PROMOTE_FUNCTION_MODE wasm_promote_function_mode
#define TARGET_ABSOLUTE_BIGGEST_ALIGNMENT 1024
#define TARGET_CAN_ELIMINATE wasm_can_eliminate
#define TARGET_PROMOTE_PROTOTYPES wasm_promote_prototypes
#define TARGET_RETURN_POPS_ARGS wasm_return_pops_args
#define TARGET_FUNCTION_VALUE wasm_function_value
#define TARGET_LIBCALL_VALUE wasm_libcall_value
#define TARGET_FUNCTION_VALUE_REGNO_P wasm_function_value_regno_p
#define TARGET_RETURN_IN_MEMORY wasm_return_in_memory
#define TARGET_STRUCT_VALUE_RTX wasm_struct_value_rtx
#define TARGET_REGISTER_MOVE_COST wasm_register_move_cost
#define TARGET_MEMORY_MOVE_COST wasm_memory_move_cost
#define TARGET_LRA_P wasm_lra_p

#include "target-def.h"

struct gcc_target targetm = TARGET_INITIALIZER;
