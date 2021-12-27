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
#include "memmodel.h"
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
asmjs_cpu_cpp_builtins (struct cpp_reader * pfile)
{
  cpp_assert(pfile, "cpu=asmjs");
  cpp_assert(pfile, "machine=asmjs");

  cpp_define(pfile, "__ASMJS__");
}

bool linux_libc_has_function(enum function_class cl ATTRIBUTE_UNUSED)
{
  return true;
}

void
asmjs_globalize_label (FILE * stream, const char *name)
{
  asm_fprintf (stream, "\t.global %s\n", name + (name[0] == '*'));
}

void
asmjs_weaken_label (FILE *stream, const char *name)
{
  asm_fprintf (stream, "\t.weak %s\n", name + (name[0] == '*'));
}

void
asmjs_output_def (FILE *stream, const char *alias, const char *name)
{
  asm_fprintf (stream, "\t.set %s, %s\n", alias + (alias[0] == '*'), name + (name[0] == '*'));
}

enum reg_class asmjs_regno_reg_class(int regno)
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
asmjs_legitimate_constant_p (machine_mode mode ATTRIBUTE_UNUSED, rtx x)
{
  return (CONST_INT_P (x)
	  || CONST_DOUBLE_P (x)
	  || CONSTANT_ADDRESS_P (x));
}

typedef struct {
  int count;
  int const_count;
  int indir;
} asmjs_legitimate_address;

static bool
asmjs_legitimate_address_p_rec (asmjs_legitimate_address *r,
				machine_mode mode, rtx x, bool strict_p);

static bool
asmjs_legitimate_address_p_rec (asmjs_legitimate_address *r,
				machine_mode mode, rtx x, bool strict_p)
{
  enum rtx_code code = GET_CODE (x);

  if (MEM_P(x))
    {
      r->indir++;
      return asmjs_legitimate_address_p_rec (r, mode, XEXP (x, 0), strict_p);
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
      if (!asmjs_legitimate_address_p_rec (r, mode, XEXP (x, 0), strict_p) ||
	  !asmjs_legitimate_address_p_rec (r, mode, XEXP (x, 1), strict_p))
	return false;

      return true;
    }

  return false;
}

static bool
asmjs_legitimate_address_p (machine_mode mode, rtx x, bool strict_p)
{
  asmjs_legitimate_address r = { 0, 0, 0 };
  bool s;

  s = asmjs_legitimate_address_p_rec (&r, mode, x, strict_p);

#if 0
  debug_rtx(x);
  fprintf(stderr, "%d %d %d %d\n",
	  s, r.count, r.const_count, r.indir);
#endif

  if (!s)
    return false;

  return (r.count <= MAX_REGS_PER_ADDRESS) && (r.count <= max_rpi) && (r.indir <= 0) && (r.const_count <= 1);
}

static int asmjs_argument_count (const_tree fntype)
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
asmjs_is_stackcall (const_tree type)
{
  if (!type)
    return false;

  tree attrs = TYPE_ATTRIBUTES (type);
  if (attrs != NULL_TREE)
    {
      if (lookup_attribute ("stackcall", attrs))
	return true;
    }

  if (asmjs_argument_count (type) < 0)
    return true;

  return false;
}

static bool
asmjs_is_real_stackcall (const_tree type)
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

void asmjs_init_cumulative_args(CUMULATIVE_ARGS *cum_p,
				const_tree fntype ATTRIBUTE_UNUSED,
				rtx libname ATTRIBUTE_UNUSED,
				const_tree fndecl,
				unsigned int n_named_args ATTRIBUTE_UNUSED)
{
  cum_p->n_areg = 0;
  cum_p->n_vreg = 0;
  cum_p->is_stackcall = asmjs_is_stackcall (fndecl ? TREE_TYPE (fndecl) : NULL_TREE) || asmjs_is_stackcall (fntype);
}

bool asmjs_strict_argument_naming(cumulative_args_t cum ATTRIBUTE_UNUSED)
{
  return true;
}
bool asmjs_function_arg_regno_p(int regno)
{
  /* XXX is this incoming or outgoing? */
  return ((regno >= R0_REG && regno < R0_REG + N_ARGREG_PASSED) ||
	  (regno >= A0_REG && regno < A0_REG + N_ARGREG_GLOBAL));
}

unsigned int asmjs_function_arg_boundary(machine_mode mode, const_tree type)
{
  if (GET_MODE_ALIGNMENT (mode) > PARM_BOUNDARY ||
      (type && TYPE_ALIGN (type) > PARM_BOUNDARY)) {
    return PARM_BOUNDARY * 2;
  } else
    return PARM_BOUNDARY;
}
unsigned int asmjs_function_arg_round_boundary(machine_mode mode, const_tree type)
{
  if (GET_MODE_ALIGNMENT (mode) > PARM_BOUNDARY ||
      (type && TYPE_ALIGN (type) > PARM_BOUNDARY)) {
    return PARM_BOUNDARY * 2;
  } else
    return PARM_BOUNDARY;
}

unsigned int asmjs_function_arg_offset(machine_mode mode ATTRIBUTE_UNUSED,
				       const_tree type ATTRIBUTE_UNUSED)
{
  return 0;
}

