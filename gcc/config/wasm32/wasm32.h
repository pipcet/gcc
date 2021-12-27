/* GCC backend for the asm.js target
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
   Copyright (C) 2016 Pip Cet <pipcet@gmail.com>
   based on the ARM port, which was:
   Contributed by Richard Earnshaw (rearnsha@armltd.co.uk).

   This file is NOT part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

#define ASM_SPEC "-I gas-macros%s"
/* #define STARTFILE_SPEC "crti%O%s crtbegin%O%s --whole-archive gdbstub%O%s --no-whole-archive" */
#define STARTFILE_SPEC "crti%O%s %{!shared:crtbegin%O%s} %{shared:crtbegin%O%s}"
#define ENDFILE_SPEC "%{!shared:crtend%O%s} %{shared:crtend%O%s} crtn%O%s"

#define WASM32_LINK_SPEC "%{shared:-shared} %{!static: %{rdynamic:-export-dynamic}} %{static:-static} -rpath-link $ORIGIN"

#undef LINK_SPEC
#define LINK_SPEC WASM32_LINK_SPEC

#define TARGET_CPU_CPP_BUILTINS() wasm32_cpu_cpp_builtins(pfile)

/* #define SHIFT_COUNT_TRUNCATED should technically be set. */

/* XXX include elfos.h? */

/* XXX TARGET_FLOAT_EXCEPTIONS_ROUNDING_SUPPORTED_P -> 0 */

/* WebAssembly is strictly little-endian.  */
#define BITS_BIG_ENDIAN 0
#define BYTES_BIG_ENDIAN 0
#define WORDS_BIG_ENDIAN BYTES_BIG_ENDIAN

#define UNITS_PER_WORD 4

#define PARM_BOUNDARY 32
#define STACK_BOUNDARY 64
#define PREFERRED_STACK_BOUNDARY 64
#define MAIN_STACK_BOUNDARY 64
#define MIN_STACK_BOUNDARY 64
#define MAX_STACK_ALIGNMENT 64
#define INCOMING_STACK_BOUNDARY 64
#define FUNCTION_BOUNDARY 8
#define BIGGEST_ALIGNMENT 64
#define MALLOC_ABI_ALIGNMENT 64
#define MAX_OFILE_ALIGNMENT (32768 * 8)
/* Nonzero if move instructions will actually fail to work when given
   unaligned data.  XXX this should be 0.  */
#define STRICT_ALIGNMENT 1
// #define MAX_FIXED_MODE_SIZE Pmode

#define INT_TYPE_SIZE 32
#define LONG_LONG_TYPE_SIZE 64
#define FLOAT_TYPE_SIZE 32
#define DOUBLE_TYPE_SIZE 64
#define LONG_DOUBLE_TYPE_SIZE 64 /* The proposed wasm ABI has 128.  */

#define DEFAULT_SIGNED_CHAR  0 /* XXX check wasm ABI */

#define SIZE_TYPE "unsigned int"
#define PTRDIFF_TYPE "int"
#define WCHAR_TYPE "int"
#define SIG_ATOMIC_TYPE "long int"

#define INT8_TYPE "signed char"
#define INT16_TYPE "short int"
#define INT32_TYPE "int"
#define INT64_TYPE "long long int"
#define UINT8_TYPE "unsigned char"
#define UINT16_TYPE "short unsigned int"
#define UINT32_TYPE "unsigned int"
#define UINT64_TYPE "long long unsigned int"
#define INT_LEAST8_TYPE "signed char"
#define INT_LEAST16_TYPE "short int"
#define INT_LEAST32_TYPE "int"
#define INT_LEAST64_TYPE "long long int"
#define UINT_LEAST8_TYPE "unsigned char"
#define UINT_LEAST16_TYPE "short unsigned int"
#define UINT_LEAST32_TYPE "unsigned int"
#define UINT_LEAST64_TYPE "long long unsigned int"
#define INT_FAST8_TYPE "signed char"
#define INT_FAST16_TYPE "int"
#define INT_FAST32_TYPE "int"
#define INT_FAST64_TYPE "long long int"
#define UINT_FAST8_TYPE "unsigned char"
#define UINT_FAST16_TYPE "unsigned int"
#define UINT_FAST32_TYPE "unsigned int"
#define UINT_FAST64_TYPE "long long unsigned int"
#define INTPTR_TYPE "int"
#define UINTPTR_TYPE "unsigned int"

#define FIRST_PSEUDO_REGISTER 36

