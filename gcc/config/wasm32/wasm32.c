/* GCC backend for the WebAssembly target
   Copyright (C) 1991-2015 Free Software Foundation, Inc.
   Copyright (C) 2016-2017 Pip Cet <pipcet@gmail.com>

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
#include "input.h"
#include "alias.h"
#include "symtab.h"
#include "wide-int.h"
#include "poly-int.h"
#include "poly-int-types.h"
#include "inchash.h"
#include "tree-core.h"
#include "stor-layout.h"
#include "tree.h"
#include "c-family/c-common.h"
#include "rtl.h"
#include "predict.h"
#include "dominance.h"
#include "function.h"
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
#include "print-tree.h"
#include "varasm.h"
#include "print-rtl.h"
#include "tree-pass.h"
#include "stringpool.h"
#include "attribs.h"

#define N_ARGREG_PASSED 2
#define N_ARGREG_GLOBAL 2

extern void
debug_tree (tree node);

void
wasm32_cpu_cpp_builtins (struct cpp_reader * pfile)
{
  cpp_assert(pfile, "cpu=wasm32");
  cpp_assert(pfile, "machine=wasm32");

  cpp_define(pfile, "__WASM32__");
}

bool
linux_libc_has_function(enum function_class cl ATTRIBUTE_UNUSED)
{
  return true;
}

void
wasm32_globalize_label (FILE * stream, const char *name)
{
  asm_fprintf (stream, "\t.global ");
  assemble_name (stream, name);
  asm_fprintf (stream, "\n");
}

void
wasm32_weaken_label (FILE *stream, const char *name)
{
  asm_fprintf (stream, "\t.weak ");
  assemble_name (stream, name);
  asm_fprintf (stream, "\n");
}

void
wasm32_output_def (FILE *stream, const char *alias, const char *name)
{
  asm_fprintf (stream, "\t.set ");
  assemble_name (stream, alias);
  asm_fprintf (stream, ", ");
  assemble_name (stream, name);
  asm_fprintf (stream, "\n");
}

bool
wasm32_assemble_integer (rtx x, unsigned int size, int aligned_p ATTRIBUTE_UNUSED)
{
  bool is_fpointer = false;
  const char *op = NULL;

  if (SYMBOL_REF_P (x) && SYMBOL_REF_FUNCTION_P (x))
    is_fpointer = true;

  switch (size) {
  case 1:
    op = ".byte";
    break;
  case 2:
    op = ".short";
    break;
  case 4:
    op = ".long";
    break;
  case 8:
    op = ".quad";
    break;
  }

  if (op) {
    fputc ('\t', asm_out_file);
    if (is_fpointer) {
      fputs (".reloc .,R_WASM32_32_CODE,", asm_out_file);
      output_addr_const (asm_out_file, x);
      fputc ('\n', asm_out_file);
      fputc ('\t', asm_out_file);
      fputs (op, asm_out_file);
      fputs (" 0\n", asm_out_file);
    } else {
      fputs (op, asm_out_file);
      fputc (' ', asm_out_file);
      output_addr_const (asm_out_file, x);
      fputc ('\n', asm_out_file);
    }
    return true;
  }

  return false;
}

enum reg_class wasm32_regno_reg_class(int regno)
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
wasm32_legitimate_constant_p (machine_mode mode ATTRIBUTE_UNUSED, rtx x)
{
  return (CONST_INT_P (x)
	  || CONST_DOUBLE_P (x)
	  || CONSTANT_ADDRESS_P (x));
}

typedef struct
{
  int count;
  int const_count;
  int indir;
}
wasm32_legitimate_address;

static bool
wasm32_legitimate_address_p_rec (wasm32_legitimate_address *r,
				 machine_mode mode, rtx x, bool strict_p);

static bool
wasm32_legitimate_address_p_rec (wasm32_legitimate_address *r,
				 machine_mode mode, rtx x, bool strict_p)
{
  enum rtx_code code = GET_CODE (x);

  if (MEM_P(x))
    {
      r->indir++;
      return wasm32_legitimate_address_p_rec (r, mode, XEXP (x, 0), strict_p);
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
      if (!wasm32_legitimate_address_p_rec (r, mode, XEXP (x, 0), strict_p)
	  || !wasm32_legitimate_address_p_rec (r, mode, XEXP (x, 1), strict_p))
	return false;

      return true;
    }

  return false;
}

static bool
wasm32_legitimate_address_p (machine_mode mode, rtx x, bool strict_p)
{
  wasm32_legitimate_address r = { 0, 0, 0 };
  bool s;

  s = wasm32_legitimate_address_p_rec (&r, mode, x, strict_p);

#if 0
  debug_rtx(x);
  fprintf(stderr, "%d %d %d %d\n",
	  s, r.count, r.const_count, r.indir);
#endif

  if (!s)
    return false;

  return (r.count <= MAX_REGS_PER_ADDRESS) && (r.count <= max_rpi)
    && (r.indir <= 0) && (r.const_count <= 1);
}

static char *
wasm32_signature_string (const_tree fntype)
{
  function_args_iterator args_iter;
  tree t;
  int count = 0;

  if (!fntype)
    return NULL;

  FOREACH_FUNCTION_ARGS (fntype, t, args_iter)
    {
      count++;
    }

  char *ret = (char *)xmalloc (count + 3);
  char *p = ret;
  *p++ = 'F';

  FOREACH_FUNCTION_ARGS (fntype, t, args_iter)
    {
      switch (TYPE_MODE (t))
	{
	case QImode:
	case HImode:
	case SImode:
	  *p++ = 'i';
	  break;
	case DImode:
	  *p++ = 'l';
	  break;
	case SFmode:
	  *p++ = 'f';
	  break;
	case DFmode:
	  *p++ = 'd';
	  break;
	case VOIDmode:
	  break;
	default:
	  abort ();
	}
    }

  switch (TYPE_MODE (TREE_TYPE (fntype)))
    {
    case QImode:
    case HImode:
    case SImode:
      *p++ = 'i';
      break;
    case DImode:
      *p++ = 'l';
      break;
    case SFmode:
      *p++ = 'f';
      break;
    case DFmode:
      *p++ = 'd';
      break;
    case VOIDmode:
      *p++ = 'v';
      break;
    default:
      abort ();
    }

  *p++ = 'E';
  *p++ = 0;

  return ret;
}

static int wasm32_argument_count (const_tree fntype)
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

  if (n && VOID_TYPE_P (n))
    ret--;

  /* This is a hack.  We'd prefer to instruct gcc to pass an invisible
     structure return argument even for functions returning void, but
     that requires changes to the core code, and it frightens me a
     little to do that because it might cause obscure ICEs.  */
  ret = ((n != NULL_TREE && n != void_type_node) ? -1 : 1) * ret;

  if (VOID_TYPE_P (TREE_TYPE (fntype)))
    ret ^= 0x40000000L;

  return ret;
}

static bool
wasm32_is_stackcall (const_tree type)
{
  if (!type)
    return false;

  tree attrs = TYPE_ATTRIBUTES (type);
  if (attrs != NULL_TREE)
    {
      if (lookup_attribute ("stackcall", attrs))
	return true;

      if (lookup_attribute ("rawcall", attrs))
	return true;
    }

  if (wasm32_argument_count (type) < 0)
    return true;

  return false;
}

static bool
wasm32_is_rawcall (const_tree type)
{
  if (!type)
    return false;

  tree attrs = TYPE_ATTRIBUTES (type);
  if (attrs != NULL_TREE)
    {
      if (lookup_attribute ("rawcall", attrs))
	return true;
    }

  return false;
}

static bool
wasm32_is_real_stackcall (const_tree type)
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

void wasm32_init_cumulative_args (CUMULATIVE_ARGS *cum_p,
				  const_tree fntype ATTRIBUTE_UNUSED,
				  rtx libname ATTRIBUTE_UNUSED,
				  const_tree fndecl,
				  unsigned int n_named_args ATTRIBUTE_UNUSED)
{
  cum_p->n_areg = 0;
  cum_p->n_vreg = 0;
  cum_p->is_stackcall = wasm32_is_stackcall (fndecl ? TREE_TYPE (fndecl) : NULL_TREE) || wasm32_is_stackcall (fntype);
}

bool
wasm32_strict_argument_naming (cumulative_args_t cum ATTRIBUTE_UNUSED)
{
  return true;
}

bool
wasm32_function_arg_regno_p(int regno)
{
  /* XXX is this incoming or outgoing? */
  return ((regno >= R0_REG && regno < R0_REG + N_ARGREG_PASSED)
	  || (regno >= A0_REG && regno < A0_REG + N_ARGREG_GLOBAL));
}

unsigned int
wasm32_function_arg_boundary (machine_mode mode, const_tree type)
{
  if (GET_MODE_ALIGNMENT (mode) > PARM_BOUNDARY
      || (type && TYPE_ALIGN (type) > PARM_BOUNDARY)) {
    return PARM_BOUNDARY * 2;
  } else
    return PARM_BOUNDARY;
}

unsigned int
wasm32_function_arg_round_boundary (machine_mode mode, const_tree type)
{
  if (GET_MODE_ALIGNMENT (mode) > PARM_BOUNDARY
      || (type && TYPE_ALIGN (type) > PARM_BOUNDARY)) {
    return PARM_BOUNDARY * 2;
  } else
    return PARM_BOUNDARY;
}

unsigned int
wasm32_function_arg_offset (machine_mode mode ATTRIBUTE_UNUSED,
			    const_tree type ATTRIBUTE_UNUSED)
{
  return 0;
}

static void
wasm32_function_arg_advance (cumulative_args_t cum_v,
			     const function_arg_info& info)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (cum_v);

  switch (info.mode)
    {
    case QImode:
    case HImode:
    case SImode:
      cum->n_areg++;
    default:
      break;
    }
}

static rtx
wasm32_function_incoming_arg (cumulative_args_t pcum_v,
			      const function_arg_info &info)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (pcum_v);

  switch (info.mode)
    {
    case VOIDmode:
      return const0_rtx;
    case QImode:
    case HImode:
    case SImode:
      if (cum->n_areg < N_ARGREG_PASSED && !cum->is_stackcall)
	return gen_rtx_REG (info.mode, R0_REG + cum->n_areg);
      else if (cum->n_areg < N_ARGREG_PASSED + N_ARGREG_GLOBAL
	       && !cum->is_stackcall)
	return gen_rtx_REG (info.mode, A0_REG + cum->n_areg - N_ARGREG_PASSED);
      return NULL_RTX;
    case SFmode:
    case DFmode:
    default:
      return NULL_RTX;
    }

  return NULL_RTX;
}

static rtx
wasm32_function_arg (cumulative_args_t pcum_v,
		     const function_arg_info &info)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (pcum_v);

  switch (info.mode)
    {
    case VOIDmode:
      return const0_rtx;
    case QImode:
    case HImode:
    case SImode:
      if (cum->n_areg < N_ARGREG_PASSED && !cum->is_stackcall)
	return gen_rtx_REG (info.mode, R0_REG + cum->n_areg);
      else if (cum->n_areg < N_ARGREG_PASSED + N_ARGREG_GLOBAL
	       && !cum->is_stackcall)
	return gen_rtx_REG (info.mode, A0_REG + cum->n_areg - N_ARGREG_PASSED);
      return NULL_RTX;
    case SFmode:
    case DFmode:
    default:
      return NULL_RTX;
    }

  return NULL_RTX;
}

bool wasm32_frame_pointer_required (void)
{
  return true;
}

enum unwind_info_type wasm32_debug_unwind_info (void)
{
  return UI_DWARF2;
}

static void
wasm32_call_args (rtx arg ATTRIBUTE_UNUSED, tree funtype)
{
  if (cfun->machine->funtype != funtype)
    cfun->machine->funtype = funtype;
}

static void
wasm32_end_call_args (void)
{
  cfun->machine->funtype = NULL;
}

static struct machine_function *
wasm32_init_machine_status (void)
{
  struct machine_function *p = ggc_cleared_alloc<machine_function> ();

  return p;
}

static void
wasm32_option_override (void)
{
  init_machine_status = wasm32_init_machine_status;
}

