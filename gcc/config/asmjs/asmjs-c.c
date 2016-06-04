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

struct asmjs_jsexport_decl
asmjs_jsexport (tree node, const char *jsname)
{
  struct asmjs_jsexport_decl ret;

  return ret;

  c_pretty_printer cpp;
  cpp.buffer->stream = NULL;
  cpp.buffer->flush_p = false;
  cpp.flags |= pp_c_flag_abstract;

  if (DECL_P (node))
    cpp.declaration (node);
  else
    cpp.type_id (node);

  //return concat(output_buffer_formatted_text (cpp.buffer), NULL);
}

/* Do not include tm.h or tm_p.h here; if it is useful for a target to
   define some macros for the initializer in a header without defining
   targetcm itself (for example, because of interactions with some
   hooks depending on the target OS and others on the target
   architecture), create a separate tm_c.h for only the relevant
   definitions.  */

struct gcc_targetcm targetcm = TARGETCM_INITIALIZER;