#define FIXED_REGISTERS				\
  { 1, 1, 1, 0,					\
    0, 0, 0, 0,					\
    0, 0, 0, 0,					\
    0, 0, 0, 0,					\
    0, 0, 0, 0,					\
    0, 0, 0, 0,					\
    0, 0, 0, 0,					\
    0, 0, 0, 0,                                 \
    1, 1, 1, 1                                  \
  }

#define CALL_USED_REGISTERS			\
  {   1, 1, 1, 1,				\
      1, 1, 1, 1,                               \
      0, 0, 0, 0, 				\
      0, 0, 0, 0, 				\
      0, 0, 0, 0, 				\
      0, 0, 0, 0, 				\
      0, 0, 0, 0, 				\
      0, 0, 0, 0, 				\
      1, 1, 1, 1,				\
   }

#define REGMODE_NATURAL_SIZE(mode) wasm32_regmode_natural_size(mode)
#define HARD_REGNO_RENAME_OK(from, to) wasm32_hard_regno_rename_ok(from, to)

#define N_REG_CLASSES  (int) LIM_REG_CLASSES
#define REG_CLASS_NAMES { "NO_REGS", "PC_REGS", "SP_REGS", "FLOAT_REGS", "GENERAL_REGS", "THREAD_REGS", "ALL_REGS", }
#define REG_CLASS_CONTENTS			\
  {						\
    { 0x00000000, 0x0, }, /* NO_REGS */		\
    { 0x00000002, 0x0, }, /* PC_REGS */		\
    { 0x00000004, 0x0, }, /* SP_REGS */		\
    { 0xFF000000, 0x0, }, /* FLOAT_REGS */	\
    { 0xFFFFFF07, 0xd, }, /* GENERAL_REGS */	\
    { 0x000000F8, 0x2, }, /* THREAD_REGS */       \
    { 0xFFFFFFFF, 0xf, }, /* ALL_REGS */		\
}
#define REGNO_REG_CLASS(REGNO) wasm32_regno_reg_class(REGNO)
#define BASE_REG_CLASS GENERAL_REGS
#define INDEX_REG_CLASS GENERAL_REGS
#define REGNO_OK_FOR_BASE_P(regno) wasm32_regno_ok_for_base_p(regno)
#define REGNO_OK_FOR_INDEX_P(regno) wasm32_regno_ok_for_index_p(regno)

#define STACK_GROWS_DOWNWARD 1
#define STACK_PUSH_CODE PRE_DEC
#define FRAME_GROWS_DOWNWARD 1

/* Offset of first parameter from the argument pointer register value.  */
#define FIRST_PARM_OFFSET(FNDECL) wasm32_first_parm_offset(FNDECL)

#define DYNAMIC_CHAIN_ADDRESS(frameaddr) wasm32_dynamic_chain_address(frameaddr)

#define SETUP_FRAME_ADDRESSES() do {				  \
    emit_insn (gen_flush_register_windows ());			  \
  } while (0)

#define RETURN_ADDR_RTX(count, frameaddr) wasm32_return_addr_rtx(count, frameaddr)
#define INCOMING_RETURN_ADDR_RTX wasm32_incoming_return_addr_rtx()
#define INCOMING_FRAME_SP_OFFSET 0

#define FRAME_POINTER_CFA_OFFSET(fundecl) (0)

#define EH_RETURN_DATA_REGNO(n) (((n) < 3) ? (A0_REG + (n)) : INVALID_REGNUM)
#define EH_RETURN_STACKADJ_RTX gen_rtx_REG (SImode, 7)
#define EH_RETURN_HANDLER_RTX RETURN_ADDR_RTX(0, gen_rtx_REG (SImode, FP_REG))

#define STACK_POINTER_REGNUM 2
#define FRAME_POINTER_REGNUM 0
#define ARG_POINTER_REGNUM 32
#define STATIC_CHAIN_REGNUM RV_REG

#define TARGET_FRAME_POINTER_REQUIRED() true
#define INITIAL_FRAME_POINTER_OFFSET(depth_var) do { depth_var = 0; } while(0)
#define ELIMINABLE_REGS { { AP_REG, FP_REG }, { AP_REG, SP_REG } }
#define INITIAL_ELIMINATION_OFFSET(from,to,ovar) ((ovar) = wasm32_initial_elimination_offset(from, to))


#define ACCUMULATE_OUTGOING_ARGS 1
#define CALL_POPS_ARGS wasm32_call_pops_args

typedef struct {
  int n_areg; // XXX used??
  int n_vreg; // XXX unused
  int is_stackcall;
  int n_max_areg;
  int n_max_vreg;
} CUMULATIVE_ARGS;