static void
asmjs_function_arg_advance(cumulative_args_t cum_v, machine_mode mode,
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
asmjs_function_incoming_arg (cumulative_args_t pcum_v, machine_mode mode,
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
asmjs_function_arg (cumulative_args_t pcum_v, machine_mode mode,
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

bool asmjs_frame_pointer_required (void)
{
  return true;
}

enum unwind_info_type asmjs_debug_unwind_info (void)
{
  return UI_DWARF2;
}

static void
asmjs_call_args (rtx arg ATTRIBUTE_UNUSED, tree funtype)
{
  if (cfun->machine->funtype != funtype) {
    cfun->machine->funtype = funtype;
  }
}

static void
asmjs_end_call_args (void)
{
  cfun->machine->funtype = NULL;
}

static struct machine_function *
asmjs_init_machine_status (void)
{
  struct machine_function *p = ggc_cleared_alloc<machine_function> ();
  return p;
}

static void
asmjs_option_override (void)
{
  init_machine_status = asmjs_init_machine_status;
}

static tree
asmjs_handle_cconv_attribute (tree *node ATTRIBUTE_UNUSED,
			      tree name ATTRIBUTE_UNUSED,
			      tree args ATTRIBUTE_UNUSED, int,
			      bool *no_add_attrs ATTRIBUTE_UNUSED)
{
  return NULL;
}

static tree
asmjs_handle_bogotics_attribute (tree *node ATTRIBUTE_UNUSED,
				 tree name ATTRIBUTE_UNUSED,
				 tree args ATTRIBUTE_UNUSED, int,
				 bool *no_add_attrs ATTRIBUTE_UNUSED)
{
  return NULL;
}

#if 0
extern void debug_tree (tree t);
extern void debug_c_tree (tree t);

#include "gimple.h"
#include "lto-streamer.h"
#include "streamer-hooks.h"
#endif

static vec<struct asmjs_jsexport_decl> asmjs_jsexport_decls;

__attribute__((weak)) void
asmjs_jsexport (tree node ATTRIBUTE_UNUSED,
		struct asmjs_jsexport_opts *opts,
		vec<struct asmjs_jsexport_decl> *decls ATTRIBUTE_UNUSED)
{
  error("jsexport not defined for this frontend");
}

#include <print-tree.h>
#include <plugin.h>

static void asmjs_jsexport_unit_callback (void *, void *)
{
  unsigned i,j;
  struct asmjs_jsexport_decl *p;
  if (!asmjs_jsexport_decls.is_empty())
    {
      switch_to_section (get_section (".jsexport", SECTION_MERGE|SECTION_STRINGS|1, NULL_TREE));

      FOR_EACH_VEC_ELT (asmjs_jsexport_decls, i, p)
	{
	  const char **pstr;
	  const char *str;
	  int c;
	  FOR_EACH_VEC_ELT (p->fragments, j, pstr)
	    {
	      if (*pstr == NULL)
		{
		  fprintf (asm_out_file, "\t.reloc .+2,R_ASMJS_HEX16,%s\n",
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

  asmjs_jsexport_decls = vec<struct asmjs_jsexport_decl>();
}

static void asmjs_jsexport_parse_args (tree args, struct asmjs_jsexport_opts *opts)
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

static void asmjs_jsexport_decl_callback (void *gcc_data, void *)
{
  tree decl = (tree)gcc_data;
  bool found = false;
  const char *jsname = NULL;
  int recurse = 0;
  struct asmjs_jsexport_opts opts;

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
	      asmjs_jsexport_parse_args (TREE_VALUE (attr), &opts);
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
	      asmjs_jsexport_parse_args (TREE_VALUE (attr), &opts);
	    }
	}
    }

  if (!found)
    {
      return;
    }

  if (found)
    asmjs_jsexport(decl, &opts, &asmjs_jsexport_decls);
}

static bool asmjs_jsexport_plugin_inited = false;

static void asmjs_jsexport_plugin_init (void)
{
  register_callback("jsexport", PLUGIN_FINISH_DECL,
		    asmjs_jsexport_decl_callback, NULL);
  register_callback("jsexport", PLUGIN_FINISH_TYPE,
		    asmjs_jsexport_decl_callback, NULL);
  register_callback("jsexport", PLUGIN_FINISH_UNIT,
		    asmjs_jsexport_unit_callback, NULL);
  flag_plugin_added = true;

  asmjs_jsexport_plugin_inited = true;
}

static tree
asmjs_handle_jsexport_attribute (tree * node, tree attr_name ATTRIBUTE_UNUSED,
				 tree args, int,
				 bool *no_add_attrs ATTRIBUTE_UNUSED)
{
  struct asmjs_jsexport_opts opts;

  opts.jsname = NULL;
  opts.recurse = 0;

  if (!asmjs_jsexport_plugin_inited)
    asmjs_jsexport_plugin_init ();

  if (TREE_CODE (*node) == TYPE_DECL)
    {
      asmjs_jsexport_parse_args (args, &opts);
      asmjs_jsexport (*node, &opts, &asmjs_jsexport_decls);
    }

  return NULL_TREE;
}

static const struct attribute_spec asmjs_attribute_table[] =
{
  /* { name, min_len, max_len, decl_req, type_req, fn_type_req, handler,
       affects_type_identity } */
  /* stackcall, no regparms but argument count on the stack */
  { "stackcall", 0, 0, false, true, true, asmjs_handle_cconv_attribute, true },
  { "regparm", 1, 1, false, true, true, asmjs_handle_cconv_attribute, true },

  { "nobogotics", 0, 0, false, true, true, asmjs_handle_bogotics_attribute, false },
  { "bogotics_backwards", 0, 0, false, true, true, asmjs_handle_bogotics_attribute, false },
  { "bogotics", 0, 0, false, true, true, asmjs_handle_bogotics_attribute, false },

  { "nobreak", 0, 0, false, true, true, asmjs_handle_bogotics_attribute, false },
  { "breakonenter", 0, 0, false, true, true, asmjs_handle_bogotics_attribute, false },
  { "fullbreak", 0, 0, false, true, true, asmjs_handle_bogotics_attribute, false },
  { "jsexport", 0, 2, false, false, false, asmjs_handle_jsexport_attribute, false },
  { NULL, 0, 0, false, false, false, NULL, false }
};

const char *asmjs_heap(rtx x, int *shift, bool wantsign)
{
  switch (GET_MODE (x)) {
  case QImode:
    *shift = 0;
    return wantsign ? "HEAP8" : "HEAPU8";
  case HImode:
    *shift = 1;
    return wantsign ? "HEAP16" : "HEAPU16";
  case SImode:
    *shift = 2;
    return wantsign ? "HEAP32" : "HEAPU32";
  case SFmode:
    *shift = 2;
    return "HEAPF32";
  case DFmode:
    *shift = 3;
    return "HEAPF64";
  default:
    *shift = 0;
    return "HEAPU8";
  }
}

const char *asmjs_heap2pre(rtx x, int nosign)
{
  switch (GET_MODE (x)) {
  case QImode:
    return nosign ? "HEAPU8[" : "HEAP8[";
  case HImode:
    return nosign ? "HEAPU16[" : "HEAP16[";
  case SImode:
    return nosign ? "HEAPU32[" : "HEAP32[";
  case SFmode:
    return "HEAPF32[";
  case DFmode:
    return "HEAPF64[";
  default:
    return "HEAPU8[";
  }
}

const char *asmjs_heap2post(rtx x)
{
  switch (GET_MODE (x)) {
  case QImode:
    return ">>0]";
  case HImode:
    return ">>1]";
  case SImode:
    return ">>2]";
  case SFmode:
    return ">>2]";
  case DFmode:
    return ">>3]";
  default:
    return ">>0]";
  }
}

void asmjs_print_operand_address(FILE *stream, rtx x)
{
  switch (GET_CODE (x)) {
  case MEM: {
    rtx addr = XEXP (x, 0);

    asmjs_print_operand (stream, addr, 0);

    return;
  }
  default: {
    print_rtl(stream, x);
  }
  }
}

enum asmjs_kind {
  kind_lval, kind_signed, kind_unsigned, kind_float
};

/* according to https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Operator_Precedence */
enum asmjs_prec {
  prec_comma,
  prec_assign,
  prec_bitor,
  prec_bitxor,
  prec_bitand,
  prec_equals,
  prec_comp,
  prec_shift,
  prec_add,
  prec_mult,
  prec_unary,
  prec_function,
  prec_brack,
  prec_paren
};

const char *asmjs_prec_names[] = {
  /* [prec_comma]    = */ "comma",
  /* [prec_assign]   = */ "assign",
  /* [prec_bitor]    = */ "bitor",
  /* [prec_bitxor]   = */ "bitxor",
  /* [prec_bitand]   = */ "bitand",
  /* [prec_equals]   = */ "equals",
  /* [prec_comp]     = */ "comp",
  /* [prec_shift]    = */ "shift",
  /* [prec_add]      = */ "add",
  /* [prec_mult]     = */ "mult",
  /* [prec_unary]    = */ "unary",
  /* [prec_function] = */ "function",
  /* [prec_brack]    = */ "brack",
  /* [prec_paren]    = */ "paren"
};

void asmjs_print_op(FILE *stream, rtx x, asmjs_prec want_prec, asmjs_kind want_kind);
bool asmjs_needs_paren(asmjs_prec have_prec, asmjs_prec want_prec)
{
  if (have_prec >= want_prec)
    return true;

  /* special case: a++b and a--b cause parse errors, so add extra
   * parentheses. This also catches a+~b and a-!b and such. */
  if (have_prec == prec_add && want_prec == prec_unary)
    return true;

  return false;
}

void asmjs_print_lparen(FILE *stream, asmjs_prec have_prec, asmjs_prec want_prec)
{
  if (asmjs_needs_paren (have_prec, want_prec))
    asm_fprintf (stream, "(");
}

void asmjs_print_rparen(FILE *stream, asmjs_prec have_prec, asmjs_prec want_prec)
{
  if (asmjs_needs_paren (have_prec, want_prec))
    asm_fprintf (stream, ")");
}

asmjs_kind asmjs_op_kind(rtx x);
asmjs_kind asmjs_rtx_kind(rtx x)
{
  switch (GET_CODE (x)) {
  case REG: {
    return kind_lval;
  }
  case CONST_INT: {
    if (XWINT (x, 0) < (1<<20) &&
	XWINT (x, 0) > -(1<<20))
      return kind_signed;
    return kind_lval;
  }
  case CONST_DOUBLE: {
    return kind_float;
  }
  case AND:
  case XOR:
  case IOR:
  case ASHIFT:
  case ASHIFTRT:
    return kind_signed;
  case LSHIFTRT:
    return kind_unsigned;
  case PLUS:
  case MINUS:
  case DIV:
  case MOD:
  case UDIV:
  case UMOD:
    return asmjs_op_kind(x);
  default:
    return kind_lval;
  }
}

void asmjs_print_assignment(FILE *stream, rtx x, asmjs_prec want_prec, asmjs_kind want_kind);
void asmjs_print_operation(FILE *stream, rtx x, asmjs_prec want_prec, asmjs_kind want_kind)
{
  asmjs_kind have_kind = asmjs_rtx_kind (x);

  if (have_kind == want_kind)
    want_kind = kind_lval;

  if (want_kind == kind_signed) {
    asmjs_print_lparen (stream, want_prec, prec_bitor);
    asmjs_print_operation (stream, x, prec_bitor, kind_lval);
    asm_fprintf(stream, "|0");
    asmjs_print_rparen (stream, want_prec, prec_bitor);
  } else if (want_kind == kind_unsigned) {
    asmjs_print_lparen (stream, want_prec, prec_shift);
    asmjs_print_operation (stream, x, prec_shift, kind_lval);
    asm_fprintf(stream, ">>>0");
    asmjs_print_rparen (stream, want_prec, prec_shift);
  } else if (want_kind == kind_float) {
    asmjs_print_lparen (stream, want_prec, prec_unary);
    asm_fprintf(stream, "+");
    asmjs_print_operation (stream, x, prec_unary, kind_lval);
    asmjs_print_rparen (stream, want_prec, prec_unary);
  } else if (want_kind == kind_lval) {
    switch (GET_CODE (x)) {
    case PC: {
      asm_fprintf (stream, "pc");
      break;
    }
    case REG: {
      asm_fprintf (stream, "%s", reg_names[REGNO (x)]);
      break;
    }
    case ZERO_EXTEND: {
      rtx mem = XEXP (x, 0);
      if (GET_CODE (mem) == MEM)
	{
	  rtx addr = XEXP (mem, 0);
	  const char *heappre = asmjs_heap2pre (mem, 1);
	  const char *heappost = asmjs_heap2post (mem);

	  asm_fprintf (stream, "%s", heappre);
	  asmjs_print_operation (stream, addr, prec_shift, kind_lval);
	  asm_fprintf (stream, "%s", heappost);
	}
      else if (GET_CODE (mem) == SUBREG)
	{
	  rtx reg = XEXP (mem, 0);

	  asmjs_print_operation (stream, reg, prec_assign, kind_lval);
	  asm_fprintf (stream, "&%d", GET_MODE (mem) == HImode ? 65535 : 255);
	}
      else if (GET_CODE (mem) == REG)
	{
	  asmjs_print_operation (stream, mem, prec_assign, kind_lval);
	  asm_fprintf (stream, "&%d", GET_MODE (mem) == HImode ? 65535 : 255);
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
      const char *heappre = asmjs_heap2pre (x, 0);
      const char *heappost = asmjs_heap2post (x);

      asm_fprintf (stream, "%s", heappre);
      asmjs_print_operation (stream, addr, prec_shift, kind_lval);
      asm_fprintf (stream, "%s", heappost);
      break;
    }
    case CONST_INT: {
      asm_fprintf (stream, "%d", (int) XWINT (x, 0));
      break;
    }
    case CONST_DOUBLE: {
      REAL_VALUE_TYPE r;
      char buf[512];
      long l[2];

      r = *CONST_DOUBLE_REAL_VALUE (x);
      REAL_VALUE_TO_TARGET_DOUBLE (r, l);

      real_to_decimal_for_mode (buf, &r, 510, 0, 1, DFmode);

      if (strcmp(buf, "+Inf") == 0) {
	asm_fprintf (stream, "+(1.0/0.0)");
      } else if (strcmp (buf, "-Inf") == 0) {
	asm_fprintf (stream, "+(-1.0/0.0)");
      } else if (buf[0] == '+')
	asm_fprintf (stream, "%s", buf+1);
      else
	asm_fprintf (stream, "%s", buf);
      break;
    }
    case SYMBOL_REF: {
      const char *name = XSTR (x, 0);
      if (!SYMBOL_REF_FUNCTION_P (x)) {
	asm_fprintf (stream, "$\n\t.datatextlabel ");
	asm_fprintf (stream, "%s", name + (name[0] == '*'));
	asm_fprintf (stream, "\n\t");
	asm_fprintf (stream, "/*%s*/", name + (name[0] == '*'));
      } else if (in_section->common.flags & SECTION_CODE) {
	asm_fprintf (stream, "$\n\t.codetextlabel ");
	asm_fprintf (stream, "%s", name + (name[0] == '*'));
	asm_fprintf (stream, "\n\t");
	asm_fprintf (stream, "/*%s*/", name + (name[0] == '*'));
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
      x = label_ref_label (x);
      asm_fprintf (stream, "$\n\t.codetextlabel ");
      ASM_GENERATE_INTERNAL_LABEL (buf, "L", CODE_LABEL_NUMBER (x));
      ASM_OUTPUT_LABEL_REF (stream, buf);
      asm_fprintf (stream, "\n\t");
      asm_fprintf (stream, "/*%s*/", buf + (buf[0] == '*'));
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
      asmjs_print_op (stream, x, want_prec, want_kind);
      break;
    }
    case SET: {
      asmjs_print_assignment (stream, x, want_prec, want_kind);
      break;
    }
    case EQ:
    case NE:
    case GT:
    case LT:
    case GE:
    case LE: {
      asmjs_print_op (stream, x, want_prec, want_kind);
      break;
    }
    case GTU:
    case LTU:
    case GEU:
    case LEU: {
      asmjs_print_op (stream, x, want_prec, want_kind);
      break;
    }
    case FLOAT:
    case FLOAT_TRUNCATE:
    case FLOAT_EXTEND:
    case FIX:
    case NEG:
    case NOT: {
      asmjs_print_op (stream, x, want_prec, want_kind);
      break;
    }
    case CONST: {
      asmjs_print_operation (stream, XEXP (x, 0), want_prec, want_kind);
      break;
    }
    default:
      asm_fprintf (stream, "BROKEN (unknown code:746): ");
      print_rtl (stream, x);
      return;
    }
  }
}

void asmjs_print_predicate(FILE *stream, rtx x)
{
  asm_fprintf(stream, "if(");
  asmjs_print_operation (stream, x, prec_comma, kind_signed);
  asm_fprintf(stream, ")");
}

static unsigned asmjs_function_regsize(tree decl ATTRIBUTE_UNUSED);

void asmjs_print_operand(FILE *stream, rtx x, int code)
{
  //print_rtl (stderr, x);
  if (code == 'O') {
    asmjs_print_operation (stream, x, prec_comma, kind_lval);
    //asm_fprintf (stream, "\n/* RTL: ");
    //print_rtl(stream, x);
    //asm_fprintf (stream, "*/");
    return;
  } else if (code == 'C') {
    asmjs_print_predicate (stream, x);
    return;
  } else if (code == '/') {
    asm_fprintf (stream, "%d", asmjs_function_regsize (NULL_TREE));
    return;
  }

  asm_fprintf (stream, "BROKEN: should be using asmjs_print_operation");
  print_rtl (stream, x);

  return;
}

bool asmjs_print_operand_punct_valid_p(int code)
{
  switch (code) {
  case 'F':
  case 'U':
  case 'L':
  case 'O':
  case 'C':
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

struct asmjs_operator {
  int code;
  machine_mode inmode;
  machine_mode outmode;

  const char *prefix;
  const char *infix;
  const char *suffix;

  asmjs_prec prec;
  asmjs_kind inkind;
  asmjs_kind outkind;
};

struct asmjs_operator asmjs_operators[] = {
  {
    PLUS, SImode, SImode,
    "", "+", "",
    prec_add, kind_signed, kind_lval
  },
  {
    MINUS, SImode, SImode,
    "", "-", "",
    prec_add, kind_signed, kind_lval
  },
  {
    MULT, SImode, SImode,
    "imul(", ",", ")",
    prec_comma, kind_signed, kind_signed
  },
  {
    DIV, SImode, SImode,
    "", "/", "",
    prec_mult, kind_signed, kind_lval
  },
  {
    MOD, SImode, SImode,
    "", "%", "",
    prec_mult, kind_signed, kind_lval
  },
  {
    UDIV, SImode, SImode,
    "", "/", "",
    prec_mult, kind_unsigned, kind_lval
  },
  {
    UMOD, SImode, SImode,
    "", "%", "",
    prec_mult, kind_unsigned, kind_lval
  },
  {
    XOR, SImode, SImode,
    "", "^", "",
    prec_bitxor, kind_lval, kind_signed
  },
  {
    IOR, SImode, SImode,
    "", "|", "",
    prec_bitor, kind_lval, kind_signed
  },
  {
    AND, SImode, SImode,
    "", "&", "",
    prec_bitand, kind_lval, kind_signed
  },
  {
    ASHIFT, SImode, SImode,
    "", "<<", "",
    prec_shift, kind_lval, kind_signed
  },
  {
    ASHIFTRT, SImode, SImode,
    "", ">>", "",
    prec_shift, kind_lval, kind_signed
  },
  {
    LSHIFTRT, SImode, SImode,
    "", ">>>", "",
    prec_shift, kind_lval, kind_unsigned
  },
  {
    EQ, SImode, VOIDmode,
    "", "==", "",
    prec_comp, kind_signed, kind_signed
  },
  {
    NE, SImode, VOIDmode,
    "", "!=", "",
    prec_comp, kind_signed, kind_signed
  },
  {
    LT, SImode, VOIDmode,
    "", "<", "",
    prec_comp, kind_signed, kind_signed
  },
  {
    LE, SImode, VOIDmode,
    "", "<=", "",
    prec_comp, kind_signed, kind_signed
  },
  {
    GT, SImode, VOIDmode,
    "", ">", "",
    prec_comp, kind_signed, kind_signed
  },
  {
    GE, SImode, VOIDmode,
    "", ">=", "",
    prec_comp, kind_signed, kind_signed
  },
  {
    LTU, SImode, VOIDmode,
    "", "<", "",
    prec_comp, kind_unsigned, kind_signed
  },
  {
    LEU, SImode, VOIDmode,
    "", "<=", "",
    prec_comp, kind_unsigned, kind_signed
  },
  {
    GTU, SImode, VOIDmode,
    "", ">", "",
    prec_comp, kind_unsigned, kind_signed
  },
  {
    GEU, SImode, VOIDmode,
    "", ">=", "",
    prec_comp, kind_unsigned, kind_signed
  },
  {
    EQ, SImode, SImode,
    "", "==", "",
    prec_comp, kind_signed, kind_lval
  },
  {
    NE, SImode, SImode,
    "", "!=", "",
    prec_comp, kind_signed, kind_lval
  },
  {
    LT, SImode, SImode,
    "", "<", "",
    prec_comp, kind_signed, kind_lval
  },
  {
    LE, SImode, SImode,
    "", "<=", "",
    prec_comp, kind_signed, kind_lval
  },
  {
    GT, SImode, SImode,
    "", ">", "",
    prec_comp, kind_signed, kind_lval
  },
  {
    GE, SImode, SImode,
    "", ">=", "",
    prec_comp, kind_signed, kind_lval
  },
  {
    LTU, SImode, SImode,
    "", "<", "",
    prec_comp, kind_unsigned, kind_lval
  },
  {
    LEU, SImode, SImode,
    "", "<=", "",
    prec_comp, kind_unsigned, kind_lval
  },
  {
    GTU, SImode, SImode,
    "", ">", "",
    prec_comp, kind_unsigned, kind_lval
  },
  {
    GEU, SImode, SImode,
    "", ">=", "",
    prec_comp, kind_unsigned, kind_lval
  },
  {
    PLUS, DFmode, DFmode,
    "", "+", "",
    prec_add, kind_float, kind_float
  },
  {
    MINUS, DFmode, DFmode,
    "", "-", "",
    prec_add, kind_float, kind_float
  },
  {
    MULT, DFmode, DFmode,
    "", "*", "",
    prec_mult, kind_float, kind_float
  },
  {
    DIV, DFmode, DFmode,
    "", "/", "",
    prec_mult, kind_float, kind_float
  },
  {
    MOD, DFmode, DFmode,
    "", "%", "",
    prec_mult, kind_float, kind_float
  },
  {
    NEG, DFmode, DFmode,
    "-", NULL, "",
    prec_unary, kind_float, kind_float
  },
  {
    EQ, DFmode, VOIDmode,
    "", "==", "",
    prec_comp, kind_float, kind_signed
  },
  {
    NE, DFmode, VOIDmode,
    "", "!=", "",
    prec_comp, kind_float, kind_signed
  },
  {
    LT, DFmode, VOIDmode,
    "", "<", "",
    prec_comp, kind_float, kind_signed
  },
  {
    LE, DFmode, VOIDmode,
    "", "<=", "",
    prec_comp, kind_float, kind_signed
  },
  {
    GT, DFmode, VOIDmode,
    "", ">", "",
    prec_comp, kind_float, kind_signed
  },
  {
    GE, DFmode, VOIDmode,
    "", ">=", "",
    prec_comp, kind_float, kind_signed
  },
  {
    NEG, SImode, SImode,
    "-", NULL, "",
    prec_unary, kind_signed, kind_lval
  },
  {
    NOT, SImode, SImode,
    "~", NULL, "",
    prec_unary, kind_signed, kind_signed
  },
  {
    FIX, DFmode, SImode,
    "~~", NULL, "",
    prec_unary, kind_float, kind_signed
  },
  {
    FLOAT, SImode, DFmode,
    "+", NULL, "",
    prec_unary, kind_signed, kind_float
  },
  /* I think this is wrong and we should use the libgcc definition instead. XXX */
  {
    FLOAT, DImode, DFmode,
    "+", NULL, "",
    prec_unary, kind_signed, kind_float
  },
  {
    FLOAT_EXTEND, SFmode, DFmode,
    "+", NULL, "",
    prec_unary, kind_float, kind_float
  },
  {
    FLOAT_TRUNCATE, DFmode, SFmode,
    "+fround(+", NULL, ")",
    prec_unary, kind_float, kind_float
  },
  {
    0, SImode, SImode,
    NULL, NULL, NULL,
    prec_shift, kind_lval, kind_unsigned
  }
};

bool modes_compatible (machine_mode mode1, machine_mode mode2)
{
  return mode1 == mode2 || mode1 == VOIDmode || mode2 == VOIDmode;
}

asmjs_kind asmjs_op_kind(rtx x)
{
  struct asmjs_operator *oper;
  rtx a = XEXP (x, 0);

  for (oper = asmjs_operators; oper->prefix; oper++) {
    if (oper->code == GET_CODE (x) &&
	oper->outmode == GET_MODE (x) &&
	modes_compatible(GET_MODE (a), oper->inmode) &&
	(!oper->infix || modes_compatible (GET_MODE (XEXP (x,1)), oper->inmode)))
      break;
  }

  if (!oper->prefix) {
    return kind_lval;
  }

  return oper->outkind;
}

void asmjs_print_op(FILE *stream, rtx x, asmjs_prec want_prec, asmjs_kind want_kind)
{
  asmjs_prec prec2;
  struct asmjs_operator *oper;
  rtx a = XEXP (x, 0);
  rtx b = NULL;

  for (oper = asmjs_operators; oper->prefix; oper++) {
    if (oper->code == GET_CODE (x) &&
	oper->outmode == GET_MODE (x) &&
	modes_compatible(GET_MODE (a), oper->inmode) &&
	(!oper->infix || modes_compatible (GET_MODE (XEXP (x,1)), oper->inmode)))
      break;
  }

  if (!oper->prefix) {
    asm_fprintf (stream, "BROKEN (unknown operator:1041): ");
    print_rtl (stream, x);
    return;
  }

  if (oper->infix)
    b = XEXP (x, 1);

  /* cases:
   * 0  A X B
   * 1  (A X B) UNUSED
   * 2  +A X B
   * 3  +(A X B)
   * 4  A X B|0
   * 5  (A X B)|0
   * 6  A X B>>>0
   * 7  (A X B)>>>0
   */

  int code = 0;

  prec2 = oper->prec;
  if (want_kind == kind_lval || want_kind == oper->outkind) {
    prec2 = oper->prec;
  } else if (want_kind == kind_float) {
    prec2 = prec_unary;
    code = 2;
  } else if (want_kind == kind_signed) {
    prec2 = prec_bitor;
    code = 4;
  } else if (want_kind == kind_unsigned) {
    prec2 = prec_shift;
    code = 6;
  }
  if (code && prec2 >= oper->prec)
    code++;
  asmjs_print_lparen (stream, want_prec, prec2);

  if ((code & 6) == 2)
    asm_fprintf (stream, "+");
  if (code & 1)
    asm_fprintf (stream, "(");

  asm_fprintf (stream, "%s", oper->prefix);
  asmjs_print_operation (stream, a, oper->prec, oper->inkind);
  if (b) {
    if (strcmp(oper->infix, "+") == 0 &&
	GET_CODE (b) == CONST_INT &&
	asmjs_rtx_kind (b) != kind_lval &&
	XWINT (b, 0) < 0)
      ;
    else
      asm_fprintf (stream, "%s", oper->infix);
    asmjs_print_operation (stream, b, oper->prec, oper->inkind);
  }
  asm_fprintf (stream, "%s", oper->suffix);

  if (code & 1)
    asm_fprintf (stream, ")");
  if ((code & 6) == 4)
    asm_fprintf (stream, "|0");
  if ((code & 6) == 6)
    asm_fprintf (stream, ">>>0");
  asmjs_print_rparen (stream, want_prec, prec2);
}

void asmjs_print_assignment(FILE *stream, rtx x, asmjs_prec want_prec ATTRIBUTE_UNUSED, asmjs_kind want_kind ATTRIBUTE_UNUSED)
{
  asmjs_kind rkind;
  switch (GET_MODE (XEXP (x, 0))) {
  case QImode:
  case HImode:
  case SImode:
    rkind = kind_signed;
    break;
  case SFmode:
  case DFmode:
    rkind = kind_float;
    break;
  default:
    rkind = kind_lval;
    break;
  }

  if (rkind == kind_lval)
    {
      asm_fprintf (stream, "BROKEN: (unknown mode:1131)");
    }

  if (GET_CODE (XEXP (x, 1)) == IF_THEN_ELSE)
    {
      rtx ite = XEXP (x, 1);
      rtx cond = XEXP (ite, 0);
      rtx lval = XEXP (x, 0);
      rtx rval_if = XEXP (ite, 1);
      rtx rval_else = XEXP (ite, 2);

      asmjs_print_predicate (stream, cond);
      asmjs_print_operation (stream, lval, prec_assign, kind_lval);
      asm_fprintf (stream, " = ");

      asmjs_print_operation (stream, rval_if, prec_assign, rkind);

      asm_fprintf (stream, ";\n\telse ");
      asmjs_print_operation (stream, lval, prec_assign, kind_lval);
      asm_fprintf (stream, " = ");

      asmjs_print_operation (stream, rval_else, prec_assign, rkind);

      return;
    }

  asmjs_print_operation (stream, XEXP (x, 0), prec_assign, kind_lval);
  asm_fprintf (stream, " = ");

  asmjs_print_operation (stream, XEXP (x, 1), prec_assign, rkind);
  if (rkind == kind_lval) {
    asm_fprintf (stream, "BROKEN: (unknown mode:1131)");
  }
}

rtx asmjs_function_value(const_tree ret_type, const_tree fn_decl ATTRIBUTE_UNUSED,
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

rtx asmjs_struct_value_rtx(tree fndecl ATTRIBUTE_UNUSED,
			   int incoming ATTRIBUTE_UNUSED)
{
  return NULL_RTX;
}

bool asmjs_return_in_memory(const_tree type,
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

bool asmjs_omit_struct_return_reg(void)
{
  if (use_rv_register)
    return false;
  else
    return true;
}

rtx asmjs_get_drap_rtx(void)
{
  return NULL_RTX;
}

rtx asmjs_libcall_value(machine_mode mode, const_rtx fun ATTRIBUTE_UNUSED)
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

bool asmjs_function_value_regno_p(unsigned int regno)
{
  if (use_rv_register)
    return regno == RV_REG;
  else
    return false;
}

bool asmjs_promote_prototypes(const_tree fntype ATTRIBUTE_UNUSED)
{
  return true;
}

void asmjs_promote_mode(machine_mode *modep,
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

machine_mode asmjs_promote_function_mode(const_tree type ATTRIBUTE_UNUSED,
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

unsigned asmjs_reg_parm_stack_space(const_tree fndecl ATTRIBUTE_UNUSED)
{
  return 0;
}

unsigned asmjs_incoming_reg_parm_stack_space(const_tree fndecl ATTRIBUTE_UNUSED)
{
  return 0;
}

unsigned asmjs_outgoing_reg_parm_stack_space(const_tree fndecl ATTRIBUTE_UNUSED)
{
  return 0;
}

void asmjs_file_start(void)
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
  fputs ("\t.include \"asmjs-macros.s\"\n", asm_out_file);
  default_file_start ();
}

static unsigned asmjs_function_regmask(tree decl ATTRIBUTE_UNUSED)
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

static unsigned asmjs_function_regsize(tree decl ATTRIBUTE_UNUSED)
{
  unsigned mask = asmjs_function_regmask (decl);
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
	  if (asmjs_regno_reg_class (i) == FLOAT_REGS)
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

static unsigned asmjs_function_regstore(FILE *stream,
					tree decl ATTRIBUTE_UNUSED)
{
  unsigned mask = asmjs_function_regmask (decl);
  unsigned size = 0;
  unsigned total_size = asmjs_function_regsize (decl);

  asm_fprintf (stream, "\t\tHEAP32[fp+%d>>2] = (fp + 0x%08x)|0;\n", size, total_size);
  size += 4;

  asm_fprintf (stream, "\t\tHEAP32[fp+%d>>2] = pc0<<4;\n", size);
  size += 4;

  asm_fprintf (stream, "\t\tHEAP32[fp+%d>>2] = (pc0 + dpc)<<4;\n", size);
  size += 4;

  asm_fprintf (stream, "\t\tHEAP32[fp+%d>>2] = rpc|0;\n", size);
  size += 4;

  asm_fprintf (stream, "\t\tHEAP32[fp+%d>>2] = sp|0;\n", size);
  size += 4;

  asm_fprintf (stream, "\t\tHEAP32[fp+%d>>2] = 0x%08x;\n", size, mask);
  size += 4;

  int i;
  for(i = 4 /* R0_REG */; i <= 31 /* F7_REG */; i++)
    {
      if (mask & (1<<(i-4)))
	{
	  if (asmjs_regno_reg_class (i) == FLOAT_REGS)
	    {
	      size += size&4;
	      asm_fprintf(stream, "\t\tHEAPF64[fp+%d>>3] = +%s;\n",
			  size, reg_names[i]);
	      size += 8;
	    }
	  else
	    {
	      if (i >= 8)
		asm_fprintf(stream, "\t\tHEAP32[fp+%d>>2] = %s|0;\n",
			    size, reg_names[i]);
	      size += 4;
	    }
	}
    }

  size += size&4;
  return size;
}

static unsigned asmjs_function_regload(FILE *stream,
					tree decl ATTRIBUTE_UNUSED)
{
  unsigned mask = asmjs_function_regmask (decl);
  unsigned size = 0;
  unsigned total_size = asmjs_function_regsize (decl);

  size += 4;

  asm_fprintf (stream, "\t\tpc0 = HEAP32[rp+%d>>2]>>4;\n", size);
  size += 4;

  asm_fprintf (stream, "\t\tdpc = ((HEAP32[rp+%d>>2]>>4) - (pc0|0))|0;\n", size);
  size += 4;

  asm_fprintf (stream, "\t\trpc = HEAP32[rp+%d>>2]|0;\n", size);
  size += 4;

  asm_fprintf (stream, "\t\tsp = HEAP32[rp+%d>>2]|0;\n", size);
  size += 4;

  size += 4;

  int i;
  for(i = 4 /* R0_REG */; i <= 31 /* F7_REG */; i++)
    {
      if (mask & (1<<(i-4)))
	{
	  if (asmjs_regno_reg_class (i) == FLOAT_REGS)
	    {
	      size += size&4;
	      asm_fprintf (stream, "\t\t%s = +HEAPF64[rp+%d>>3];\n",
			   reg_names[i], size);
	      size += 8;
	    }
	  else
	    {
	      if (i >= 8)
		asm_fprintf (stream, "\t\t%s = HEAP32[rp+%d>>2]|0;\n",
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

void asmjs_start_function(FILE *f, const char *name, tree decl)
{
  char *cooked_name = (char *)alloca(strlen(name)+1);
  const char *p = name;
  char *q = cooked_name;
  tree type = decl ? TREE_TYPE (decl) : NULL;
  enum asmjs_bogotics_type bt_type = bogotics_type;
  int regno;

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
  asm_fprintf(f, "\t.pushsection .javascript%%S,\"a\"\n");
  asm_fprintf(f, "    function f_$\n\t.codetextlabel .L.%s\n", cooked_name);
  asm_fprintf(f, "\t/* %s */ (dpc, sp1, r0, r1, rpc, pc0) {\n", name);
  asm_fprintf(f, "\tdpc = dpc|0;\n");
  asm_fprintf(f, "\tsp1 = sp1|0;\n");
  asm_fprintf(f, "\tr0 = r0|0;\n");
  asm_fprintf(f, "\tr1 = r1|0;\n");
  asm_fprintf(f, "\trpc = rpc|0;\n");
  asm_fprintf(f, "\tpc0 = pc0|0;\n");
  asm_fprintf(f, "\tvar sp = 0;\n");
  asm_fprintf(f, "\tvar fp = 0;\n");
  asm_fprintf(f, "\tvar rp = 0;\n");
  for (regno = R2_REG; regno <= I7_REG; regno++)
    {
      if (df_regs_ever_live_p (regno) || crtl->calls_eh_return)
	asm_fprintf(f, "\tvar %s = 0;\n", reg_names[regno]);
    }
  for (regno = F0_REG; regno <= F7_REG; regno++)
    {
      if (df_regs_ever_live_p (regno) || crtl->calls_eh_return)
	asm_fprintf(f, "\tvar %s = 0.0;\n", reg_names[regno]);
    }
  asm_fprintf(f, "\tvar pc = 0;\n");
  asm_fprintf(f, "\tvar a = 0;\n");
  asm_fprintf(f, "\tsp = sp1-16|0;\n");
  asm_fprintf(f, "\tmainloop:\n");
  asm_fprintf(f, "\twhile(1) {\n");
  switch (breakpoints_type) {
  case BP_SINGLE:
    asm_fprintf(f, "\tif ((HEAP32[bp_addr>>2]|0) == (pc|0)) {\n");
    asm_fprintf(f, "\t\tbp_hit = 1;\n");
    asm_fprintf(f, "\t}\n");
    break;
  case BP_MANY:
    asm_fprintf(f, "\tfor (a = bp_addr|0; (HEAP32[a>>2]|0) != -1; a = ((a|0)+4)|0) {\n");
    asm_fprintf(f, "\t\tif ((HEAP32[a>>2]|0) == (pc|0)) bp_hit = 1;\n");
    asm_fprintf(f, "\t}\n");
    break;
  case BP_PLETHORA:
    asm_fprintf(f, "\tif (foreign_breakpoint_p(pc|0)|0) bp_hit = 1;\n");
    break;
  case BP_NONE:
  default:
    break;
  }
  if (bt_type != BOGOTICS_NONE)
    {
      asm_fprintf(f, "\tbogotics = (bogotics|0)-1|0;\n");
    }
  if ((bt_type == BOGOTICS_ALL) && breakpoints_type != BP_NONE) {
    asm_fprintf(f, "\ta = ((bp_hit)|0) | ((bogotics|0)<0);\n");
    asm_fprintf(f, "\tif ((a|0) != 0) {\n");
    asm_fprintf(f, "\t\tif (fp|0) {\n");
    asm_fprintf(f, "\t\t\trp = fp|1;\n");
    asm_fprintf(f, "\t\t\tbreak mainloop;\n");
    asm_fprintf(f, "\t\t}\n");
    asm_fprintf(f, "\t}\n");
  } else if (bt_type == BOGOTICS_ALL) {
    asm_fprintf(f, "\tif ((bogotics|0) < 0) {\n");
    asm_fprintf(f, "\t\tif (fp|0) {\n");
    asm_fprintf(f, "\t\t\trp = fp|1;\n");
    asm_fprintf(f, "\t\t\tbreak mainloop;\n");
    asm_fprintf(f, "\t\t}\n");
    asm_fprintf(f, "\t}\n");
  } else if (breakpoints_type != BP_NONE) {
    asm_fprintf(f, "\tif ((bp_hit|0) != 0) {\n");
    asm_fprintf(f, "\t\tif (fp|0) {\n");
    asm_fprintf(f, "\t\t\trp = fp|1;\n");
    asm_fprintf(f, "\t\t\tbreak mainloop;\n");
    asm_fprintf(f, "\t\t}\n");
    asm_fprintf(f, "\t}\n");
  }
  asm_fprintf(f, "\tswitch (dpc|0) {\n");
  asm_fprintf(f, "\t.codetextlabeldeffirst %s\n", cooked_name);
}


void asmjs_end_function(FILE *f, const char *name, tree decl ATTRIBUTE_UNUSED)
{
  char *cooked_name = (char *)alloca(strlen(name)+1);
  const char *p = name;
  char *q = cooked_name;
  enum asmjs_bogotics_type bt_type = bogotics_type;
  tree type = decl ? TREE_TYPE (decl) : NULL;

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

  asm_fprintf(f, "\t.codetextlabeldeflast .ends.%s\n", cooked_name);
  asm_fprintf(f, " default:\n");
  asm_fprintf(f, "\tif ((dpc+pc0|0) == 0) {\n");
  asm_fprintf(f, "\t\trp = sp|0;\n");
  asmjs_function_regload (f, decl);
  asm_fprintf(f, "\t\tfp = rp|0;\n");
  asm_fprintf(f, "\t\tcontinue;\n");
  asm_fprintf(f, "\t} else {\n");
  asm_fprintf(f, "\t\tforeign_abort(0|0, dpc|0, pc0|0, 0, 0);\n");
  asm_fprintf(f, "\t}\n");
  asm_fprintf(f, "\t}\n");
  if (use_interrupts)
    {
      asm_fprintf(f, "\tif ((HEAP32[0>>2]|0) != 0) {\n");
      asm_fprintf(f, "\t\tif (fp|0) {\n");
      asm_fprintf(f, "\t\t\trp = fp|1;\n");
      asm_fprintf(f, "\t\t\tbreak mainloop;\n");
      asm_fprintf(f, "\t\t}\n");
      asm_fprintf(f, "\t}\n");
    }
  if (bt_type != BOGOTICS_NONE)
    {
      asm_fprintf(f, "\t{\n");
      asm_fprintf(f, "\tbogotics = (bogotics|0)-1|0;\n");
    }
  if ((bt_type != BOGOTICS_NONE) && breakpoints_type != BP_NONE) {
    asm_fprintf(f, "\ta = ((bp_hit)|0) | ((bogotics|0)<0);\n");
    asm_fprintf(f, "\tif ((a|0) != 0) {\n");
    asm_fprintf(f, "\t\tif (fp|0) {\n");
    asm_fprintf(f, "\t\t\trp = fp|1;\n");
    asm_fprintf(f, "\t\t\tbreak mainloop;\n");
    asm_fprintf(f, "\t\t}\n");
    asm_fprintf(f, "\t}\n");
    asm_fprintf(f, "\t}\n");
  } else if (bt_type != BOGOTICS_NONE) {
    asm_fprintf(f, "\tif ((bogotics|0) < 0) {\n");
    asm_fprintf(f, "\t\tif (fp|0) {\n");
    asm_fprintf(f, "\t\t\trp = fp|1;\n");
    asm_fprintf(f, "\t\t\tbreak mainloop;\n");
    asm_fprintf(f, "\t\t}\n");
    asm_fprintf(f, "\t}\n");
    asm_fprintf(f, "\t}\n");
  } else if (breakpoints_type != BP_NONE) {
    asm_fprintf(f, "\tif ((bp_hit|0) != 0) {\n");
    asm_fprintf(f, "\t\tif (fp|0) {\n");
    asm_fprintf(f, "\t\t\trp = fp|1;\n");
    asm_fprintf(f, "\t\t\tbreak mainloop;\n");
    asm_fprintf(f, "\t\t}\n");
    asm_fprintf(f, "\t}\n");
  }
  asm_fprintf(f, "\t}\n");
  asm_fprintf(f, "\tif ((rp&3) == 1) {\n");
  asm_fprintf(f, "\t\tHEAP32[sp-16>>2] = fp;\n");
  asmjs_function_regstore (f, decl);
  asm_fprintf(f, "\t}\n");
  asm_fprintf(f, "\treturn rp|0;}\n");
  asm_fprintf(f, "\t.popsection\n");
  asm_fprintf(f, "\t.pushsection .special.export,\"a\"\n");
  asm_fprintf(f, "\t.pushsection .javascript%%S,\"a\"\n");
  asm_fprintf(f, "\t.ascii \"f_\"\n\t.codetextlabel .L.%s\n\t.ascii \": f_\"\n\t.codetextlabel .L.%s\n\t.ascii \",\\n\"\n", cooked_name, cooked_name);
  asm_fprintf(f, "\t.popsection\n");
  asm_fprintf(f, "\t.popsection\n");

  asm_fprintf(f, "\t.pushsection .special.define,\"a\"\n");
  asm_fprintf(f, "\t.pushsection .javascript%%S,\"a\"\n");
  asm_fprintf(f, "\t.ascii \"\\tdeffun({name: \\\"f_\"\n\t.codetextlabel .L.%s\n\t.ascii \"\\\", symbol: \\\"%s\\\", pc0: \"\n\t.codetextlabel .L.%s\n\t.ascii \", pc1: \"\n\t.codetextlabel .ends.%s\n\t.ascii \", regsize: %d, regmask: 0x%x});\\n\"\n", cooked_name, cooked_name, cooked_name, cooked_name, asmjs_function_regsize(decl), asmjs_function_regmask(decl));
  asm_fprintf(f, "\t.popsection\n");
  asm_fprintf(f, "\t.popsection\n");

  asm_fprintf(f, "\t.pushsection .special.fpswitch%%S,\"a\"\n");
  asm_fprintf(f, "\t.pushsection .javascript%%S,\"a\"\n");
  asm_fprintf(f, "\tcase $\n\t.codetextlabelr12 .L.%s\n\t:\n\treturn f_$\n\t.codetextlabel .L.%s\n\t(dpc|0, sp|0, r0|0, r1|0, rpc|0, pc0|0)|0;\n", cooked_name, cooked_name);
  asm_fprintf(f, "\t.popsection\n");
  asm_fprintf(f, "\t.popsection\n");
  asm_fprintf(f, "\t.pushsection .javascript%%S,\"a\"\n");
}

void asmjs_output_ascii (FILE *f, const void *ptr, size_t len)
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

void asmjs_output_label (FILE *stream, const char *name)
{
  fprintf(stream, "%s:\n", name + (name[0] == '*'));
}

void asmjs_output_debug_label (FILE *stream, const char *prefix, int num)
{
  fprintf(stream, "\t.labeldef_debug .%s%d\n", prefix, num);
}

void asmjs_output_internal_label (FILE *stream, const char *name)
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

void asmjs_output_labelref (FILE *stream, const char *name)
{
  fprintf(stream, "%s", name + (name[0] == '*'));
}

void asmjs_output_label_ref (FILE *stream, const char *name)
{
  fprintf(stream, "%s", name + (name[0] == '*'));
}

void asmjs_output_symbol_ref (FILE *stream, rtx x)
{
  const char *name = XSTR (x, 0);
  fprintf(stream, "%s", name + (name[0] == '*'));
}

void
asmjs_asm_named_section (const char *name, unsigned int flags,
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
      fprintf (asm_out_file, "\t.pushsection .javascript%%S,\"a\"\n");
    }
}

void asmjs_output_common (FILE *stream, const char *name, size_t size ATTRIBUTE_UNUSED, size_t rounded)
{
  fprintf (stream, "\t.comm ");
  assemble_name(stream, name);
  fprintf (stream, ", %d", (int)rounded);
  fprintf (stream, "\n");
}

void asmjs_output_aligned_decl_local (FILE *stream, tree decl ATTRIBUTE_UNUSED, const char *name, size_t size ATTRIBUTE_UNUSED, size_t align ATTRIBUTE_UNUSED)
{
  fprintf (stream, "\t.local %s\n", name);
}

void asmjs_output_local (FILE *stream, const char *name, size_t size ATTRIBUTE_UNUSED, size_t rounded ATTRIBUTE_UNUSED)
{
  fprintf (stream, "\t.local ");
  assemble_name(stream, name);
  fprintf (stream, "\n");
  fprintf (stream, "\t.comm ");
  assemble_name(stream, name);
  fprintf (stream, ", %d", (int)rounded);
  fprintf (stream, "\n");
}

void asmjs_output_skip (FILE *stream, size_t bytes)
{
  size_t n;
  for (n = 0; n < bytes; n++) {
    fprintf (stream, "\t.QI 0\n");
  }
}

void asmjs_output_aligned_bss (FILE *stream, const_tree tree ATTRIBUTE_UNUSED, const char *name,
			       unsigned HOST_WIDE_INT size,
			       unsigned HOST_WIDE_INT rounded ATTRIBUTE_UNUSED)
{
  fprintf (stream, ".aligned-bss %s %d %d\n", name,
	   (int) size, (int) rounded);
}

bool asmjs_hard_regno_mode_ok(unsigned int regno, machine_mode mode)
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

bool asmjs_hard_regno_rename_ok(unsigned int from, unsigned int to)
{
  return asmjs_regno_reg_class(from) == asmjs_regno_reg_class(to);
}

bool asmjs_modes_tieable_p(machine_mode mode1, machine_mode mode2)
{
  return ((mode1 == SFmode || mode1 == DFmode) ==
	  (mode2 == SFmode || mode2 == DFmode));
}

bool asmjs_regno_ok_for_index_p(unsigned int regno)
{
  if (regno >= FIRST_PSEUDO_REGISTER)
    return false;

  return asmjs_regno_reg_class(regno) == GENERAL_REGS;
}

bool asmjs_regno_ok_for_base_p(unsigned int regno)
{
  if (regno >= FIRST_PSEUDO_REGISTER)
    return false;

  return asmjs_regno_reg_class(regno) == GENERAL_REGS;
}

/* Uh, I don't understand the documentation for this. Using anything
 * but 4 breaks df_read_modify_subreg. */
int asmjs_regmode_natural_size(machine_mode ATTRIBUTE_UNUSED mode)
{
  return 4;
}

int asmjs_hard_regno_nregs(unsigned int regno, machine_mode mode)
{
  if (asmjs_regno_reg_class(regno) == FLOAT_REGS &&
      mode == DFmode)
    return 1;
  else
    return (GET_MODE_SIZE(mode) + UNITS_PER_WORD - 1)/UNITS_PER_WORD;
}

static int asmjs_register_priority(int regno)
{
  switch (asmjs_regno_reg_class (regno)) {
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


void asmjs_expand_builtin_va_start (tree valist, rtx nextarg)
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

int asmjs_return_pops_args(tree fundecl ATTRIBUTE_UNUSED,
			   tree funtype ATTRIBUTE_UNUSED,
			   int size ATTRIBUTE_UNUSED)
{
  return 0;
}

int asmjs_call_pops_args(CUMULATIVE_ARGS size ATTRIBUTE_UNUSED)
{
  return 0;
}

rtx asmjs_dynamic_chain_address(rtx frameaddr)
{
  return gen_rtx_MEM (SImode, frameaddr);
}

rtx asmjs_incoming_return_addr_rtx(void)
{
  return gen_rtx_REG (SImode, RPC_REG);
  //return gen_rtx_MEM (SImode, plus_constant (SImode, gen_rtx_MEM (SImode, frame_pointer_rtx), 8));
}

rtx asmjs_return_addr_rtx(int count ATTRIBUTE_UNUSED, rtx frameaddr)
{
  if (count == 0)
    return gen_rtx_ASHIFT (SImode, gen_rtx_REG (SImode, RPC_REG), gen_rtx_CONST_INT (SImode, 4));
  else
    {
      return gen_rtx_MEM (SImode, gen_rtx_PLUS (SImode, gen_rtx_MEM (SImode, asmjs_dynamic_chain_address(frameaddr)), gen_rtx_CONST_INT (SImode, 8)));
    }
}

int asmjs_first_parm_offset(const_tree fntype ATTRIBUTE_UNUSED)
{
  return 0;
}

rtx asmjs_expand_call (rtx retval, rtx address, rtx callarg1)
{
  int argcount;
  rtx use = NULL, call;
  rtx_insn *call_insn;
  rtx sp = gen_rtx_REG (SImode, SP_REG);
  tree funtype = cfun->machine->funtype;

  argcount = asmjs_argument_count (funtype);

  if (asmjs_is_real_stackcall (funtype))
    {
      rtx loc = gen_rtx_MEM (SImode,
			     gen_rtx_PLUS (SImode,
					   sp,
					   gen_rtx_CONST_INT (SImode,
							      -4)));

      emit_move_insn (loc, gen_rtx_CONST_INT (SImode, argcount));
    }

  if (asmjs_is_real_stackcall (funtype))
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

rtx asmjs_expand_prologue()
{
  HOST_WIDE_INT size = get_frame_size () + crtl->outgoing_args_size;
  rtx sp = gen_rtx_REG (SImode, SP_REG);
  size = (size + 7) & -8;
  int regsize = asmjs_function_regsize (NULL_TREE);
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

rtx asmjs_expand_epilogue()
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

const char *asmjs_expand_ret_insn()
{
  char buf[1024];
  snprintf (buf, 1022, "return fp+%d|0;\n\t.set __asmjs_fallthrough, 0",
	    asmjs_function_regsize (NULL_TREE));

  return ggc_strdup (buf);
}

int asmjs_max_conditional_execute()
{
  return max_conditional_insns;
}

bool asmjs_can_eliminate(int reg0, int reg1)
{
  if (reg0 == AP_REG && reg1 == FP_REG)
    return true;
  if (reg0 == AP_REG && reg1 == SP_REG)
    return true;

  return false;
}

int asmjs_initial_elimination_offset(int reg0, int reg1)
{
  if (reg0 == AP_REG && reg1 == FP_REG)
    return 16 + asmjs_function_regsize (NULL_TREE);
  else if (reg0 == FP_REG && reg1 == AP_REG)
    return -16 - asmjs_function_regsize (NULL_TREE);
  if (reg0 == FP_REG && reg1 == SP_REG)
    return 0;
  else if (reg0 == SP_REG && reg1 == FP_REG)
    return 0;
  if (reg0 == AP_REG && reg1 == SP_REG)
    return 16 + asmjs_function_regsize (NULL_TREE);
  else if (reg0 == SP_REG && reg1 == AP_REG)
    return -16 - asmjs_function_regsize (NULL_TREE);
  else
    gcc_unreachable ();
}

int asmjs_branch_cost(bool speed_p ATTRIBUTE_UNUSED,
		      bool predictable_p ATTRIBUTE_UNUSED)
{
  return my_branch_cost;
}

int asmjs_register_move_cost(machine_mode mode ATTRIBUTE_UNUSED,
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

int asmjs_memory_move_cost(machine_mode mode ATTRIBUTE_UNUSED,
			   reg_class_t from ATTRIBUTE_UNUSED,
			   bool in ATTRIBUTE_UNUSED)
{
  return my_memory_cost;
}

bool asmjs_rtx_costs (rtx x, machine_mode mode ATTRIBUTE_UNUSED,
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

bool asmjs_lra_p ()
{
  return true;
}


bool asmjs_cxx_library_rtti_comdat(void)
{
  return true;
}

bool asmjs_cxx_class_data_always_comdat(void)
{
  return true;
}


static rtx
asmjs_trampoline_adjust_address (rtx addr)
{
  return addr;
}

/* Trampolines are merely three-word blocks of data aligned to a
 * 16-byte boundary; "TRAM" followed by a function pointer followed by
 * a value to load in the static chain register.  The JavaScript code
 * handles the recognition and evaluation of trampolines, so it's
 * quite slow. */

static void
asmjs_trampoline_init (rtx m_tramp, tree fndecl, rtx static_chain)
{
  rtx fnaddr = force_reg (Pmode, XEXP (DECL_RTL (fndecl), 0));
  m_tramp = asmjs_trampoline_adjust_address (XEXP (m_tramp, 0));
  m_tramp = force_reg (Pmode, m_tramp);
  emit_insn (gen_rtx_SET (gen_rtx_MEM (SImode, m_tramp), gen_rtx_CONST_INT(SImode, 0x4d4a5254)));
  emit_insn (gen_rtx_SET (m_tramp, plus_constant (SImode, m_tramp, 4)));
  emit_insn (gen_rtx_SET (gen_rtx_MEM (SImode, m_tramp), fnaddr));
  emit_insn (gen_rtx_SET (m_tramp, plus_constant (SImode, m_tramp, 4)));
  emit_insn (gen_rtx_SET (gen_rtx_MEM (SImode, m_tramp), static_chain));
  emit_insn (gen_rtx_SET (m_tramp, plus_constant (SImode, m_tramp, -8)));
}

static bool
asmjs_asm_can_output_mi_thunk (const_tree, HOST_WIDE_INT, HOST_WIDE_INT,
			       const_tree)
{
  return true;
}

#if 0 /* unused */
/* Return RTX for the first (implicit this) parameter passed to a method. */

static rtx
asmjs_this_parameter (tree function)
{
  if (asmjs_is_stackcall (TREE_TYPE (function)))
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
asmjs_asm_output_mi_thunk (FILE *f, tree thunk, HOST_WIDE_INT delta,
			   HOST_WIDE_INT vcall_offset, tree function)
{
  const char *tname = XSTR (XEXP (DECL_RTL (function), 0), 0);
  const char *name = XSTR (XEXP (DECL_RTL (thunk), 0), 0);
  bool stackcall = asmjs_is_stackcall (TREE_TYPE (function));
  bool structret = aggregate_value_p (TREE_TYPE (TREE_TYPE (function)), TREE_TYPE (function));
  const char *r = (structret && !stackcall) ? "r1" : "r0";

  tname += tname[0] == '*';
  name += name[0] == '*';

  if (stackcall)
    {
      asm_fprintf (f, "\t%s = (sp|0) + %d|0;\n", r,
		   structret ? 20 : 16);
      asm_fprintf (f, "\t%s = HEAP32[%s>>2]|0;\n", r, r);
    }

  asm_fprintf (f, "\t%s = (%s|0) + (%d)|0;\n", r, r, (int)delta);
  if (vcall_offset)
    {
      asm_fprintf (f, "\trv = HEAP32[%s>>2]|0;\n", r);
      asm_fprintf (f, "\trv = (rv|0) + %d|0;\n", (int)vcall_offset);
      asm_fprintf (f, "\trv = HEAP32[rv>>2]|0;\n");
      asm_fprintf (f, "\t%s = (%s|0) + (rv|0)|0;\n", r, r);
    }

  if (stackcall)
    asm_fprintf (f, "\tHEAP32[(sp|0)+%d>>2] = %s|0;\n",
		 structret ? 20 : 16, r);

  asm_fprintf (f, "\treturn f_$\n\t.codetextlabel %s\n\t(0, sp+16|0, r0|0, r1|0, pc|0, $\n\t.codetextlabel %s\n\t>>4)|0;\n",
	       tname,tname);
}

#undef TARGET_ASM_GLOBALIZE_LABEL
#define TARGET_ASM_GLOBALIZE_LABEL asmjs_globalize_label

#undef TARGET_LEGITIMATE_CONSTANT_P
#define TARGET_LEGITIMATE_CONSTANT_P asmjs_legitimate_constant_p

#undef TARGET_LEGITIMATE_ADDRESS_P
#define TARGET_LEGITIMATE_ADDRESS_P asmjs_legitimate_address_p

#undef TARGET_STRICT_ARGUMENT_NAMING
#define TARGET_STRICT_ARGUMENT_NAMING asmjs_strict_argument_naming

#undef TARGET_FUNCTION_ARG_BOUNDARY
#define TARGET_FUNCTION_ARG_BOUNDARY asmjs_function_arg_boundary

#undef TARGET_FUNCTION_ARG_ROUND_BOUNDARY
#define TARGET_FUNCTION_ARG_ROUND_BOUNDARY asmjs_function_arg_round_boundary

#undef TARGET_FUNCTION_ARG_ADVANCE
#define TARGET_FUNCTION_ARG_ADVANCE asmjs_function_arg_advance

#undef TARGET_FUNCTION_INCOMING_ARG
#define TARGET_FUNCTION_INCOMING_ARG asmjs_function_incoming_arg

#undef TARGET_FUNCTION_ARG
#define TARGET_FUNCTION_ARG asmjs_function_arg

#undef TARGET_FRAME_POINTER_REQUIRED
#define TARGET_FRAME_POINTER_REQUIRED asmjs_frame_pointer_required

#undef TARGET_DEBUG_UNWIND_INFO
#define TARGET_DEBUG_UNWIND_INFO  asmjs_debug_unwind_info

#undef TARGET_CALL_ARGS
#define TARGET_CALL_ARGS asmjs_call_args

#undef TARGET_END_CALL_ARGS
#define TARGET_END_CALL_ARGS asmjs_end_call_args

#undef TARGET_OPTION_OVERRIDE
#define TARGET_OPTION_OVERRIDE asmjs_option_override

#undef TARGET_ATTRIBUTE_TABLE
#define TARGET_ATTRIBUTE_TABLE asmjs_attribute_table

#undef TARGET_REGISTER_PRIORITY
#define TARGET_REGISTER_PRIORITY asmjs_register_priority

#undef TARGET_RTX_COSTS
#define TARGET_RTX_COSTS asmjs_rtx_costs

#undef TARGET_CXX_LIBRARY_RTTI_COMDAT
#define TARGET_CXX_LIBRARY_RTTI_COMDAT asmjs_cxx_library_rtti_comdat

#undef TARGET_CXX_CLASS_DATA_ALWAYS_COMDAT
#define TARGET_CXX_CLASS_DATA_ALWAYS_COMDAT asmjs_cxx_class_data_always_comdat

#undef TARGET_TRAMPOLINE_ADJUST_ADDRESS
#define TARGET_TRAMPOLINE_ADJUST_ADDRESS asmjs_trampoline_adjust_address

#undef TARGET_TRAMPOLINE_INIT
#define TARGET_TRAMPOLINE_INIT asmjs_trampoline_init

#undef TARGET_ABSOLUTE_BIGGEST_ALIGNMENT
#define TARGET_ABSOLUTE_BIGGEST_ALIGNMENT 1024

#undef TARGET_ASM_OUTPUT_MI_THUNK
#define TARGET_ASM_OUTPUT_MI_THUNK asmjs_asm_output_mi_thunk

#undef TARGET_ASM_CAN_OUTPUT_MI_THUNK
#define TARGET_ASM_CAN_OUTPUT_MI_THUNK asmjs_asm_can_output_mi_thunk

#define TARGET_PROMOTE_FUNCTION_MODE asmjs_promote_function_mode
#define TARGET_ABSOLUTE_BIGGEST_ALIGNMENT 1024
#define TARGET_CAN_ELIMINATE asmjs_can_eliminate
#define TARGET_PROMOTE_PROTOTYPES asmjs_promote_prototypes
#define TARGET_RETURN_POPS_ARGS asmjs_return_pops_args
#define TARGET_FUNCTION_VALUE asmjs_function_value
#define TARGET_LIBCALL_VALUE asmjs_libcall_value
#define TARGET_FUNCTION_VALUE_REGNO_P asmjs_function_value_regno_p
#define TARGET_RETURN_IN_MEMORY asmjs_return_in_memory
#define TARGET_STRUCT_VALUE_RTX asmjs_struct_value_rtx
#define TARGET_REGISTER_MOVE_COST asmjs_register_move_cost
#define TARGET_MEMORY_MOVE_COST asmjs_memory_move_cost
#define TARGET_LRA_P asmjs_lra_p

#include "target-def.h"

struct gcc_target targetm = TARGET_INITIALIZER;
