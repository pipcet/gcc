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
#include "cp/cxx-pretty-print.h"

extern void mangle_decl(tree decl);
extern tree get_mangled_id(tree decl, bool force);

struct asmjs_jsexport_decl
asmjs_jsexport_type (tree type, const char *jsname ATTRIBUTE_UNUSED)
{
  debug_tree (type);

  struct asmjs_jsexport_decl ret;
  cxx_pretty_printer cxxpp;

  cxxpp.buffer->stream = NULL;
  cxxpp.buffer->flush_p = false;
  cxxpp.flags |= pp_c_flag_abstract;

  cxxpp.type_id (type);

  ret.pre_addr = concat (output_buffer_formatted_text (cxxpp.buffer), NULL);

  return ret;
}

struct asmjs_jsexport_decl
asmjs_jsexport_decl (tree decl, const char *jsname)
{
  struct asmjs_jsexport_decl ret;
  tree type = TREE_TYPE (decl);
  cxx_pretty_printer cxxpp;
  const char *typestr;

  cxxpp.buffer->stream = NULL;
  cxxpp.buffer->flush_p = false;
  cxxpp.flags |= pp_c_flag_abstract;

  cxxpp.declaration (decl);

  const char *types = "";
  if (TREE_CODE (decl) == FUNCTION_DECL)
    {
      tree arg_types = TYPE_ARG_TYPES (type);

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
	    continue;

	  debug_tree (arg_type);
	  types = concat (types, asmjs_jsexport_type (arg_type, NULL).pre_addr, "; ", NULL);
	}
    }
  debug_tree (build_pointer_type (type));
  {
    cxx_pretty_printer cxxpp;

    cxxpp.buffer->stream = NULL;
    cxxpp.buffer->flush_p = false;
    cxxpp.flags |= pp_c_flag_abstract;

    cxxpp.type_id (build_pointer_type (type));

    typestr = ggc_strdup(output_buffer_formatted_text (cxxpp.buffer));
  }

  ret.decl = decl;
  if (!jsname)
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
  ret.jsname = jsname;
  ret.symbol = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl));
  ret.pre_addr =
    concat (types,
	    "asmjs_register(\"", ret.jsname,
	    "\", ", ret.symbol, ", ",
	    "(", typestr, ")", NULL);
  ret.post_addr =
    concat(", typeid(", typestr, ").name());", NULL);

  return ret;
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

      debug_tree (node);
  if (DECL_P (node))
    return asmjs_jsexport_decl (node, jsname);
  else if (TYPE_P (node))
    return asmjs_jsexport_type (node, jsname);
  else
    {
      debug_tree (node);
      error("unknown tree type");
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