#define INIT_CUMULATIVE_ARGS(CUM, FNTYPE, LIBNAME, FNDECL, N_NAMED_ARGS) \
  wasm32_init_cumulative_args(&(CUM), FNTYPE, LIBNAME, FNDECL, N_NAMED_ARGS)

#define EXIT_IGNORE_STACK 1

#define FUNCTION_PROFILER(STREAM, LABELNO)	        \
  do { } while(0)

/* Length in units of the trampoline for entering a nested function.  */
#define TRAMPOLINE_SIZE 32
/* Alignment required for a trampoline in bits.  */
#define TRAMPOLINE_ALIGNMENT 128

#define MAX_REGS_PER_ADDRESS 5 /* XXX */

#define BRANCH_COST(speed_p, predictable_p) wasm32_branch_cost(speed_p, predictable_p)

/* Nonzero if access to memory by bytes is slow and undesirable.  */
#define SLOW_BYTE_ACCESS 0

#define NO_FUNCTION_CSE true
//#define LOGICAL_OP_NON_SHORT_CIRCUIT 1

/* This is slightly complicated: we have to create the .space.code.%s
   and .wasm.code.%S sections at the same time, or the linker will see
   .wasm.code.A, then .wasm.code.B, then .space.code.B, then
   .space.code.A, and our index space will be disordered.  */
#define TEXT_SECTION_ASM_OP "\ttext_section"
#define DATA_SECTION_ASM_OP "\t.data"
#define READONLY_DATA_SECTION_ASM_OP "\t.section .rodata"
#define BSS_SECTION_ASM_OP "\t.data"
#define TLS_COMMON_ASM_OP "\t.data" /* XXX */
#define TYPE_ASM_OP	"\t.type\t"
#define SIZE_ASM_OP	"\t.size\t"
#define INIT_ARRAY_SECTION_ASM_OP
#define FINI_ARRAY_SECTION_ASM_OP

/* hand-crafted version of "call". It's a bit of a hack to use #FUNC
   as the unique identifier since we don't have %=... (there's __COUNTER__) */
/* XXX is this still needed? */
#define CRT_CALL_STATIC_FUNCTION(SECTION_OP, FUNC)		\
   asm (SECTION_OP "\n\t"					\
	"r1 = 0;\n\tr2 = 0;\n\tsp = ((sp|0)-16)|0;\n\t.labeldef_internal .LI" #FUNC "\n\tHEAP32[(sp+12)>>2] = r1;\n\tHEAP32[(sp+4)>>2] = fp;\n\tHEAP32[(sp+8)>>2] = r2;\n\tHEAP32[sp>>2] = ($\n\t.codetextlabel .LI" #FUNC "B\n\t);\n\trp = foreign_indcall($\n\t.codetextlabel " #FUNC "\n\t>>4, sp|0)|0;\n\tif (rp&3) {\n\t\tpc = HEAP32[sp>>2]>>4;\n\t\tbreak mainloop;\n\t}\n\t.labeldef_internal .LI" #FUNC "B\n\tsp = ((sp|0)+16)|0;\n" \
        TEXT_SECTION_ASM_OP);

#define ASM_COMMENT_START ";;"
#define ASM_APP_ON "#APP\n"
#define ASM_APP_OFF "#APP\n"

#undef ASM_OUTPUT_ALIGNED_DECL_COMMON
#define ASM_OUTPUT_ALIGNED_DECL_COMMON wasm32_output_aligned_decl_common

#define ASM_OUTPUT_LOCAL wasm32_output_local
#define ASM_OUTPUT_LABEL(stream, name) \
  wasm32_output_label(stream, name)
#define ASM_OUTPUT_INTERNAL_LABEL(stream, name) \
  wasm32_output_internal_label(stream, name)
#define ASM_WEAKEN_LABEL(stream, name) wasm32_weaken_label (stream, name)
#define MAKE_DECL_ONE_ONLY(DECL) (DECL_WEAK (DECL) = 1)
#define ASM_OUTPUT_LABELREF(stream, name) \
  wasm32_output_labelref(stream, name)
#define ASM_OUTPUT_SYMBOL_REF(stream, rtl) \
  wasm32_output_symbol_ref(stream, rtl)
#define ASM_OUTPUT_LABEL_REF(stream, buf) \
  wasm32_output_label_ref(stream, buf)
#define ASM_OUTPUT_DEBUG_LABEL(stream, prefix, num)	\
  wasm32_output_debug_label(stream, prefix, num)
#define ASM_OUTPUT_DEF(FILE, ALIAS, NAME) wasm32_output_def (FILE, ALIAS, NAME)

