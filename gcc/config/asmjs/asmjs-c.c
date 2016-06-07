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
#include "predict.h"
#include "dominance.h"
#include "cfg.h"
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
#include "tm_p.h"
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
#include "sched-int.h"
#include "output.h"
#include "calls.h"
#include "config/linux-protos.h"
#include "real.h"
#include "fixed-value.h"
#include "explow.h"
#include "stmt.h"
#include "tree-chkp.h"
#include "print-tree.h"
#include "varasm.h"
#include "c-family/c-target.h"
#include "c-family/c-target-def.h"
#include "c-family/c-pretty-print.h"

const char *
asmjs_jsexport_format_type (tree type)
{
  c_pretty_printer cpp;

  cpp.buffer->stream = NULL;
  cpp.buffer->flush_p = false;
  cpp.flags |= pp_c_flag_abstract;

  cpp.type_id (type);

  return concat (output_buffer_formatted_text (cpp.buffer), NULL);
}

struct asmjs_jsexport_decl
asmjs_jsexport_record_type (tree type, const char *jsname ATTRIBUTE_UNUSED)
{
  struct asmjs_jsexport_decl ret;
  const char *typestr = asmjs_jsexport_format_type (type);

  if (!jsname)
    jsname = IDENTIFIER_POINTER (TYPE_IDENTIFIER (type));

  ret.jsname = jsname;
  ret.fragments = vec<const char *>();

  ret.fragments.safe_push(concat ("asmjs_register_record_type(\"", ret.jsname,
			     "\", ", typestr, ");", NULL));

  tree field;

  for (field = TYPE_FIELDS (type); TREE_CODE (field) == FIELD_DECL; field = TREE_CHAIN (field))
    {
      const char *fieldtypestr = asmjs_jsexport_format_type (TREE_TYPE (field));

      /* ignore bitfields */
      if (strchr (fieldtypestr, ':'))
	continue;

      char bitpos[16];
      char bytesize[16];
      snprintf(bitpos, 16, "%ld", (long)int_bit_position (field));
      snprintf(bytesize, 16, "%ld", (long)int_size_in_bytes (TREE_TYPE (field)));

      ret.fragments.safe_push(concat ("asmjs_register_field(\"", ret.jsname,
				      "\", ", typestr,
				      ", ", fieldtypestr,
				      ", ",
				      IDENTIFIER_POINTER (DECL_NAME (field)),
				      ", ", bitpos,
				      ", ", bytesize,
				      ");",
				 NULL));
    }

  return ret;
}

struct asmjs_jsexport_decl
asmjs_jsexport_type (tree type, const char *jsname ATTRIBUTE_UNUSED)
{
  if (TREE_CODE (type) == POINTER_TYPE)
    return asmjs_jsexport_type (TREE_TYPE (type), jsname);
  if (TREE_CODE (type) == RECORD_TYPE)
    return asmjs_jsexport_record_type (type, jsname);
  struct asmjs_jsexport_decl ret;
  c_pretty_printer cpp;

  debug_tree (type);

  cpp.buffer->stream = NULL;
  cpp.buffer->flush_p = false;
  cpp.flags |= pp_c_flag_abstract;

  cpp.type_id (type);

  ret.fragments = vec<const char *>();
  ret.fragments.safe_push (concat (output_buffer_formatted_text (cpp.buffer), NULL));

  return ret;
}

struct asmjs_jsexport_decl
asmjs_jsexport_function_decl (tree decl, const char *jsname)
{
  struct asmjs_jsexport_decl ret;
  tree type = TREE_TYPE (decl);
  const char *typestr;
  const char *rettypestr;
  const char *classtypestr = "void";

  const char *types = "";
  if (TREE_CODE (decl) == FUNCTION_DECL)
    {
      tree arg_types = TYPE_ARG_TYPES (type);
      bool first = true;

      while (arg_types)
	{
	  tree arg_type = TREE_VALUE (arg_types);
	  arg_types = TREE_CHAIN (arg_types);

	  while (TREE_CODE (arg_type) == POINTER_TYPE)
	    {
	      arg_type = TREE_TYPE (arg_type);
	    }

	  if (TREE_CODE (arg_type) != RECORD_TYPE &&
	      TREE_CODE (arg_type) != UNION_TYPE &&
	      TREE_CODE (arg_type) != ENUMERAL_TYPE)
	    {
	      first = false;
	      continue;
	    }

	  if (first)
	    {
	      classtypestr = asmjs_jsexport_format_type (build_pointer_type (arg_type));
	      first = false;
	    }

	  types = concat (types, asmjs_jsexport_format_type (arg_type), "; ", NULL);
	}
    }
  typestr = asmjs_jsexport_format_type (build_pointer_type (type));
  rettypestr = asmjs_jsexport_format_type (TREE_TYPE (type));

  ret.decl = decl;
  if (!jsname && (TREE_CODE (type) == METHOD_TYPE ||
		  TREE_CODE (type) == FUNCTION_TYPE))
    {
      tree basetype = TYPE_METHOD_BASETYPE (type);

      jsname = "";

      while (basetype)
	{
	  switch (TREE_CODE (basetype))
	    {
	    case IDENTIFIER_NODE:
	      if (IDENTIFIER_POINTER (basetype))
		jsname = concat (IDENTIFIER_POINTER (basetype), "::", NULL);
	      basetype = NULL;
	      break;

	    case TYPE_DECL:
	      basetype = DECL_NAME (basetype);
	      break;

	    case RECORD_TYPE:
	      basetype = TYPE_NAME (basetype);
	      break;

	    default:
	      basetype = NULL;
	      break;
	    }
	}

      jsname = concat (jsname, IDENTIFIER_POINTER (DECL_NAME (decl)), NULL);
    }
  else if (!jsname)
    {
      jsname = "";
      jsname = concat (jsname, IDENTIFIER_POINTER (DECL_NAME (decl)), NULL);
    }
  ret.jsname = jsname;
  ret.symbol = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl));
  ret.fragments = vec<const char *>();
  ret.fragments.safe_push(concat (types,
				  "asmjs_register_function(\"", ret.jsname,
				  "\", ", ret.symbol, "__", NULL));
  ret.fragments.safe_push(NULL);
  ret.fragments.safe_push(concat (", ", "(", typestr, ")", NULL));
  ret.fragments.safe_push(NULL);
  ret.fragments.safe_push(concat(", typeid(", classtypestr, ").name()",
				 ", typeid(", rettypestr, ").name()",
				 ", typeid(", typestr, ").name());", NULL));

  return ret;
}