static tree
wasm32_handle_cconv_attribute (tree *node ATTRIBUTE_UNUSED,
			      tree name ATTRIBUTE_UNUSED,
			      tree args ATTRIBUTE_UNUSED, int,
			      bool *no_add_attrs ATTRIBUTE_UNUSED)
{
  return NULL;
}

static vec<struct wasm32_jsexport_decl> wasm32_jsexport_decls;

__attribute__((weak)) void
wasm32_jsexport (tree node ATTRIBUTE_UNUSED,
		struct wasm32_jsexport_opts *opts ATTRIBUTE_UNUSED,
		vec<struct wasm32_jsexport_decl> *decls ATTRIBUTE_UNUSED)
{
  error ("jsexport not defined for this frontend");
}

#include <print-tree.h>
#include <plugin.h>

static void wasm32_jsexport_unit_callback (void *, void *)
{
  unsigned i,j;
  struct wasm32_jsexport_decl *p;
  if (!wasm32_jsexport_decls.is_empty())
    {
      switch_to_section (get_section (".jsexport", SECTION_MERGE|SECTION_STRINGS|1, NULL_TREE));

      FOR_EACH_VEC_ELT (wasm32_jsexport_decls, i, p)
	{
	  const char **pstr;
	  const char *str;
	  int c;
	  FOR_EACH_VEC_ELT (p->fragments, j, pstr)
	    {
	      if (*pstr == NULL)
		{
		  fprintf (asm_out_file, "\t.reloc .+2,R_WASM32_HEX16,%s\n",
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

  wasm32_jsexport_decls = vec<struct wasm32_jsexport_decl>();
}

static void wasm32_jsexport_parse_args (tree args, struct wasm32_jsexport_opts *opts)
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

static void wasm32_jsexport_decl_callback (void *gcc_data, void *)
{
  tree decl = (tree)gcc_data;
  bool found = false;
  struct wasm32_jsexport_opts opts;

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
	      wasm32_jsexport_parse_args (TREE_VALUE (attr), &opts);
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
	      wasm32_jsexport_parse_args (TREE_VALUE (attr), &opts);
	    }
	}
    }

  if (found)
    wasm32_jsexport(decl, &opts, &wasm32_jsexport_decls);
}

static bool wasm32_jsexport_plugin_inited = false;

static void wasm32_jsexport_plugin_init (void)
{
  register_callback("jsexport", PLUGIN_FINISH_DECL,
		    wasm32_jsexport_decl_callback, NULL);
  register_callback("jsexport", PLUGIN_FINISH_TYPE,
		    wasm32_jsexport_decl_callback, NULL);
  register_callback("jsexport", PLUGIN_FINISH_UNIT,
		    wasm32_jsexport_unit_callback, NULL);
  flag_plugin_added = true;

  wasm32_jsexport_plugin_inited = true;
}

static tree
wasm32_handle_jsexport_attribute (tree * node, tree attr_name ATTRIBUTE_UNUSED,
				  tree args, int,
				  bool *no_add_attrs ATTRIBUTE_UNUSED)
{
  struct wasm32_jsexport_opts opts;

  opts.jsname = NULL;
  opts.recurse = 0;

  if (!wasm32_jsexport_plugin_inited)
    wasm32_jsexport_plugin_init ();

  if (TREE_CODE (*node) == TYPE_DECL)
    {
      wasm32_jsexport_parse_args (args, &opts);
      wasm32_jsexport (*node, &opts, &wasm32_jsexport_decls);
    }

  return NULL_TREE;
}

static vec<struct wasm32_imexport_decl> wasm32_imexport_decls;

static void wasm32_imexport_unit_callback (void *, void *)
{
  unsigned i,j;
  struct wasm32_imexport_decl *p;
  const char *pstr;

  if (!wasm32_imexport_decls.is_empty())
    {
      FOR_EACH_VEC_ELT (wasm32_imexport_decls, i, p)
	{
	  FOR_EACH_VEC_ELT (p->fragments, j, pstr)
	    {
	      fprintf (asm_out_file, "\t%s\n", pstr);
	    }
	}
    }

  wasm32_imexport_decls = vec<struct wasm32_imexport_decl>();
}

static bool
wasm32_import_parse (const_tree decl, const char **name,
		     const char **module, const char **field)
{
  tree attrs = DECL_ATTRIBUTES (decl);
  if (attrs)
    {
      tree imattr = lookup_attribute ("import", attrs);
      if (imattr)
	{
	  tree args = TREE_VALUE (imattr);
	  tree arg = TREE_VALUE (args);;

	  *name = IDENTIFIER_POINTER (DECL_NAME (decl));

	  *module = TREE_STRING_POINTER (arg);
	  args = TREE_CHAIN (args);
	  if (args)
	    arg = TREE_VALUE (args);

	  if (args && TREE_CODE (arg) == STRING_CST)
	    *field = TREE_STRING_POINTER (arg);
	  else if (TREE_CODE (decl) == FUNCTION_DECL
		   || TREE_CODE (decl) == VAR_DECL)
	    *field = IDENTIFIER_POINTER (DECL_NAME (decl));
	  else
	    *field = *name;

	  return true;
	}
    }

  return false;
}

static bool
wasm32_export_parse (const_tree decl, const char **name, const char **field)
{
  tree attrs = DECL_ATTRIBUTES (decl);
  if (attrs)
    {
      tree exattr = lookup_attribute ("export", attrs);
      if (exattr)
	{
	  tree args = TREE_VALUE (exattr);

	  *name = IDENTIFIER_POINTER (DECL_NAME (decl));

	  if (!args)
	    *field = *name;
	  else
	    {
	      tree arg = TREE_VALUE (args);

	      *field = TREE_STRING_POINTER (arg);
	    }

	  return true;
	}
    }

  return false;
}

static void
wasm32_imexport_decl_callback (void *gcc_data, void *)
{
  tree decl = (tree) gcc_data;

  if (DECL_P (decl))
    {
      const char *name;
      const char *module;
      const char *field;

      if (wasm32_import_parse (decl, &name, &module, &field))
	{
	  struct wasm32_imexport_decl ret;

	  ret.fragments = vec<const char *> ();

	  if (TREE_CODE (decl) == VAR_DECL)
	    {
	      bool is_mutable = false; /* XXX */
	      ret.fragments.safe_push
		(concat ("import_global ", name,
			 ", \"", module, "\", \"", field,
			 "\", ", is_mutable ? "1" : "0",
			 NULL));
	    }
	  else if (TREE_CODE (decl) == FUNCTION_DECL)
	    {
	      const char *sig;

	      if (lookup_attribute ("rawcall", TYPE_ATTRIBUTES (TREE_TYPE (decl))))
		{
		  sig = wasm32_signature_string (TREE_TYPE (decl));
		  ret.fragments.safe_push (concat ("createsig ", sig, NULL));
		}
	      else
		sig = "FiiiiiiiE";

	      ret.fragments.safe_push
		(concat ("import_function ", name,
			 ", \"", module, "\", \"", field,
			 "\", __sigchar_", sig, NULL));
	    }

	  wasm32_imexport_decls.safe_push (ret);
	}

      if (wasm32_export_parse (decl, &name, &field))
	{
	  struct wasm32_imexport_decl ret;

	  ret.fragments = vec<const char *> ();

	  if (TREE_CODE (decl) == VAR_DECL)
	    ret.fragments.safe_push (concat ("export_global ", name,
					     ", \"", field, "\"", NULL));
	  else if (TREE_CODE (decl) == FUNCTION_DECL)
	    ret.fragments.safe_push (concat ("export_function ", name,
					     ", \"", field, "\"", NULL));

	  wasm32_imexport_decls.safe_push (ret);
	}
    }
}

static bool
wasm32_imexport_plugin_inited = false;

static void
wasm32_imexport_plugin_init (void)
{
  if (!wasm32_imexport_plugin_inited)
    {
      register_callback("imexport", PLUGIN_FINISH_DECL,
			wasm32_imexport_decl_callback, NULL);
      register_callback("imexport", PLUGIN_FINISH_TYPE,
			wasm32_imexport_decl_callback, NULL);
      register_callback("imexport", PLUGIN_FINISH_UNIT,
			wasm32_imexport_unit_callback, NULL);
      flag_plugin_added = true;

      wasm32_imexport_plugin_inited = true;
    }
}

static tree
wasm32_handle_import_attribute (tree * node, tree attr_name ATTRIBUTE_UNUSED,
				tree args, int,
				bool *no_add_attrs ATTRIBUTE_UNUSED)
{
  const char *module ATTRIBUTE_UNUSED;
  const char *field ATTRIBUTE_UNUSED;
  tree arg;

  wasm32_imexport_plugin_init ();
  wasm32_imexport_decl_callback ((void *)*node, NULL);

  arg = TREE_VALUE (args);

  if (TREE_CODE (arg) != STRING_CST)
    {
      *no_add_attrs = true;
      return NULL_TREE;
    }

  module = TREE_STRING_POINTER (arg);

  args = TREE_CHAIN (args);

  if (args)
    arg = TREE_VALUE (args);

  if (args && TREE_CODE (arg) == STRING_CST)
    {
      field = TREE_STRING_POINTER (arg);
    }
  else if (TREE_CODE (*node) == FUNCTION_DECL)
    {
      field = IDENTIFIER_POINTER (DECL_NAME (*node));
    }
  else if (TREE_CODE (*node) == VAR_DECL)
    {
      field = IDENTIFIER_POINTER (DECL_NAME (*node));
    }
  else
    {
      *no_add_attrs = true;
      return NULL_TREE;
    }

  return NULL_TREE;
}

static tree
wasm32_handle_export_attribute (tree * node, tree attr_name ATTRIBUTE_UNUSED,
				tree args, int,
				bool *no_add_attrs ATTRIBUTE_UNUSED)
{
  const char *module ATTRIBUTE_UNUSED;
  const char *field ATTRIBUTE_UNUSED;
  tree arg;

  wasm32_imexport_plugin_init ();
  wasm32_imexport_decl_callback ((void *)*node, NULL);

  if (!args)
    {
      field = IDENTIFIER_POINTER (DECL_NAME (*node));
      return NULL_TREE;
    }

  arg = TREE_VALUE (args);

  if (TREE_CODE (arg) != STRING_CST)
    {
      *no_add_attrs = true;
      return NULL_TREE;
    }

  field = TREE_STRING_POINTER (arg);

  return NULL_TREE;
}

static const struct attribute_spec
wasm32_attribute_table[] =
{
  /* { name, min_len, max_len, decl_req, type_req, fn_type_req, affects_type_identity, handler,
       affects_type_identity } */
  /* stackcall, no regparms but argument count on the stack */
  { "stackcall", 0, 0, false, true, true, true, wasm32_handle_cconv_attribute, NULL },
  { "rawcall", 0, 0, false, true, true, true, wasm32_handle_cconv_attribute, NULL },
  { "regparm", 1, 1, false, true, true, true, wasm32_handle_cconv_attribute, NULL },

  { "jsexport", 0, 2, false, false, false, false, wasm32_handle_jsexport_attribute, NULL },
  { "import", 1, 2, true, false, false, false, wasm32_handle_import_attribute, NULL },
  { "export", 0, 1, true, false, false, false, wasm32_handle_export_attribute, NULL},
  { NULL, 0, 0, false, false, false, false, NULL, NULL }
};

bool
wasm32_print_operand_address (FILE *stream, rtx x)
{
  switch (GET_CODE (x))
    {
    case MEM:
      {
	rtx addr = XEXP (x, 0);

	wasm32_print_operand (stream, addr, 0);

	return true;
      }
    default:
      print_rtl(stream, x);
    }

  return true;
}

bool
wasm32_print_op (FILE *stream, rtx x);

bool wasm32_print_assignment (FILE *stream, rtx x);

void
wasm32_print_label (FILE *stream, rtx x)
{
  switch (GET_CODE (x))
    {
    case SYMBOL_REF:
      {
	const char *name = XSTR (x, 0);
	if (!SYMBOL_REF_FUNCTION_P (x))
	  {
	    assemble_name (stream, name);
	  }
	else if (in_section->common.flags & SECTION_CODE)
	  {
	    assemble_name (stream, name);
	  }
	else
	  {
	    asm_fprintf (stream, "BROKEN");
	    asm_fprintf (stream, "i64.const ");
	    asm_fprintf (stream, "%s", name + (name[0] == '*'));
	    asm_fprintf (stream, "\n\t");
	  }
	break;
      }
    case LABEL_REF:
      {
	char buf[256];
	x = label_ref_label (x);
	ASM_GENERATE_INTERNAL_LABEL (buf, "L", CODE_LABEL_NUMBER (x));
	ASM_OUTPUT_LABEL_REF (stream, buf);
	break;
      }
    case CONST_INT:
      {
	/* for pr21840.c */
	asm_fprintf (stream, "%ld", (long) XWINT (x, 0));
	break;
      }
    default:
      gcc_unreachable ();
    }
}

void
wasm32_print_label_plt (FILE *stream, rtx x)
{
  switch (GET_CODE (x))
    {
    case SYMBOL_REF:
	{
	  const char *name = XSTR (x, 0);
	  if (!SYMBOL_REF_FUNCTION_P (x))
	    {
	      assemble_name (stream, name);
	    }
	  else if (in_section->common.flags & SECTION_CODE)
	    {
	      assemble_name (stream, name);
	    }
	  else
	    {
	      asm_fprintf (stream, "BROKEN");
	      asm_fprintf (stream, "i64.const ");
	      asm_fprintf (stream, "%s", name + (name[0] == '*'));
	      asm_fprintf (stream, "\n\t");
	    }
	  if (SYMBOL_REF_FUNCTION_P (x) && !SYMBOL_REF_LOCAL_P (x))
	    {
	      fputs("@plt", stream);
	    }
	  break;
	}
    case LABEL_REF:
      {
	char buf[256];
	x = label_ref_label (x);
	ASM_GENERATE_INTERNAL_LABEL (buf, "L", CODE_LABEL_NUMBER (x));
	ASM_OUTPUT_LABEL_REF (stream, buf);
	break;
      }
    default:
      gcc_unreachable ();
    }
}

const char *
wasm32_mode (machine_mode mode)
{
  switch (mode)
    {
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
      return "XXX"; /* why is this happening? */
    }
}

const char *
wasm32_load (machine_mode outmode, machine_mode inmode, bool sign)
{
  switch (outmode)
    {
    case SImode:
      switch (inmode)
	{
	case SImode:
	  return "i32.load a=2 0";
	case HImode:
	  return sign ? "i32.load16_s a=1 0" : "i32.load16_u a=1 0";
	case QImode:
	  return sign ? "i32.load8_s a=0 0" : "i32.load8_u a=0 0";
	default: ;
	}
      gcc_unreachable ();
    case DImode:
      gcc_unreachable ();
    case SFmode:
      switch (inmode)
	{
	case SFmode:
	  return "f32.load a=2 0";
	default: ;
	}
      gcc_unreachable ();
    case DFmode:
      switch (inmode)
	{
	case DFmode:
	  return "f64.load a=3 0";
	default: ;
	}
      gcc_unreachable ();
    default: ;
    }
  gcc_unreachable ();
}

const char *
wasm32_store (machine_mode outmode, machine_mode inmode)
{
  switch (outmode)
    {
    case SImode:
      switch (inmode)
	{
	case SImode:
	  return "i32.store a=2 0";
	case HImode:
	  return "i32.store16 a=1 0";
	case QImode:
	  return "i32.store8 a=0 0";
	default: ;
	}
      gcc_unreachable ();
    case DImode:
      gcc_unreachable ();
    case SFmode:
      switch (inmode)
	{
	case SFmode:
	  return "f32.store a=2 0";
	default: ;
	}
      gcc_unreachable ();
    case DFmode:
      switch (inmode)
	{
	case DFmode:
	  return "f64.store a=3 0";
	default: ;
	}
      gcc_unreachable ();
    default: ;
    }
  gcc_unreachable ();
}

#include <print-tree.h>

bool
wasm32_print_operation (FILE *stream, rtx x, bool want_lval,
			bool lval_l = false)
{
  switch (GET_CODE (x))
    {
    case PC:
      {
	asm_fprintf (stream, "pc");
	break;
      }
    case REG: {
      if (want_lval && lval_l)
	{
	  if (REGNO (x) == RV_REG)
	    asm_fprintf (stream, "i32.const 8288");
	  else if (REGNO (x) >= A0_REG && REGNO (x) <= A3_REG)
	    asm_fprintf (stream, "i32.const %d",
			 8296 + 8*(REGNO (x)-A0_REG));
	  else
	    return false;

	  return true;
	}
      else if (want_lval)
	{
	  if (REGNO (x) == RV_REG)
	    asm_fprintf (stream, "i32.store a=2 0");
	  else if (REGNO (x) >= A0_REG && REGNO (x) <= A3_REG)
	    asm_fprintf (stream, "i32.store a=2 0");
	  else
	    asm_fprintf (stream, "local.set $%s", reg_names[REGNO (x)]);
	}
      else
	{
	  if (REGNO (x) == RV_REG)
	    asm_fprintf (stream, "i32.const 8288\n\ti32.load a=2 0");
	  else if (REGNO (x) >= A0_REG && REGNO (x) <= A3_REG)
	    asm_fprintf (stream, "i32.const %d\n\ti32.load a=2 0",
			 8296 + 8*(REGNO (x)-A0_REG));
	  else
	    asm_fprintf (stream, "local.get $%s", reg_names[REGNO (x)]);
	}
      break;
    }
    case ZERO_EXTEND:
      {
	rtx mem = XEXP (x, 0);
	if (GET_CODE (mem) == MEM)
	  {
	    rtx addr = XEXP (mem, 0);
	    if (want_lval)
	      gcc_unreachable ();

	    wasm32_print_operation (stream, addr, false);
	    asm_fprintf (stream, "\n\t%s\n\t", wasm32_load (GET_MODE (x), GET_MODE (mem), false));
	  }
	else if (GET_CODE (mem) == SUBREG)
	  {
	    rtx reg = XEXP (mem, 0);

	    wasm32_print_operation (stream, reg, false);
	    asm_fprintf (stream, "\n\ti32.const %d\n\t", GET_MODE (mem) == HImode ? 65535 : 255);
	    asm_fprintf (stream, "i32.and");
	  }
	else if (GET_CODE (mem) == REG)
	  {
	    wasm32_print_operation (stream, mem, false);
	    asm_fprintf (stream, "\n\ti32.const %d\n\t", GET_MODE (mem) == HImode ? 65535 : 255);
	    asm_fprintf (stream, "i32.and");
	  }
	else
	  {
	    print_rtl (stderr, x);
	    gcc_unreachable ();
	  }
	break;
      }
    case MEM:
      {
	rtx addr = XEXP (x, 0);

	if (want_lval && lval_l)
	  {
	    wasm32_print_operation (stream, addr, false);
	  }
	else if (want_lval)
	  {
	    machine_mode outmode = GET_MODE (x);
	    if (outmode == QImode || outmode == HImode || outmode == SImode)
	      outmode = SImode;
	    asm_fprintf (stream, "%s", wasm32_store (outmode, GET_MODE (x)));
	  }
	else
	  {
	    wasm32_print_operation (stream, addr, false);
	    machine_mode outmode = GET_MODE (x);
	    if (outmode == QImode || outmode == HImode || outmode == SImode)
	      outmode = SImode;
	    asm_fprintf (stream, "\n\t%s", wasm32_load (outmode, GET_MODE (x), true));
	  }

	break;
      }
    case CONST_INT:
      {
	asm_fprintf (stream, "i32.const %ld", (long) XWINT (x, 0));
	break;
      }
    case CONST_DOUBLE:
      {
	REAL_VALUE_TYPE r;
	char buf[512];
	long l[2];

	r = *CONST_DOUBLE_REAL_VALUE (x);
	REAL_VALUE_TO_TARGET_DOUBLE (r, l);

	real_to_decimal_for_mode (buf, &r, 510, 0, 1, DFmode);

	if (GET_MODE (x) == DFmode)
	  {
	    if (strcmp(buf, "+Inf") == 0)
	      asm_fprintf (stream, "f64.const 1.0\n\tf64.const 0.0\n\tf64.div");
	    else if (strcmp (buf, "-Inf") == 0)
	      asm_fprintf (stream, "f64.const -1.0\n\tf64.const 0.0\n\tf64.div");
	    else if (strcmp (buf, "+QNaN") == 0
		     || strcmp (buf, "-SNaN") == 0)
	      asm_fprintf (stream, "f64.const 0.0\n\tf64.const 0.0\n\tf64.div");
	    else if (strcmp (buf, "+SNaN") == 0
		     || strcmp (buf, "-QNaN") == 0)
	      asm_fprintf (stream, "f64.const -0.0\n\tf64.const 0.0\n\tf64.div");
	    else if (buf[0] == '+')
	      asm_fprintf (stream, "f64.const %s", buf+1);
	    else
	      asm_fprintf (stream, "f64.const %s", buf);
	  }
	else
	  {
	    if (strcmp(buf, "+Inf") == 0)
	      asm_fprintf (stream, "f32.const 1.0\n\tf32.const 0.0\n\tf32.div");
	    else if (strcmp (buf, "-Inf") == 0)
	      asm_fprintf (stream, "f32.const -1.0\n\tf32.const 0.0\n\tf32.div");
	    else if (strcmp (buf, "+QNaN") == 0
		     || strcmp (buf, "-SNaN") == 0)
	      asm_fprintf (stream, "f32.const 0.0\n\tf32.const 0.0\n\tf32.div");
	    else if (strcmp (buf, "+SNaN") == 0
		     || strcmp (buf, "-QNaN") == 0)
	      asm_fprintf (stream, "f32.const -0.0\n\tf32.const 0.0\n\tf32.div");
	    else if (buf[0] == '+')
	      asm_fprintf (stream, "f32.const %s", buf+1);
	    else
	      asm_fprintf (stream, "f32.const %s", buf);
	  }
	break;
      }
  case SYMBOL_REF:
    {
      const char *name = XSTR (x, 0);
      tree decl = XTREE (x, 1);
      tree attrs;
      tree import_attr;

      if (decl && TREE_CODE (decl) == VAR_DECL
	  && (attrs = DECL_ATTRIBUTES (decl))
	  && (import_attr = lookup_attribute ("import", attrs)))
	{
	  asm_fprintf (stream, "global.get __wasm_import_global_");
	  assemble_name (stream, name);
	}
      else if (decl && TREE_CODE (decl) == FUNCTION_DECL
	  && (attrs = DECL_ATTRIBUTES (decl))
	  && (import_attr = lookup_attribute ("import", attrs)))
	{
	  asm_fprintf (stream, "i32.const ");
	  assemble_name (stream, name);
	}
      else if (!SYMBOL_REF_FUNCTION_P (x))
	{
	  if (flag_pic)
	    {
	      asm_fprintf (stream, "global.get $got\n");
	      asm_fprintf (stream, "\ti32.const ");
	      assemble_name (stream, name);
	      asm_fprintf (stream, "@got\n");
	      asm_fprintf (stream, "\ti32.add\n\t");
	      asm_fprintf (stream, "i32.load a=2 0");
	    }
	  else
	    {
	      asm_fprintf (stream, "i32.const ");
	      assemble_name (stream, name);
	    }
	}
      else if (in_section->common.flags & SECTION_CODE)
	{
	  if (flag_pic || !SYMBOL_REF_LOCAL_P (x))
	    {
	      asm_fprintf (stream, "global.get $got\n");
	      asm_fprintf (stream, "\ti32.const ");
	      assemble_name (stream, name);
	      asm_fprintf (stream, "@gotcode\n");
	      asm_fprintf (stream, "\ti32.add\n\t");
	      asm_fprintf (stream, "i32.load a=2 0");
	    }
	  else
	    {
	      asm_fprintf (stream, "i32.const ");
	      assemble_name (stream, name);
	    }
	}
      else
	{
	  asm_fprintf (stream, "BROKEN");
	  asm_fprintf (stream, "i64.const ");
	  asm_fprintf (stream, "%s", name + (name[0] == '*'));
	}
      break;
    }
    case LABEL_REF:
      {
	char buf[256];
	x = label_ref_label (x);
	asm_fprintf (stream, "i32.const ");
	ASM_GENERATE_INTERNAL_LABEL (buf, "L", CODE_LABEL_NUMBER (x));
	ASM_OUTPUT_LABEL_REF (stream, buf);
	break;
      }
    case SET:
      {
	return wasm32_print_assignment (stream, x);
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
    case XOR:
    case EQ:
    case NE:
    case GT:
    case LT:
    case GE:
    case LE:
    case GTU:
    case LTU:
    case GEU:
    case LEU:
    case FLOAT:
    case FLOAT_TRUNCATE:
    case FLOAT_EXTEND:
    case FIX:
    case UNSIGNED_FIX:
    case NEG:
    case NOT:
    case CLZ:
    case CTZ:
    case POPCOUNT:
    return wasm32_print_op (stream, x);
    case CONST:
      {
	wasm32_print_operation (stream, XEXP (x, 0), false);
	break;
      }
    default:
      asm_fprintf (stream, "BROKEN (unknown code:746): ");
      print_rtl (stream, x);
      return true;
    }

  return true;
}

static unsigned wasm32_function_regsize(tree decl ATTRIBUTE_UNUSED);

static const char *wasm32_function_name;

bool
wasm32_print_operand(FILE *stream, rtx x, int code)
{
  //print_rtl (stderr, x);
  if (code == 'O' || code == 0)
    {
      if (wasm32_print_operation (stream, x, false) == false)
	asm_fprintf (stream, "\n\t");

      return true;
    }
  else if (code == 'R')
    {
      return wasm32_print_operation (stream, x, true, false);
    }
  else if (code == 'S')
    {
      return wasm32_print_operation (stream, x, true, true);
    }
  else if (code == '/')
    {
      asm_fprintf (stream, "%d", wasm32_function_regsize (NULL_TREE));
      return true;
    }
  else if (code == '@')
    {
      assemble_name (stream, wasm32_function_name);
      return true;
    }
  else if (code == 'L')
    {
      wasm32_print_label (stream, x);
      return true;
    }
  else if (code == 'P')
    {
      wasm32_print_label_plt (stream, x);
      return true;
    }

  asm_fprintf (stream, "BROKEN: should be using wasm32_print_operation");
  print_rtl (stream, x);

  return true;
}

bool wasm32_print_operand_punct_valid_p(int code)
{
  switch (code)
    {
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
    case '@':
      return true;
    default:
      return false;
    }
}

struct wasm32_operator
{
  int code;
  machine_mode inmode;
  machine_mode outmode;

  const char *prefix;
  const char *str;

  int nargs;
};

struct wasm32_operator
wasm32_operators[] = {
  {
    PLUS, SImode, SImode,
    NULL, "i32.add",
    2
  },
  {
    MINUS, SImode, SImode,
    NULL, "i32.sub",
    2
  },
  {
    MULT, SImode, SImode,
    NULL, "i32.mul",
    2
  },
  {
    DIV, SImode, SImode,
    NULL, "i32.div_s",
    2
  },
  {
    MOD, SImode, SImode,
    NULL, "i32.rem_s",
    2
  },
  {
    UDIV, SImode, SImode,
    NULL, "i32.div_u",
    2
  },
  {
    UMOD, SImode, SImode,
    NULL, "i32.rem_u",
    2
  },
  {
    XOR, SImode, SImode,
    NULL, "i32.xor",
    2
  },
  {
    IOR, SImode, SImode,
    NULL, "i32.or",
    2
  },
  {
    AND, SImode, SImode,
    NULL, "i32.and",
    2
  },
  {
    ASHIFT, SImode, SImode,
    NULL, "i32.shl",
    2
  },
  {
    ASHIFTRT, SImode, SImode,
    NULL, "i32.shr_s",
    2
  },
  {
    LSHIFTRT, SImode, SImode,
    NULL, "i32.shr_u",
    2
  },
  {
    EQ, SImode, VOIDmode,
    NULL, "i32.eq",
    2
  },
  {
    NE, SImode, VOIDmode,
    NULL, "i32.ne",
    2
  },
  {
    LT, SImode, VOIDmode,
    NULL, "i32.lt_s",
    2
  },
  {
    LE, SImode, VOIDmode,
    NULL, "i32.le_s",
    2
  },
  {
    GT, SImode, VOIDmode,
    NULL, "i32.gt_s",
    2
  },
  {
    GE, SImode, VOIDmode,
    NULL, "i32.ge_s",
    2
  },
  {
    LTU, SImode, VOIDmode,
    NULL, "i32.lt_u",
    2
  },
  {
    LEU, SImode, VOIDmode,
    NULL, "i32.le_u",
    2
  },
  {
    GTU, SImode, VOIDmode,
    NULL, "i32.gt_u",
    2
  },
  {
    GEU, SImode, VOIDmode,
    NULL, "i32.ge_u",
    2
  },
  {
    EQ, SImode, SImode,
    NULL, "i32.eq",
    2
  },
  {
    NE, SImode, SImode,
    NULL, "i32.ne",
    2
  },
  {
    LT, SImode, SImode,
    NULL, "i32.lt_s",
    2
  },
  {
    LE, SImode, SImode,
    NULL, "i32.le_s",
    2
  },
  {
    GT, SImode, SImode,
    NULL, "i32.gt_s",
    2
  },
  {
    GE, SImode, SImode,
    NULL, "i32.ge_s",
    2
  },
  {
    LTU, SImode, SImode,
    NULL, "i32.lt_u",
    2
  },
  {
    LEU, SImode, SImode,
    NULL, "i32.le_u",
    2
  },
  {
    GTU, SImode, SImode,
    NULL, "i32.gt_u",
    2
  },
  {
    GEU, SImode, SImode,
    NULL, "i32.ge_u",
    2
  },
  {
    PLUS, DFmode, DFmode,
    NULL, "f64.add",
    2
  },
  {
    MINUS, DFmode, DFmode,
    NULL, "f64.sub",
    2
  },
  {
    MULT, DFmode, DFmode,
    NULL, "f64.mul",
    2
  },
  {
    DIV, DFmode, DFmode,
    NULL, "f64.div",
    2
  },
  {
    NEG, DFmode, DFmode,
    NULL, "f64.neg",
    1
  },
  {
    EQ, DFmode, VOIDmode,
    NULL, "f64.eq",
    2
  },
  {
    NE, DFmode, VOIDmode,
    NULL, "f64.ne",
    2
  },
  {
    LT, DFmode, VOIDmode,
    NULL, "f64.lt",
    2
  },
  {
    LE, DFmode, VOIDmode,
    NULL, "f64.le",
    2
  },
  {
    GT, DFmode, VOIDmode,
    NULL, "f64.gt",
    2
  },
  {
    GE, DFmode, VOIDmode,
    NULL, "f64.ge",
    2
  },
#if 0
  {
    NEG, SImode, SImode,
    "i32.const 0", "i32.sub",
    1
  },
#endif
  {
    CLZ, SImode, SImode,
    NULL, "i32.clz",
    1
  },
  {
    CTZ, SImode, SImode,
    NULL, "i32.ctz",
    1
  },
  {
    POPCOUNT, SImode, SImode,
    NULL, "i32.popcount",
    1
  },
  {
    NOT, SImode, SImode,
    "i32.const -1", "i32.xor",
    1
  },
  {
    FIX, SFmode, SImode,
    NULL, "i32.trunc_sat_f32_s",
    1
  },
  {
    FIX, DFmode, SImode,
    NULL, "i32.trunc_sat_f64_s",
    1
  },
  {
    UNSIGNED_FIX, DFmode, SImode,
    NULL, "i32.trunc_f64_u",
    1
  },
  {
    FLOAT, SImode, DFmode,
    NULL, "f64.convert_i32_s",
    1
  },
  {
    FLOAT_EXTEND, SFmode, DFmode,
    NULL, "f64.promote_f32",
    1
  },
  {
    FLOAT_TRUNCATE, DFmode, SFmode,
    NULL, "f32.demote_f64",
    1
  },
  {
    0, SImode, SImode,
    NULL, NULL,
    0
  }
};

bool
modes_compatible (rtx x, machine_mode y)
{
  return (GET_MODE (x) == y) || CONSTANT_P (x) || CONST_DOUBLE_P (x);
}

bool
wasm32_print_op (FILE *stream, rtx x)
{
  struct wasm32_operator *oper;
  machine_mode outmode = GET_MODE (x);

  rtx a = XEXP (x, 0);
  rtx b;

  for (oper = wasm32_operators; oper->code; oper++)
    {
      if (oper->code == GET_CODE (x)
	  && oper->outmode == outmode
	  && modes_compatible(a, oper->inmode)
	  && (oper->nargs == 1 ||
	      modes_compatible (XEXP (x,1), oper->inmode)))
	break;
    }

  if (oper->code == 0)
    {
      debug_rtx (x);
      if (GET_MODE (x) == SImode) {
	if (GET_CODE (x) == SIGN_EXTEND || GET_CODE (x) == ZERO_EXTEND)
	  return false;

	if (wasm32_print_op (stream, gen_rtx_ZERO_EXTEND (SImode, x)))
	  return true;
      }

      return false;
    }

  if (oper->prefix)
    asm_fprintf (stream, "%s\n\t", oper->prefix);
  wasm32_print_operation(stream, a, false);
  asm_fprintf (stream, "\n\t");
  if (oper->nargs == 2)
    {
      b = XEXP (x, 1);
      wasm32_print_operation(stream, b, false);
      asm_fprintf (stream, "\n\t");
    }
  asm_fprintf (stream, "%s", oper->str);

  return true;
}

bool
wasm32_print_assignment (FILE *stream, rtx x)
{
  if (wasm32_print_operation (stream, XEXP (x, 0), true, true))
    asm_fprintf (stream, "\n\t");
  wasm32_print_operation (stream, XEXP (x, 1), false);
  asm_fprintf (stream, "\n\t");
  wasm32_print_operation (stream, XEXP (x, 0), true, false);

  return true;
}

rtx
wasm32_function_value (const_tree ret_type, const_tree fn_decl ATTRIBUTE_UNUSED,
		       bool outgoing ATTRIBUTE_UNUSED)
{
  if (!use_rv_register)
    return NULL_RTX;

  /* XXX does this work for DImode? */
  switch (TYPE_MODE (ret_type))
    {
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

rtx
wasm32_struct_value_rtx (tree fndecl ATTRIBUTE_UNUSED,
			 int incoming ATTRIBUTE_UNUSED)
{
  return NULL_RTX;
}

bool
wasm32_return_in_memory (const_tree type,
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

bool
wasm32_omit_struct_return_reg (void)
{
  if (use_rv_register)
    return false;
  else
    return true;
}

rtx
wasm32_get_drap_rtx (void)
{
  return NULL_RTX;
}

rtx
wasm32_libcall_value (machine_mode mode, const_rtx fun ATTRIBUTE_UNUSED)
{
  if (!use_rv_register)
    return NULL_RTX;

  switch (mode)
    {
    case SFmode:
    case DFmode:
    default:
      return NULL_RTX;
    case QImode:
    case HImode:
    case SImode:
      return gen_rtx_REG (mode, RV_REG);
    }
}

bool
wasm32_function_value_regno_p (unsigned int regno)
{
  if (use_rv_register)
    return regno == RV_REG;
  else
    return false;
}

bool
wasm32_promote_prototypes (const_tree fntype ATTRIBUTE_UNUSED)
{
  return true;
}

void
wasm32_promote_mode (machine_mode *modep,
		     int *punsignedp ATTRIBUTE_UNUSED,
		     const_tree type ATTRIBUTE_UNUSED)
{
  switch (*modep)
    {
    case QImode:
    case HImode:
    case SImode:
      *modep = SImode;
      break;
    case DFmode:
      *modep = DFmode;
      break;
    default:
      break;
    }
}

machine_mode
wasm32_promote_function_mode (const_tree type ATTRIBUTE_UNUSED,
			      machine_mode mode,
			      int *punsignedp ATTRIBUTE_UNUSED,
			      const_tree funtype ATTRIBUTE_UNUSED,
			      int for_return)
{
  if (for_return == 1)
    {
    switch (mode)
      {
      case VOIDmode:
      case QImode:
      case HImode:
      case SImode:
	return SImode;
      case DFmode:
	return DFmode;
	break;
      default:
	return mode;
      }
    }
  else if (for_return == 2)
    {
      switch (mode)
	{
	case QImode:
	case HImode:
	case SImode:
	  return SImode;
	case DFmode:
	  return DFmode;
	  break;
	default:
	  return mode;
	}
    }

  return mode;
}

unsigned
wasm32_reg_parm_stack_space (const_tree fndecl ATTRIBUTE_UNUSED)
{
  return 0;
}

unsigned
wasm32_incoming_reg_parm_stack_space (const_tree fndecl ATTRIBUTE_UNUSED)
{
  return 0;
}

unsigned
wasm32_outgoing_reg_parm_stack_space (const_tree fndecl ATTRIBUTE_UNUSED)
{
  return 0;
}

void
wasm32_file_start (void)
{
  fputs ("#NO_APP\n", asm_out_file);
  fputs ("\t.include \"wasm32-macros.s\"\n", asm_out_file);
  default_file_start ();
}

void
wasm32_file_end (void)
{
  fputs ("\twasm_fini\n", asm_out_file);
}

static unsigned
wasm32_function_regmask (tree decl ATTRIBUTE_UNUSED)
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

static unsigned
wasm32_function_regsize (tree decl ATTRIBUTE_UNUSED)
{
  unsigned mask = wasm32_function_regmask (decl);
  unsigned size = 0;

  size += 8;
  size += 8;
  size += 8;
  size += 8;
  size += 8;
  size += 8;

  int i;
  for (i = 4 /* A0_REG */; i <= 31 /* F7_REG */;  i++)
    {
      if (mask & (1<<(i-4)))
	{
	  if (wasm32_regno_reg_class (i) == FLOAT_REGS)
	    {
	      size += 8;
	    }
	  else
	    {
	      size += 8;
	    }
	}
    }

  return size;
}

static unsigned
wasm32_function_regstore (FILE *stream,
			  tree decl ATTRIBUTE_UNUSED,
			  const char *cooked_name)
{
  unsigned mask = wasm32_function_regmask (decl);
  unsigned size = 0;
  unsigned total_size = wasm32_function_regsize (decl);

  asm_fprintf (stream, "\tnextcase\n");
  asm_fprintf (stream, "\tend\n");
  asm_fprintf (stream, "\ti32.const 3\n\tlocal.get $rp\n\ti32.and\n\ti32.const 1\n\ti32.ne\n\tif[]\n\tlocal.get $rp\n\treturn\n\tend\n");
  asm_fprintf (stream, "\tlocal.get $sp\n\ti32.const -16\n\ti32.add\n\tlocal.get $fp\n\ti32.store a=2 0\n");
  asm_fprintf (stream, "\tlocal.get $sp\n\ti32.const -8\n\ti32.add\n\tglobal.get $gpo\n\tlocal.get $dpc\n\ti32.const __wasm_pc_base_%s\n\ti32.add\n\ti32.add\n\ti32.store a=2 0\n", cooked_name);
  asm_fprintf (stream, "\ti32.const %d\n\tlocal.get $fp\n\ti32.add\n\tlocal.get $fp\n\ti32.const %d\n\ti32.add\n\ti32.store a=2 0\n", size, total_size);
  size += 8;

  asm_fprintf (stream, "\ti32.const %d\n\tlocal.get $fp\n\ti32.add\n\tglobal.get $plt\n\ti32.const %s\n\ti32.add\n\ti32.store a=2 0\n", size, cooked_name);
  size += 8;

  asm_fprintf (stream, "\ti32.const %d\n\tlocal.get $fp\n\ti32.add\n\tlocal.get $dpc\n\ti32.store a=2 0\n", size);
  size += 8;

  asm_fprintf (stream, "\ti32.const %d\n\tlocal.get $fp\n\ti32.add\n\tglobal.get $gpo\n\tlocal.get $dpc\n\ti32.const __wasm_pc_base_%s\n\ti32.add\n\ti32.add\n\ti32.store a=2 0\n", size, cooked_name);
  size += 4;
  asm_fprintf (stream, "\ti32.const %d\n\tlocal.get $fp\n\ti32.add\n\tglobal.get $gpo\n\ti32.const __wasm_pc_base_%s\n\ti32.add\n\ti32.store a=2 0\n", size, cooked_name);
  size += 4;

  asm_fprintf (stream, "\ti32.const %d\n\tlocal.get $fp\n\ti32.add\n\tlocal.get $sp\n\ti32.store a=2 0\n", size);
  size += 8;

  asm_fprintf (stream, "\ti32.const %d\n\tlocal.get $fp\n\ti32.add\n\ti32.const %d\n\ti32.store a=2 0\n", size, mask);
  size += 8;

  int i;
  for(i = 4 /* R0_REG */; i <= 31 /* F7_REG */; i++)
    {
      if (mask & (1<<(i-4)))
	{
	  if (wasm32_regno_reg_class (i) == FLOAT_REGS)
	    {
	      asm_fprintf(stream, "\ti32.const %d\n\tlocal.get $fp\n\ti32.add\n\tlocal.get $%s\n\tf64.store a=3 0\n",
			  size, reg_names[i]);
	      size += 8;
	    }
	  else
	    {
	      if (i >= 8)
		asm_fprintf(stream, "\ti32.const %d\n\tlocal.get $fp\n\ti32.add\n\tlocal.get $%s\n\ti32.store a=2 0\n",
			    size, reg_names[i]);
	      size += 8;
	    }
	}
    }

  asm_fprintf (stream, "\tlocal.get $rp\n");
  asm_fprintf (stream, "\treturn\n\tend\n");
  return size;
}

static unsigned
wasm32_function_regload (FILE *stream,
			 tree decl ATTRIBUTE_UNUSED)
{
  unsigned mask = wasm32_function_regmask (decl);
  unsigned size = 0;
  unsigned total_size = wasm32_function_regsize (decl);

  size += 8;

  //asm_fprintf (stream, "\ti32.const %d\n\tlocal.get $rp\n\ti32.add\n\ti32.load a=2 0\n\tlocal.set $pc0\n", size);
  size += 8;

  asm_fprintf (stream, "\ti32.const %d\n\tlocal.get $rp\n\ti32.add\n\ti32.load a=2 0\n\tlocal.set $dpc\n", size);
  size += 8;

  //asm_fprintf (stream, "\ti32.const %d\n\tlocal.get $rp\n\ti32.add\n\ti32.load a=2 0\n\tlocal.set $rpc\n", size);
  size += 8;

  asm_fprintf (stream, "\ti32.const %d\n\tlocal.get $rp\n\ti32.add\n\ti32.load a=2 0\n\tlocal.set $sp\n", size);
  size += 8;

  size += 8;

  int i;
  for(i = 4 /* R0_REG */; i <= 31 /* F7_REG */; i++)
    {
      if (mask & (1<<(i-4)))
	{
	  if (wasm32_regno_reg_class (i) == FLOAT_REGS)
	    {
	      asm_fprintf (stream, "\ti32.const %d\n\tlocal.get $rp\n\ti32.add\n\tf64.load a=3 0\n\tlocal.set $%s\n", size, reg_names[i]);
	      size += 8;
	    }
	  else
	    {
	      if (i >= 8)
		asm_fprintf (stream, "\ti32.const %d\n\tlocal.get $rp\n\ti32.add\n\ti32.load a=2 0\n\tlocal.set $%s\n", size, reg_names[i]);
	      size += 8;
	    }
	}
    }

  asm_fprintf (stream, "\tlocal.get $rp\n\tlocal.set $fp\n");
  asm_fprintf (stream, "\tjump2\n");

  if (size != total_size)
    gcc_unreachable ();
  return size;
}

static void
save_stack_args (const char *sig)
{
  rtx *operands = NULL;
  bool stackret = (sig[1] == 'l'
		   || sig[1] == 'f'
		   || sig[1] == 'd');
  int stacksize = stackret ? 4 : 0;
  int local_index = 0;
  int sigindex;

  for (sigindex = 2; sig[sigindex] != 'E'; sigindex++, local_index++)
    {
      switch (sig[sigindex])
	{
	case 'i':
	case 'f':
	  stacksize += 4;
	  break;
	case 'l':
	case 'd':
	  stacksize += (stacksize & 4);
	  stacksize += 8;
	  break;
	}
    }

  stacksize += 7;
  stacksize &= -8;

  char *templ;
  output_asm_insn ("global.get __wasm_stack_pointer", operands);
  asprintf (&templ, "i32.const %d", -stacksize);
  output_asm_insn (templ, operands);
  free (templ);
  output_asm_insn ("i32.add", operands);
  output_asm_insn ("global.set __wasm_stack_pointer", operands);

  stacksize = stackret ? 4 : 0;
  local_index = 0;
  for (sigindex = 2; sig[sigindex] != 'E'; sigindex++, local_index++)
    {
      switch (sig[sigindex])
	{
	case 'i':
	  output_asm_insn ("global.get __wasm_stack_pointer", operands);
	  asprintf (&templ, "i32.const %d", stacksize);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("i32.add", operands);
	  asprintf (&templ, "local.get %d", local_index);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("i32.store a=2 0", operands);
	  stacksize += 4;
	  break;
	case 'f':
	  output_asm_insn ("global.get __wasm_stack_pointer", operands);
	  asprintf (&templ, "i32.const %d", stacksize);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("i32.add", operands);
	  asprintf (&templ, "local.get %d", local_index);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("f32.store a=2 0", operands);
	  stacksize += 4;
	  break;
	case 'd':
	  stacksize += stacksize & 4;
	  output_asm_insn ("global.get __wasm_stack_pointer", operands);
	  asprintf (&templ, "i32.const %d", stacksize);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("i32.add", operands);
	  asprintf (&templ, "local.get %d", local_index);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("f64.store a=2 0", operands);
	  stacksize += 8;
	  break;
	case 'l':
	  stacksize += stacksize & 4;
	  output_asm_insn ("global.get __wasm_stack_pointer", operands);
	  asprintf (&templ, "i32.const %d", stacksize);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("i32.add", operands);
	  asprintf (&templ, "local.get %d", local_index);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("i64.store a=2 0", operands);
	  stacksize += 8;
	  break;
	}
    }
}

static void
load_stack_rets (const char *sig)
{
  rtx *operands = NULL;
  bool stackret = (sig[1] == 'l'
		   || sig[1] == 'f'
		   || sig[1] == 'd');
  int stacksize = stackret ? 4 : 0;
  int local_index = 0;
  int sigindex;

  for (sigindex = 2; sig[sigindex] != 'E'; sigindex++, local_index++)
    {
      switch (sig[sigindex])
	{
	case 'i':
	case 'f':
	  stacksize += 4;
	  break;
	case 'l':
	case 'd':
	  stacksize += (stacksize & 4);
	  stacksize += 8;
	  break;
	}
    }

  char *templ;
  output_asm_insn ("global.get __wasm_stack_pointer", operands);
  asprintf (&templ, "i32.const %d", -stacksize);
  output_asm_insn (templ, operands);
  free (templ);
  output_asm_insn ("i32.add", operands);
  output_asm_insn ("global.set __wasm_stack_pointer", operands);

  stacksize = stackret ? 4 : 0;
  local_index = 0;
  for (sigindex = 2; sig[sigindex] != 'E'; sigindex++, local_index++)
    {
      switch (sig[sigindex])
	{
	case 'i':
	  output_asm_insn ("global.get __wasm_stack_pointer", operands);
	  asprintf (&templ, "i32.const %d", stacksize);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("i32.add", operands);
	  asprintf (&templ, "local.get %d", local_index);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("i32.store a=2 0", operands);
	  stacksize += 4;
	  break;
	case 'f':
	  output_asm_insn ("global.get __wasm_stack_pointer", operands);
	  asprintf (&templ, "i32.const %d", stacksize);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("i32.add", operands);
	  asprintf (&templ, "local.get %d", local_index);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("f32.store a=2 0", operands);
	  stacksize += 4;
	  break;
	case 'd':
	  stacksize += stacksize & 4;
	  output_asm_insn ("global.get __wasm_stack_pointer", operands);
	  asprintf (&templ, "i32.const %d", stacksize);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("i32.add", operands);
	  asprintf (&templ, "local.get %d", local_index);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("f64.store a=2 0", operands);
	  stacksize += 8;
	  break;
	case 'l':
	  stacksize += stacksize & 4;
	  output_asm_insn ("global.get __wasm_stack_pointer", operands);
	  asprintf (&templ, "i32.const %d", stacksize);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("i32.add", operands);
	  asprintf (&templ, "local.get %d", local_index);
	  output_asm_insn (templ, operands);
	  free (templ);
	  output_asm_insn ("i64.store a=2 0", operands);
	  stacksize += 8;
	  break;
	}
    }
}

void
wasm32_start_function (FILE *f, const char *name, tree decl)
{
  char *cooked_name = (char *)alloca (strlen(name)+1);
  const char *p = name;
  char *q = cooked_name;

  for (p = name; *p; p++)
      *q++ = *p;

  *q = 0;

  while (cooked_name[0] == '*')
    cooked_name++;

  wasm32_function_name = ggc_strdup (cooked_name);

  tree type = TREE_TYPE (decl);
  if (lookup_attribute ("rawcall", TYPE_ATTRIBUTES (type)))
    {
      const char *sig = wasm32_signature_string (type);
      wasm32_function_name = concat ("__wasm_wrapped_", wasm32_function_name,
				     NULL);

      asm_fprintf (f, "\tdefun %s, %s, 1\n", cooked_name, sig);

      save_stack_args (sig);
      switch (sig[1])
	{
	case 'v':
	case 'i':
	  break;
	case 'l':
	case 'f':
	case 'd':
	  asm_fprintf (f, "\tglobal.get __wasm_stack_pointer\n");
	  break;
	}
      asm_fprintf (f, "\ti32.const -1\n");
      asm_fprintf (f, "\tglobal.get __wasm_stack_pointer\n");
      asm_fprintf (f, "\ti32.const 0\n");
      asm_fprintf (f, "\ti32.const 0\n");
      asm_fprintf (f, "\ti32.const 0\n");
      asm_fprintf (f, "\ti32.const 0\n");
      asm_fprintf (f, "\tcall %s\n", wasm32_function_name);
      asm_fprintf (f, "\tdrop\n");
      switch (sig[1])
	{
	case 'v':
	  break;
	case 'i':
	  asm_fprintf (f, "\ti32.const 8288\n");
	  asm_fprintf (f, "\ti32.load a=2 0\n");
	  break;
	case 'l':
	  asm_fprintf (f, "\ti32.load a=2 0\n");
	  asm_fprintf (f, "\ti64.load a=3 0\n");
	  break;
	case 'f':
	  asm_fprintf (f, "\ti32.load a=2 0\n");
	  asm_fprintf (f, "\tf32.load a=2 0\n");
	  break;
	case 'd':
	  asm_fprintf (f, "\ti32.load a=2 0\n");
	  asm_fprintf (f, "\tf64.load a=3 0\n");
	  break;
	}
      asm_fprintf (f, "\treturn\n");
      asm_fprintf (f, "\tend\n");
      asm_fprintf (f, "\tendefun %s, 1\n", cooked_name);

      cooked_name = (char *)wasm32_function_name;
    }

  asm_fprintf (f, "\tdefun %s, FiiiiiiiE\n", cooked_name);
}

void
wasm32_end_function (FILE *f, const char *name, tree decl ATTRIBUTE_UNUSED)
{
  char *cooked_name = (char *) alloca (strlen (name)+1);
  const char *p = name;
  char *q = cooked_name;

  for (p = name; *p; p++)
    *q++ = *p;

  *q = 0;

  while (cooked_name[0] == '*')
    cooked_name++;

  asm_fprintf (f, "\tnextcase\n");
  asm_fprintf (f, "\tlocal.get $sp\n\tlocal.set $rp\n");
  wasm32_function_regload (f, decl);
  wasm32_function_regstore (f, decl, cooked_name);

  asm_fprintf (f, "\tendefun ");
  assemble_name (f, wasm32_function_name);
  asm_fprintf (f, "\n");
}

void
wasm32_output_ascii (FILE *f, const void *ptr, size_t len)
{
  const unsigned char *bytes = (const unsigned char *)ptr;

  if (len) {
    size_t i;

    asm_fprintf(f, "\t.byte ");

    for (i=0; i<len; i++) {
      asm_fprintf(f, "%d%c", bytes[i], i == len-1 ? '\n' : ',');
    }
  }
}

void
wasm32_output_label (FILE *stream, const char *name)
{
  fprintf (stream, "\t");
  assemble_name (stream, name);
  fprintf (stream, ":\n");
}

void
wasm32_output_debug_label (FILE *stream, const char *prefix, int num)
{
  fprintf (stream, "\t.labeldef_debug .%s%d\n", prefix, num);
}

void
wasm32_output_internal_label (FILE *stream, const char *name)
{
  if (in_section && in_section->common.flags & SECTION_CODE)
    {
      fprintf (stream, "\t.labeldef_internal ");
      assemble_name (stream, name);
      fprintf (stream, "\n");
    }
  else
    {
      assemble_name (stream, name);
      fprintf (stream, ":\n");
    }
}

void
wasm32_output_labelref (FILE *stream, const char *name)
{
  fprintf (stream, "%s", name);
}

void
wasm32_output_label_ref (FILE *stream, const char *name)
{
  fprintf (stream, "%s", name);
}

void
wasm32_output_symbol_ref (FILE *stream, rtx x)
{
  const char *name = XSTR (x, 0);
  assemble_name (stream, name);
}

void
wasm32_asm_named_section (const char *name, unsigned int flags,
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
      if (flags & SECTION_CODE)
	fprintf (asm_out_file, "\t.pushsection .wasm.code.%%S,2*__wasm_counter+1,\"ax\"\n");
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

  fprintf (asm_out_file, "\t.section\t%s,\"%s\"",
	   name, flagchars);

  if (!(flags & SECTION_NOTYPE))
    {
      const char *type;
      const char *format;

      if (flags & SECTION_BSS)
	type = "nobits";
      else
	type = "progbits";

      format = ",@%s";
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
    fprintf (asm_out_file, "\t.pushsection .wasm.code.%%S,2*__wasm_counter+1,\"ax\"\n");
}

void
wasm32_output_aligned_decl_common (FILE *stream, tree decl ATTRIBUTE_UNUSED,
				   const char *name, size_t size, size_t align)
{
  fprintf (stream, "\t.comm ");
  assemble_name(stream, name);
  fprintf (stream, ", %d, %d", (int)size, (int)(align / BITS_PER_UNIT));
  fprintf (stream, "\n");
}

void
wasm32_output_aligned_decl_local (FILE *stream, tree decl, const char *name, size_t size, size_t align)
{
  fprintf (stream, "\t.local %s\n", name);
  wasm32_output_aligned_decl_common (stream, decl, name, size, align);
}

void
wasm32_output_local (FILE *stream, const char *name, size_t size ATTRIBUTE_UNUSED, size_t rounded ATTRIBUTE_UNUSED)
{
  fprintf (stream, "\t.local ");
  assemble_name (stream, name);
  fprintf (stream, "\n");
  fprintf (stream, "\t.comm ");
  assemble_name (stream, name);
  fprintf (stream, ", %d", (int)rounded);
  fprintf (stream, "\n");
}

void
wasm32_output_skip (FILE *stream, size_t bytes)
{
  size_t n;
  for (n = 0; n < bytes; n++) {
    fprintf (stream, "\t.byte 0\n");
  }
}

void
wasm32_output_aligned_bss (FILE *stream, const_tree tree ATTRIBUTE_UNUSED, const char *name,
			   unsigned HOST_WIDE_INT size,
			   unsigned HOST_WIDE_INT rounded ATTRIBUTE_UNUSED)
{
  fprintf (stream, ".aligned-bss %s %d %d\n", name,
	   (int) size, (int) rounded);
}

bool
wasm32_hard_regno_mode_ok (unsigned int regno, machine_mode mode)
{
  if (regno >= 24 && regno <= 31)
    return mode == DFmode;
  return mode == SImode;
}


#if 0
bool
wasm32_hard_regno_mode_ok (unsigned int regno, machine_mode mode)
{
  if (regno >= FIRST_PSEUDO_REGISTER)
    return false;

  if (regno >= 24 && regno <= 31)
    return FLOAT_MODE_P (mode);
  else
    return !FLOAT_MODE_P (mode) && mode != DImode;

  return true;
}
#endif

bool
wasm32_hard_regno_rename_ok (unsigned int from, unsigned int to)
{
  return wasm32_regno_reg_class (from) == wasm32_regno_reg_class (to);
}

bool
wasm32_modes_tieable_p (machine_mode mode1, machine_mode mode2)
{
  return (mode1 == mode2) || (!FLOAT_MODE_P (mode1) && !FLOAT_MODE_P (mode2));
}

bool
wasm32_regno_ok_for_index_p (unsigned int regno)
{
  if (regno >= FIRST_PSEUDO_REGISTER)
    return false;

  return wasm32_regno_reg_class (regno) == GENERAL_REGS;
}

bool
wasm32_regno_ok_for_base_p (unsigned int regno)
{
  if (regno >= FIRST_PSEUDO_REGISTER)
    return false;

  return wasm32_regno_reg_class (regno) == GENERAL_REGS;
}

/* Uh, I don't understand the documentation for this. Using anything
   but 4 breaks df_read_modify_subreg. */
int
wasm32_regmode_natural_size (machine_mode ATTRIBUTE_UNUSED mode)
{
  return 4;
}

int
wasm32_hard_regno_nregs (unsigned int regno, machine_mode mode)
{
  if (wasm32_regno_reg_class (regno) == FLOAT_REGS
      && mode == DFmode)
    return 1;
  else
    return ((GET_MODE_SIZE (mode) + UNITS_PER_WORD - 1).to_constant () /
	    UNITS_PER_WORD);
}

static int
wasm32_register_priority (int regno)
{
  switch (wasm32_regno_reg_class (regno)) {
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


void
wasm32_expand_builtin_va_start (tree valist, rtx nextarg)
{
  rtx va_r = expand_expr (valist, NULL_RTX, VOIDmode, EXPAND_WRITE);
  convert_move (va_r, gen_rtx_PLUS (SImode, nextarg, gen_rtx_CONST_INT (SImode, 0)), 0);
}

poly_int64
wasm32_return_pops_args (tree fundecl ATTRIBUTE_UNUSED,
			 tree funtype ATTRIBUTE_UNUSED,
			 poly_int64 size ATTRIBUTE_UNUSED)
{
  return 0;
}

int
wasm32_call_pops_args (CUMULATIVE_ARGS size ATTRIBUTE_UNUSED)
{
  return 0;
}

rtx
wasm32_dynamic_chain_address (rtx frameaddr)
{
  return gen_rtx_MEM (SImode, frameaddr);
}

static int last_regsize;

rtx
wasm32_incoming_return_addr_rtx (void)
{
  return
    gen_rtx_MEM (Pmode, plus_constant
		 (Pmode, gen_rtx_REG (Pmode, SP_REG),
		  8));
}

rtx
wasm32_return_addr_rtx (int count, rtx frameaddr ATTRIBUTE_UNUSED)
{
  if (count == 0)
    {
      crtl->calls_eh_return = 1;
      return
	gen_rtx_MEM (Pmode, plus_constant
		     (Pmode, gen_rtx_REG (Pmode, FP_REG),
		      272 + 8));
    }
  else
    return const0_rtx;
}

int
wasm32_first_parm_offset (const_tree fntype ATTRIBUTE_UNUSED)
{
  return 0;
}

static void
load_stack_args (const char *sig)
{
  rtx *operands = NULL;
  bool stackret = (sig[1] == 'l'
		   || sig[1] == 'f'
		   || sig[1] == 'd');
  int num_args = strlen (sig) - 3;

  int stackoff = stackret ? 4 : 0;
  int sigindex = 2;
  while (num_args)
    {
      char *templ;

      output_asm_insn ("local.get $sp", operands);
      if (sig[sigindex] == 'd' ||
	  sig[sigindex] == 'l')
	stackoff += (stackoff & 4);
      asprintf (&templ, "i32.const %d", stackoff);
      output_asm_insn (templ, operands);
      output_asm_insn ("i32.add", operands);
      if (sig[sigindex] == 'i')
	{
	  output_asm_insn ("i32.load a=2 0", operands);
	  stackoff += 4;
	}
      else if (sig[sigindex] == 'f')
	{
	  output_asm_insn ("f32.load a=2 0", operands);
	  stackoff += 4;
	}
      else if (sig[sigindex] == 'l')
	{
	  output_asm_insn ("i64.load a=3 0", operands);
	  stackoff += 8;
	}
      else if (sig[sigindex] == 'd')
	{
	  output_asm_insn ("f64.load a=3 0", operands);
	  stackoff += 8;
	}
      num_args--;
      sigindex++;
      free (templ);
    }
}

const char *
output_call (rtx *operands, bool immediate, bool value)
{
  const char *sig = XSTR (operands[1], 0);
  bool retval = sig[1] != 'v';
  bool stackret = (sig[1] == 'l'
		   || sig[1] == 'f'
		   || sig[1] == 'd');
  if (sig[0])
    {
      if (value)
	{
	  output_asm_insn ("i32.const 8288", operands);
	}
      else if (stackret)
	{
	  output_asm_insn ("local.get $sp", operands);
	  output_asm_insn ("i32.load a=2 0", operands);
	}

      load_stack_args (sig);

      output_asm_insn ("local.get $sp", operands);
      output_asm_insn ("i32.const -16", operands);
      output_asm_insn ("i32.add", operands);
      output_asm_insn ("global.set __wasm_stack_pointer", operands);

      if (immediate)
	{
	  tree decl;
	  tree attrs;

	  if (GET_CODE (operands[0]) == SYMBOL_REF
	      && (decl = XTREE (operands[0], 1))
	      && (attrs = DECL_ATTRIBUTES (decl))
	      && (lookup_attribute ("import", attrs)
		  || lookup_attribute ("noplt", attrs)))
	    {
	      char *templ;
	      asprintf (&templ, "call %%L0");
	      output_asm_insn (templ, operands);
	      free (templ);
	    }
	  else
	    {
	      char *templ;
	      asprintf (&templ, "call %%L0@plt{__sigchar_%s}", sig);
	      output_asm_insn (templ, operands);
	      free (templ);
	    }
	}
      else
	{
	  char *templ;
	  output_asm_insn ("%0", operands);
	  asprintf (&templ, "call_indirect __sigchar_%s 0", sig);
	  output_asm_insn (templ, operands);
	  free (templ);
	}

      if (value)
	{
	  if (sig[1] == 'i')
	    {
	      output_asm_insn ("i32.store a=2 0", operands);
	    }
	}
      else if (stackret)
	{
	  if (sig[1] == 'f')
	    {
	      output_asm_insn ("f32.store a=2 0", operands);
	    }
	  else if (sig[1] == 'l')
	    {
	      output_asm_insn ("i64.store a=3 0", operands);
	    }
	  else if (sig[1] == 'd')
	    {
	      output_asm_insn ("f64.store a=3 0", operands);
	    }
	}
      else if (retval)
	{
	  output_asm_insn ("drop", operands);
	}

      return "";
    }
  output_asm_insn ("i32.const -1", operands);
  output_asm_insn ("local.get $sp", operands);
  output_asm_insn ("local.get $r0", operands);
  output_asm_insn ("local.get $r1", operands);
  output_asm_insn ("i32.const 0", operands);
  output_asm_insn ("i32.const 0", operands);

  if (immediate)
    {
      const char *sig = "FiiiiiiiE";
      tree decl;
      tree attrs;

      if (GET_CODE (operands[0]) == SYMBOL_REF
	  && (decl = XTREE (operands[0], 1))
	  && (attrs = DECL_ATTRIBUTES (decl))
	  && (lookup_attribute ("import", attrs)
	      || lookup_attribute ("noplt", attrs)))
	{
	  char *templ;
	  asprintf (&templ, "call %%L0");
	  output_asm_insn (templ, operands);
	  free (templ);
	}
      else
	{
	  char *templ;
	  asprintf (&templ, "call %%L0@plt{__sigchar_%s}", sig);
	  output_asm_insn (templ, operands);
	  free (templ);
	}
    }
  else
    {
      output_asm_insn ("%0",
		       operands);
      output_asm_insn ("call_indirect __sigchar_FiiiiiiiE 0",
		       operands);
    }

  output_asm_insn ("local.tee $rp", operands);
  output_asm_insn ("i32.const 3", operands);
  output_asm_insn ("i32.and", operands);
  output_asm_insn ("if[]", operands);
  output_asm_insn (".dpc .LI%=", operands);
  output_asm_insn ("local.set $dpc", operands);
  output_asm_insn ("throw1", operands);
  output_asm_insn ("end", operands);

  output_asm_insn (".wasmtextlabeldpcdef .LI%=", operands);

  return "";
}

rtx
wasm32_expand_call (rtx retval, rtx address, rtx callarg1 ATTRIBUTE_UNUSED)
{
  int argcount;
  rtx use = NULL, call;
  rtx_insn *call_insn;
  rtx sp = gen_rtx_REG (SImode, SP_REG);
  tree funtype = cfun->machine->funtype;
  rtx sig = gen_rtx_CONST_STRING (VOIDmode, "");

  if (wasm32_is_rawcall (funtype))
    {
      sig = gen_rtx_CONST_STRING (VOIDmode, wasm32_signature_string (funtype));
    }

  argcount = wasm32_argument_count (funtype);

#if 0
  emit_insn (gen_rtx_UNSPEC_VOLATILE (VOIDmode,
				      gen_rtvec (1, gen_rtx_REG (SImode, R0_REG)),
				      UNSPECV_STACK_PUSH));
  emit_insn (gen_rtx_SET (gen_rtx_REG (SImode, R0_REG),
			  gen_rtx_UNSPEC_VOLATILE (SImode,
						   gen_rtvec (1, const0_rtx),
						   UNSPECV_STACK_POP)));
#endif

  if (wasm32_is_real_stackcall (funtype))
    {
      rtx loc = gen_rtx_MEM (SImode,
			     gen_rtx_PLUS (SImode,
					   sp,
					   gen_rtx_CONST_INT (SImode,
							      -8)));

      emit_move_insn (loc, gen_rtx_CONST_INT (SImode, argcount));
    }

  if (wasm32_is_real_stackcall (funtype))
    {
      rtx loc = gen_rtx_MEM (SImode,
			     gen_rtx_PLUS (SImode,
					   sp,
					   gen_rtx_CONST_INT (SImode,
							      -16)));
      emit_move_insn (loc, const0_rtx);
    }

  call = gen_rtx_CALL (retval ? SImode : VOIDmode, address, sig);

  if (retval)
    call = gen_rtx_SET (gen_rtx_REG (SImode, RV_REG), call);
  else
    clobber_reg (&use, gen_rtx_REG (SImode, RV_REG));

  clobber_reg (&use, gen_rtx_REG (SImode, RV_REG)); // XXX
  use_reg (&use, gen_rtx_REG (SImode, SP_REG)); // XXX
  use_reg (&use, gen_rtx_REG (SImode, R0_REG)); // XXX
  use_reg (&use, gen_rtx_REG (SImode, R1_REG)); // XXX
  call_insn = emit_call_insn (call);

  if (use)
    CALL_INSN_FUNCTION_USAGE (call_insn) = use;

  return call_insn;
}

/*
  Stack layout
	      stack args
 AP,CFA   ->
	      unused
	      unused
	      unused
	      old FP
	      registers as specified
	      size of regblock
	      current SP
	      current PC
	      bitmask
 FP,SP    ->
*/

rtx
wasm32_expand_prologue ()
{
  poly_int64 size = get_frame_size () + crtl->outgoing_args_size;
  rtx sp = gen_rtx_REG (SImode, SP_REG);
  size = (size.to_constant () + 7) & -8;
  int regsize = wasm32_function_regsize (NULL_TREE);
  last_regsize = regsize;
  rtx insn;

  RTX_FRAME_RELATED_P (insn = emit_move_insn (sp, plus_constant (Pmode, sp, -regsize))) = 0;
  RTX_FRAME_RELATED_P (insn = emit_move_insn (frame_pointer_rtx, sp)) = 1;
  add_reg_note (insn, REG_CFA_DEF_CFA,
		plus_constant (Pmode, frame_pointer_rtx, 0));
  add_reg_note (insn, REG_CFA_EXPRESSION, gen_rtx_SET (gen_rtx_MEM (Pmode, gen_rtx_PLUS (Pmode, sp, gen_rtx_MINUS (Pmode, gen_rtx_MEM (Pmode, sp), sp))), frame_pointer_rtx));
  add_reg_note (insn, REG_CFA_OFFSET, gen_rtx_SET (gen_rtx_MEM (Pmode, plus_constant (Pmode, sp, 24)), pc_rtx));

  if (crtl->calls_eh_return)
    {
      /* XXX recalc offsets */
      add_reg_note (insn, REG_CFA_OFFSET, gen_rtx_SET (gen_rtx_MEM (SImode, plus_constant (Pmode, sp, 48)), gen_rtx_REG (SImode, A0_REG)));
      add_reg_note (insn, REG_CFA_OFFSET, gen_rtx_SET (gen_rtx_MEM (SImode, plus_constant (Pmode, sp, 56)), gen_rtx_REG (SImode, A1_REG)));
      add_reg_note (insn, REG_CFA_OFFSET, gen_rtx_SET (gen_rtx_MEM (SImode, plus_constant (Pmode, sp, 64)), gen_rtx_REG (SImode, A2_REG)));
      add_reg_note (insn, REG_CFA_OFFSET, gen_rtx_SET (gen_rtx_MEM (SImode, plus_constant (Pmode, sp, 72)), gen_rtx_REG (SImode, A3_REG)));
    }

  if (size.to_constant () != 0)
    RTX_FRAME_RELATED_P (emit_move_insn (sp, gen_rtx_PLUS (SImode, sp, gen_rtx_CONST_INT (SImode, (-size).to_constant ())))) = 0;

  return NULL;
}

rtx
wasm32_expand_epilogue()
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

const char *
wasm32_expand_ret_insn()
{
  char buf[1024];
  snprintf (buf, 1022, "i32.const %d\n\tlocal.get $fp\n\ti32.add\n\treturn\n\t.set __wasm32_fallthrough, 0",
	    wasm32_function_regsize (NULL_TREE));

  return ggc_strdup (buf);
}

int
wasm32_max_conditional_execute()
{
  return max_conditional_insns;
}

bool
wasm32_can_eliminate (int reg0, int reg1)
{
  if (reg0 == AP_REG && reg1 == FP_REG)
    return true;
  if (reg0 == AP_REG && reg1 == SP_REG)
    return true;

  return false;
}

int
wasm32_initial_elimination_offset (int reg0, int reg1)
{
  if (reg0 == AP_REG && reg1 == FP_REG)
    return 16 + wasm32_function_regsize (NULL_TREE);
  else if (reg0 == FP_REG && reg1 == AP_REG)
    return -16 - wasm32_function_regsize (NULL_TREE);
  if (reg0 == FP_REG && reg1 == SP_REG)
    return 0;
  else if (reg0 == SP_REG && reg1 == FP_REG)
    return 0;
  if (reg0 == AP_REG && reg1 == SP_REG)
    return 16 + wasm32_function_regsize (NULL_TREE);
  else if (reg0 == SP_REG && reg1 == AP_REG)
    return -16 - wasm32_function_regsize (NULL_TREE);
  else
    gcc_unreachable ();
}

int
wasm32_branch_cost (bool speed_p ATTRIBUTE_UNUSED,
		    bool predictable_p ATTRIBUTE_UNUSED)
{
  return my_branch_cost;
}

int
wasm32_register_move_cost (machine_mode mode ATTRIBUTE_UNUSED,
			   reg_class_t from,
			   reg_class_t to)
{
  if (from == GENERAL_REGS && to == GENERAL_REGS)
    return my_register_cost;
  if ((from == GENERAL_REGS && to == THREAD_REGS)
      || (from == THREAD_REGS && to == GENERAL_REGS))
    return 4 * my_register_cost;
  if (from == THREAD_REGS && to == THREAD_REGS)
    return 7 * my_register_cost;
  return my_register_cost;
}

int
wasm32_memory_move_cost (machine_mode mode ATTRIBUTE_UNUSED,
			 reg_class_t from ATTRIBUTE_UNUSED,
			 bool in ATTRIBUTE_UNUSED)
{
  return my_memory_cost;
}

bool
wasm32_rtx_costs (rtx x, machine_mode mode ATTRIBUTE_UNUSED,
		  int outer_code ATTRIBUTE_UNUSED,
		  int opno ATTRIBUTE_UNUSED, int *total,
		  bool speed ATTRIBUTE_UNUSED)
{
  switch (GET_CODE (x))
    {
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

/* Always use LRA, as is now recommended.  */

bool
wasm32_lra_p ()
{
  return true;
}


bool
wasm32_cxx_library_rtti_comdat (void)
{
  return true;
}

bool
wasm32_cxx_class_data_always_comdat (void)
{
  return true;
}

static rtx
wasm32_trampoline_adjust_address (rtx addr)
{
  return gen_rtx_MEM (SImode, plus_constant (SImode, addr, 24));
}

/* Trampolines are merely three-word blocks of data aligned to a
   16-byte boundary; "TRAM" followed by a function pointer followed by
   a value to load in the static chain register.  The JavaScript code
   handles the recognition and evaluation of trampolines, so it's
   quite slow.  */

static void
wasm32_trampoline_init (rtx m_tramp, tree fndecl, rtx static_chain)
{
  rtx fnaddr = force_reg (Pmode, XEXP (DECL_RTL (fndecl), 0));
  m_tramp = XEXP (m_tramp, 0);
  emit_insn (gen_rtx_SET (gen_rtx_MEM (SImode, m_tramp), gen_rtx_CONST_INT(SImode, 0x4d4a5254L)));
  emit_insn (gen_rtx_SET (gen_rtx_MEM (SImode, plus_constant (SImode, m_tramp, 8)), fnaddr));
  emit_insn (gen_rtx_SET (gen_rtx_MEM (SImode, plus_constant (SImode, m_tramp, 16)), static_chain));
  emit_insn (gen_rtx_SET (gen_rtx_REG (SImode, R0_REG), m_tramp));
  wasm32_expand_call (NULL, gen_rtx_MEM (QImode, gen_rtx_SYMBOL_REF (SImode, "*__wasm_init_trampoline")), gen_rtx_CONST_STRING (VOIDmode, ""));
}

static void
wasm32_trampoline_destroy (rtx m_tramp)
{
  emit_insn (gen_rtx_SET (gen_rtx_REG (SImode, R0_REG), m_tramp));
  wasm32_expand_call (NULL, gen_rtx_MEM (QImode, gen_rtx_SYMBOL_REF (SImode, "*__wasm_destroy_trampoline")), gen_rtx_CONST_STRING (VOIDmode, ""));
}

static bool
wasm32_asm_can_output_mi_thunk (const_tree, HOST_WIDE_INT, HOST_WIDE_INT,
				const_tree)
{
  return true;
}

#if 0 /* unused */
/* Return RTX for the first (implicit this) parameter passed to a method. */

static rtx
wasm32_this_parameter (tree function)
{
  if (wasm32_is_stackcall (TREE_TYPE (function)))
    return gen_rtx_MEM (SImode, plus_constant (SImode,
					       gen_rtx_REG (SImode, SP_REG),
					       /* XXX */
					       16));
  else
    return gen_rtx_REG (SImode, R0_REG);
}
#endif

/* Adjust the "this" parameter and jump to function. We can't actually
   force a tail call to happen on the JS stack, but we can force one
   on the VM stack. Do so.

   A slight complication here: FUNCTION might be a stack-call function
   with an aggregate return type, so we need to check for that before
   deciding where on the stack the this argument lives.
 */
static void
wasm32_asm_output_mi_thunk (FILE *f, tree thunk_fndecl, HOST_WIDE_INT delta,
			    HOST_WIDE_INT vcall_offset, tree function)
{
  const char *tname = XSTR (XEXP (DECL_RTL (function), 0), 0);
  const char *name = XSTR (XEXP (DECL_RTL (thunk_fndecl), 0), 0);
  bool stackcall = wasm32_is_stackcall (TREE_TYPE (function));
  bool structret = aggregate_value_p (TREE_TYPE (TREE_TYPE (function)), TREE_TYPE (function));
  const char *r = (structret && !stackcall) ? "$r1" : "$r0";
  const char *fnname = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (thunk_fndecl));

  tname += tname[0] == '*';
  name += name[0] == '*';

  assemble_start_function (thunk_fndecl, fnname);

  if (stackcall)
    {
      asm_fprintf (f, "\tlocal.get $sp\n\ti32.const %d\n\ti32.add\n\tlocal.set %s\n", structret ? 20 : 16, r);
      asm_fprintf (f, "\tlocal.get %s\n\ti32.load a=2 0\n\tlocal.set %s\n", r, r);
    }

  asm_fprintf (f, "\tlocal.get %s\n\ti32.const %d\n\ti32.add\n\tlocal.set %s\n",
	       r, (int) delta, r);
  if (vcall_offset)
    {
      asm_fprintf (f, "\ti32.const $rv\n\tlocal.get %s\n\ti32.load a=2 0\n\ti32.store a=2 0\n", r);
      asm_fprintf (f, "\ti32.const $rv\n\ti32.const $rv\n\ti32.load a=2 0\n\ti32.const %d\n\ti32.add\n\ti32.store a=2 0\n", (int)vcall_offset);
      asm_fprintf (f, "\ti32.const $rv\n\ti32.const $rv\n\ti32.load a=2 0\n\ti32.load a=2 0\n\ti32.store a=2 0\n");
      asm_fprintf (f, "\tlocal.get %s\n\ti32.const $rv\n\ti32.load a=2 0\n\ti32.add\n\tlocal.set %s\n",
		   r, r);
    }

  if (stackcall)
    asm_fprintf (f, "\tlocal.get $sp\n\ti32.const %d\n\ti32.add\n\tlocal.get %s\n\ti32.store a=2 0\n", structret ? 20 : 16, r);

  asm_fprintf (f, "\ti32.const -1\n\tlocal.get $sp1\n\tlocal.get $r0\n\tlocal.get $r1\n\ti32.const 0\n\ti32.const 0\n\tcall %s@plt{__sigchar_FiiiiiiiE}\n\treturn\n",
	       tname);

  assemble_end_function (thunk_fndecl, fnname);
}

#undef TARGET_ASM_GLOBALIZE_LABEL
#define TARGET_ASM_GLOBALIZE_LABEL wasm32_globalize_label

#undef TARGET_LEGITIMATE_CONSTANT_P
#define TARGET_LEGITIMATE_CONSTANT_P wasm32_legitimate_constant_p

#undef TARGET_LEGITIMATE_ADDRESS_P
#define TARGET_LEGITIMATE_ADDRESS_P wasm32_legitimate_address_p

#undef TARGET_STRICT_ARGUMENT_NAMING
#define TARGET_STRICT_ARGUMENT_NAMING wasm32_strict_argument_naming

#undef TARGET_FUNCTION_ARG_BOUNDARY
#define TARGET_FUNCTION_ARG_BOUNDARY wasm32_function_arg_boundary

#undef TARGET_FUNCTION_ARG_ROUND_BOUNDARY
#define TARGET_FUNCTION_ARG_ROUND_BOUNDARY wasm32_function_arg_round_boundary

#undef TARGET_FUNCTION_ARG_ADVANCE
#define TARGET_FUNCTION_ARG_ADVANCE wasm32_function_arg_advance

#undef TARGET_FUNCTION_INCOMING_ARG
#define TARGET_FUNCTION_INCOMING_ARG wasm32_function_incoming_arg

#undef TARGET_FUNCTION_ARG
#define TARGET_FUNCTION_ARG wasm32_function_arg

#undef TARGET_FRAME_POINTER_REQUIRED
#define TARGET_FRAME_POINTER_REQUIRED wasm32_frame_pointer_required

#undef TARGET_DEBUG_UNWIND_INFO
#define TARGET_DEBUG_UNWIND_INFO  wasm32_debug_unwind_info

#undef TARGET_CALL_ARGS
#define TARGET_CALL_ARGS wasm32_call_args

#undef TARGET_END_CALL_ARGS
#define TARGET_END_CALL_ARGS wasm32_end_call_args

#undef TARGET_OPTION_OVERRIDE
#define TARGET_OPTION_OVERRIDE wasm32_option_override

#undef TARGET_ATTRIBUTE_TABLE
#define TARGET_ATTRIBUTE_TABLE wasm32_attribute_table

#undef TARGET_REGISTER_PRIORITY
#define TARGET_REGISTER_PRIORITY wasm32_register_priority

#undef TARGET_RTX_COSTS
#define TARGET_RTX_COSTS wasm32_rtx_costs

#undef TARGET_CXX_LIBRARY_RTTI_COMDAT
#define TARGET_CXX_LIBRARY_RTTI_COMDAT wasm32_cxx_library_rtti_comdat

#undef TARGET_CXX_CLASS_DATA_ALWAYS_COMDAT
#define TARGET_CXX_CLASS_DATA_ALWAYS_COMDAT wasm32_cxx_class_data_always_comdat

#undef TARGET_TRAMPOLINE_ADJUST_ADDRESS
#define TARGET_TRAMPOLINE_ADJUST_ADDRESS wasm32_trampoline_adjust_address

#undef TARGET_TRAMPOLINE_INIT
#define TARGET_TRAMPOLINE_INIT wasm32_trampoline_init

#undef TARGET_DESTROY_TRAMPOLINE
#define TARGET_DESTROY_TRAMPOLINE wasm32_trampoline_destroy

#undef TARGET_ABSOLUTE_BIGGEST_ALIGNMENT
#define TARGET_ABSOLUTE_BIGGEST_ALIGNMENT 1024

#undef TARGET_ASM_OUTPUT_MI_THUNK
#define TARGET_ASM_OUTPUT_MI_THUNK wasm32_asm_output_mi_thunk

#undef TARGET_ASM_CAN_OUTPUT_MI_THUNK
#define TARGET_ASM_CAN_OUTPUT_MI_THUNK wasm32_asm_can_output_mi_thunk

#define TARGET_PROMOTE_FUNCTION_MODE wasm32_promote_function_mode
#define TARGET_ABSOLUTE_BIGGEST_ALIGNMENT 1024
#define TARGET_CAN_ELIMINATE wasm32_can_eliminate
#define TARGET_PROMOTE_PROTOTYPES wasm32_promote_prototypes
#define TARGET_RETURN_POPS_ARGS wasm32_return_pops_args
#define TARGET_FUNCTION_VALUE wasm32_function_value
#define TARGET_LIBCALL_VALUE wasm32_libcall_value
#define TARGET_FUNCTION_VALUE_REGNO_P wasm32_function_value_regno_p
#define TARGET_RETURN_IN_MEMORY wasm32_return_in_memory
#define TARGET_STRUCT_VALUE_RTX wasm32_struct_value_rtx
#define TARGET_REGISTER_MOVE_COST wasm32_register_move_cost
#define TARGET_MEMORY_MOVE_COST wasm32_memory_move_cost
#define TARGET_LRA_P wasm32_lra_p

#include "target-def.h"

#undef TARGET_ASM_ALIGNED_DI_OP
#define TARGET_ASM_ALIGNED_DI_OP "\t.quad\t"

struct gcc_target targetm = TARGET_INITIALIZER;