/* This is how to store into the string LABEL
   the symbol_ref name of an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.
   This is suitable for output with `assemble_name'.

   For most svr4 systems, the convention is that any symbol which begins
   with a period is not put into the linker symbol table by the assembler.  */

#undef  ASM_GENERATE_INTERNAL_LABEL
#define ASM_GENERATE_INTERNAL_LABEL(LABEL, PREFIX, NUM)		\
  do								\
    {								\
      char *__p;						\
      (LABEL)[0] = '.';						\
      __p = stpcpy (&(LABEL)[1], PREFIX);			\
      sprint_ul (__p, (unsigned long) (NUM));			\
    }								\
  while (0)

#define OBJECT_FORMAT_ELF 1

#define REGISTER_NAMES {                               \
    "fp",  /* frame pointer. must not be eliminated */ \
    "dpc", /* function-relative PC */		       \
    "sp",  /* stack pointer */			       \
    "rv",  /* return value; per-thread */	       \
    "a0",  /* argument registers; per-thread */        \
    "a1",  /* argument registers; per-thread */        \
    "a2",  /* argument registers; per-thread */        \
    "a3",  /* argument registers; per-thread */        \
    "r0",  /* general registers */		       \
    "r1",					       \
    "r2",					       \
    "r3",					       \
    "r4",  /* general registers */		       \
    "r5",					       \
    "r6",					       \
    "r7",					       \
    "i0",  /* integer; no difference to r* now */      \
    "i1",					       \
    "i2",					       \
    "i3",					       \
    "i4",  /* integer; no difference to r* now */      \
    "i5",					       \
    "i6",					       \
    "i7",					       \
    "f0",  /* floating-point registers */	       \
    "f1",					       \
    "f2",					       \
    "f3",					       \
    "f4",  /* floating-point registers */	       \
    "f5",					       \
    "f6",					       \
    "f7",					       \
    "ap",  /* argument pointer; eliminated */	       \
    "tp",  /* thread pointer; per-thread */	       \
    "pc0", /* function PC */                           \
    "rpc", /* return PC */                             \
}

#define PRINT_OPERAND(stream,x,code) wasm32_print_operand(stream, x, code)
#define PRINT_OPERAND_PUNCT_VALID_P(code) wasm32_print_operand_punct_valid_p(code)
#define PRINT_OPERAND_ADDRESS(stream,x) wasm32_print_operand_address(stream, x)

/* XXX TARGET_ASM_EMIT_UNWIND_LABEL */

#define EH_TABLES_CAN_BE_READ_ONLY 0


#undef ASM_OUTPUT_SKIP
#define ASM_OUTPUT_SKIP wasm32_output_skip
#define ASM_NO_SKIP_IN_TEXT 1
/* Align output to a power of two. XXX text/output_align_with_nop */
#define ASM_OUTPUT_ALIGN(STREAM, POWER)			\
  do							\
    {							\
      fprintf (STREAM, "\t.p2align %d\n", POWER);         \
    }							\
  while (0)

#define DWARF2_DEBUGGING_INFO 1
#define DWARF2_FRAME_INFO 1
#define DWARF2_ASM_LINE_DEBUG_INFO 0
#undef HAVE_GAS_CFI_PERSONALITY_DIRECTIVE
#define HAVE_GAS_CFI_PERSONALITY_DIRECTIVE 0

/* Specify the machine mode that this machine uses for the index in
   the tablejump instruction.  XXX, but I think this is fine. */
#define CASE_VECTOR_MODE Pmode

/* Define if operations between registers always perform the operation
 * on the full register even if a narrower mode is specified. */
#define WORD_REGISTER_OPERATIONS 1

/* Max number of bytes we can move from memory to memory
   in one reasonably fast instruction. XXX SIMD */
#define MOVE_MAX 4
/* The machine modes of pointers and functions */
#define Pmode  SImode
#define FUNCTION_MODE  QImode /* XXX */
/* The maximum number of instructions that is beneficial to
   conditionally execute. */
#define MAX_CONDITIONAL_EXECUTE wasm32_max_conditional_execute ()




/* Register classes.  */
enum reg_class
{
  NO_REGS,
  PC_REGS,
  SP_REGS,

  FLOAT_REGS,
  GENERAL_REGS,
  THREAD_REGS,
  ALL_REGS,
  LIM_REG_CLASSES
};

#define FUNCTION_ARG_REGNO_P(REGNO) wasm32_function_arg_regno_p(REGNO)

//#define DEFAULT_PCC_STRUCT_RETURN
#define TARGET_OMIT_STRUCT_RETURN_REG 1
#define TARGET_PROMOTE_MODE(m,unsignedp,type) wasm32_promote_mode(&(m),&(unsignedp),type)