struct asmjs_jsexport_decl
asmjs_jsexport_type_decl (tree node, const char *jsname)
{
  debug_tree (node);

  if (TREE_CODE (TREE_TYPE (node)) == RECORD_TYPE)
    return asmjs_jsexport_type (TREE_TYPE (node), jsname);

  struct asmjs_jsexport_decl ret;

  ret.fragments = vec<const char *>();

  return ret;
}

struct asmjs_jsexport_decl
asmjs_jsexport_var_decl (tree decl, const char *jsname)
{
  struct asmjs_jsexport_decl ret;
  tree type = TREE_TYPE (decl);
  tree attrs = TYPE_ATTRIBUTES (type);
  const char *type_jsname = NULL;

  if (attrs != NULL_TREE)
    {
      tree attr = lookup_attribute ("jsexport", attrs);

      if (attr != NULL_TREE)
	{
	  tree arg1 = TREE_VALUE (attr);

	  if (arg1)
	    type_jsname = TREE_STRING_POINTER (arg1);
	}
    }

  if (!type_jsname)
    {
      tree basetype = type;

      while (basetype)
	{
	  switch (TREE_CODE (basetype))
	    {
	    case IDENTIFIER_NODE:
	      if (IDENTIFIER_POINTER (basetype))
		type_jsname = concat (IDENTIFIER_POINTER (basetype), NULL);
	      basetype = NULL;
	      break;

	    case TYPE_DECL:
	      basetype = DECL_NAME (basetype);
	      break;

	    case RECORD_TYPE:
	      basetype = TYPE_NAME (basetype);
	      break;

	    default:
	      basetype = NULL;
	      break;
	    }
	}
    }

  if (!jsname)
    {
      jsname = "";
      jsname = concat (jsname, IDENTIFIER_POINTER (DECL_NAME (decl)), NULL);
    }

  ret.jsname = jsname;

  ret.fragments = vec<const char *>();
  ret.fragments.safe_push (concat ("asmjs_register_variable(\"", ret.jsname,
				   "\", \"", type_jsname, "\", ", NULL));
  ret.fragments.safe_push (NULL);
  ret.fragments.safe_push (concat (");", NULL));
  ret.symbol = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl));

  return ret;
}

struct asmjs_jsexport_decl
asmjs_jsexport_decl (tree node, const char *jsname)
{
  switch (TREE_CODE (node))
    {
    case FUNCTION_DECL:
      return asmjs_jsexport_function_decl (node, jsname);
    case TYPE_DECL:
      return asmjs_jsexport_type_decl (node, jsname);
    case VAR_DECL:
      return asmjs_jsexport_var_decl (node, jsname);
    default:
      printf("unknown code:\n");
      debug_tree (node);
      gcc_unreachable();
    }
}

struct asmjs_jsexport_decl
asmjs_jsexport (tree node, const char *jsname)
{
#if 0
  const char *mangled = NULL;
  const char *force_mangled = NULL;

  if (DECL_P (node) && !DECL_ASSEMBLER_NAME_SET_P (node))
    mangle_decl (node);

  if (DECL_P (node))
    {
      mangled = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (node));

      force_mangled = IDENTIFIER_POINTER (get_mangled_id (node, true));
    }
  fprintf(stderr, "name %s mangled %s force-mangled %s\n", DECL_P (node) ? IDENTIFIER_POINTER (DECL_NAME (node)) : NULL,
	  mangled, force_mangled);
#endif

  if (DECL_P (node))
    return asmjs_jsexport_decl (node, jsname);
  else if (TYPE_P (node))
    return asmjs_jsexport_type (node, jsname);
  else
    {
      error("unknown tree type");
      debug_tree (node);
      gcc_unreachable ();
    }
}

/* Do not include tm.h or tm_p.h here; if it is useful for a target to
   define some macros for the initializer in a header without defining
   targetcm itself (for example, because of interactions with some
   hooks depending on the target OS and others on the target
   architecture), create a separate tm_c.h for only the relevant
   definitions.  */

struct gcc_targetcm targetcm = TARGETCM_INITIALIZER;