/* XXX PROMOTE_FUNCTION_RETURN */
#define TARGET_EXPAND_BUILTIN_VA_START wasm32_expand_builtin_va_start

//#define TARGET_GET_DRAP_RTX wasm32_get_drap_rtx

/* This macro produces the initial definition of a function.  The
 * wasm32 version produces lots of extra code. */

#undef ASM_DECLARE_FUNCTION_NAME
#define ASM_DECLARE_FUNCTION_NAME(FILE,NAME,DECL) \
  wasm32_start_function(FILE,NAME,DECL);

/* This macro closes up a function definition for the assembler.  The
 * wasm32 version produces lots of extra code. */

#undef ASM_DECLARE_FUNCTION_SIZE
#define ASM_DECLARE_FUNCTION_SIZE(FILE,NAME,DECL) \
  wasm32_end_function(FILE,NAME,DECL)

#ifdef HAVE_GAS_GNU_UNIQUE_OBJECT
#define USE_GNU_UNIQUE_OBJECT flag_gnu_unique
#else
#define USE_GNU_UNIQUE_OBJECT 0
#endif

#define ASM_DECLARE_OBJECT_NAME(FILE, NAME, DECL)			\
  do									\
    {									\
      HOST_WIDE_INT size;						\
									\
      /* For template static data member instantiations or		\
	 inline fn local statics and their guard variables, use		\
	 gnu_unique_object so that they will be combined even under	\
	 RTLD_LOCAL.  Don't use gnu_unique_object for typeinfo,		\
	 vtables and other read-only artificial decls.  */		\
      if (USE_GNU_UNIQUE_OBJECT && DECL_ONE_ONLY (DECL)			\
	  && (!DECL_ARTIFICIAL (DECL) || !TREE_READONLY (DECL)))	\
	ASM_OUTPUT_TYPE_DIRECTIVE (FILE, NAME, "gnu_unique_object");	\
      else								\
	ASM_OUTPUT_TYPE_DIRECTIVE (FILE, NAME, "object");		\
									\
      size_directive_output = 0;					\
      if (!flag_inhibit_size_directive					\
	  && (DECL) && DECL_SIZE (DECL))				\
	{								\
	  size_directive_output = 1;					\
	  size = tree_to_uhwi (DECL_SIZE_UNIT (DECL));			\
	  ASM_OUTPUT_SIZE_DIRECTIVE (FILE, NAME, size);			\
	}								\
									\
      ASM_OUTPUT_LABEL (FILE, NAME);					\
    }									\
  while (0)

#define ASM_FINISH_DECLARE_OBJECT(FILE, DECL, TOP_LEVEL, AT_END)	\
  do								\
    {								\
      const char *name = XSTR (XEXP (DECL_RTL (DECL), 0), 0);	\
      HOST_WIDE_INT size;					\
								\
      if (!flag_inhibit_size_directive				\
	  && DECL_SIZE (DECL)					\
	  && ! AT_END && TOP_LEVEL				\
	  && DECL_INITIAL (DECL) == error_mark_node		\
	  && !size_directive_output)				\
	{							\
	  size_directive_output = 1;				\
	  size = tree_to_uhwi (DECL_SIZE_UNIT (DECL));		\
	  ASM_OUTPUT_SIZE_DIRECTIVE (FILE, name, size);		\
	}							\
    }								\
  while (0)

#define TARGET_UNWIND_TABLES_DEFAULT 1

#define THREAD_MODEL_SPEC "single"


#undef TARGET_ASM_FILE_START
#define TARGET_ASM_FILE_START wasm32_file_start

#if defined HOST_WIDE_INT
struct GTY(()) machine_function
{
  tree funtype;
};
#endif

#define SUPPORTS_WEAK 1
#define USE_INITFINI_ARRAY

#define TARGET_HAVE_TLS 1
#define TARGET_HAVE_NAMED_SECTIONS 1

#define TARGET_HARD_REGNO_MODE_OK wasm32_hard_regno_mode_ok

#define TARGET_ASM_CONSTRUCTOR default_elf_init_array_asm_out_constructor
#define TARGET_ASM_DESTRUCTOR default_elf_fini_array_asm_out_destructor
#define TARGET_ASM_NAMED_SECTION wasm32_asm_named_section

#define TARGET_ASM_INTEGER wasm32_assemble_integer

#define TYPE_OPERAND_FMT	"@%s"
#define TYPE_ASM_OP	"\t.type\t"
#define SIZE_ASM_OP	"\t.size\t"
