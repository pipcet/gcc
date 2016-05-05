;;- Machine description for the asm.js target for GCC
;;  Copyright (C) 1991-2015 Free Software Foundation, Inc.
;;  Copyright (C) 2016 Pip Cet <pipcet@gmail.com>

(define_c_enum "unspecv" [
  UNSPECV_BLOCKAGE
  UNSPECV_PROBE_STACK_RANGE

  UNSPECV_FLUSHW
  UNSPECV_SAVEW

  UNSPECV_FLUSH

  UNSPECV_LDSTUB
  UNSPECV_SWAP
  UNSPECV_CAS

  UNSPECV_LDFSR
  UNSPECV_STFSR

  UNSPECV_EH_RETURN
  UNSPECV_NONLOCAL_GOTO
  UNSPECV_NONLOCAL_GOTO_RECEIVER
  UNSPECV_BUILTIN_LONGJMP
  UNSPECV_THUNK_GOTO
])

(define_c_enum "unspec" [
  UNSPEC_TRAMPOLINE_INIT
])

(define_constants
  [(FP_REG        0)
   (DPC_REG       1)
   (SP_REG        2)
   (RV_REG        3)
   (A0_REG        4)
   (A1_REG        5)
   (A2_REG        6)
   (A3_REG        7)
   (R0_REG        8)
   (R1_REG        9)
   (R2_REG       10)
   (R3_REG       11)
   (R4_REG       12)
   (R5_REG       13)
   (R6_REG       14)
   (R7_REG       15)
   (I0_REG       16)
   (I1_REG       17)
   (I2_REG       18)
   (I3_REG       19)
   (I4_REG       20)
   (I5_REG       21)
   (I6_REG       22)
   (I7_REG       23)
   (F0_REG       24)
   (F1_REG       25)
   (F2_REG       26)
   (F3_REG       27)
   (F4_REG       28)
   (F5_REG       29)
   (F6_REG       30)
   (F7_REG       31)
   (AP_REG       32)
   (TP_REG       33)
   (PC0_REG      34)
   (RPC_REG      35)])

(define_register_constraint "t" "THREAD_REGS"
    "registers shared between all functions in a thread")

(define_attr "predicable" "no,yes" (const_string "yes"))

;;(define_attr "conds" "set,unconditional"
;;  (const_string "unconditional"))

;; (define_insn "*save_label"
;;   [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
;;         (label_ref (match_operand 1)))]
;;    ""
;;    "%O0 = %l1;")

(define_insn "*assignqi"
  [(match_operator 2 "set_operator"
      [(match_operand:QI 0 "nonimmediate_operand" "=rmt")
       (match_operand:QI 1 "general_operand" "rmit")])]
  ""
  "%O2;")

(define_insn "*assignhi"
  [(match_operator 2 "set_operator"
      [(match_operand:HI 0 "nonimmediate_operand" "=rmt")
       (match_operand:HI 1 "general_operand" "rmit")])]
  ""
  "%O2;")

(define_insn "*assignsil"
  [(match_operator 2 "set_operator"
      [(match_operand:SI 0 "nonimmediate_operand" "=rm")
       (label_ref (match_operand 1))])]
  ""
  "%O0 = $\n\t.codetextlabel %l1\n\t;")

(define_insn "*assignsi"
  [(match_operator 2 "set_operator"
      [(match_operand:SI 0 "nonimmediate_operand" "=rmt")
       (match_operand:SI 1 "general_operand" "rmit")])]
  ""
  "%O2;")

(define_insn "*assignsf"
  [(match_operator 2 "set_operator"
      [(match_operand:SF 0 "nonimmediate_operand" "=rm")
       (match_operand:SF 1 "general_operand" "rmi")])]
  ""
  "%O2;")

(define_insn "*assigndf"
  [(match_operator 2 "set_operator"
      [(match_operand:DF 0 "nonimmediate_operand" "=rm")
       (match_operand:DF 1 "general_operand" "rmi")])]
  ""
  "%O2;")

(define_insn "*assignsi_unop"
  [(match_operator 3 "set_operator"
     [(match_operand:SI 0 "nonimmediate_operand" "=rm")
      (match_operator 1 "unary_operator"
         [(match_operand 2 "general_operand" "rmi")])])]
   ""
   "%O3;")

(define_insn "*assigndf_unop"
  [(match_operator 3 "set_operator"
     [(match_operand:DF 0 "nonimmediate_operand" "=rm")
      (match_operator 1 "unary_operator"
         [(match_operand 2 "general_operand" "rmi")])])]
   ""
   "%O3;")

(define_insn "*assignsi_binop"
  [(match_operator 4 "set_operator"
     [(match_operand:SI 0 "nonimmediate_operand" "=rm")
      (match_operator 1 "binary_operator"
         [(match_operand:SI 2 "general_operand" "rmi")
          (match_operand:SI 3 "general_operand" "rmi")])])]
   ""
   "%O4;")

(define_insn "*assigndf_binop"
  [(match_operator 4 "set_operator"
     [(match_operand:DF 0 "nonimmediate_operand" "=rm")
      (match_operator 1 "binary_operator"
         [(match_operand:DF 2 "general_operand" "rmi")
          (match_operand:DF 3 "general_operand" "rmi")])])]
   ""
   "%O4;")

(define_insn "*zero_extendqisi2"
  [(match_operator 2 "set_operator"
      [(match_operand:SI 0 "nonimmediate_operand" "=rm")
       (zero_extend:SI (match_operand:QI 1 "memory_operand" "m"))])]
  ""
  "%O2;")

(define_insn "*zero_extendqisi2"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (zero_extend:SI (match_operand:QI 1 "general_operand" "rmi")))]
  ""
  "%O0 = (%O1)&255;")

(define_expand "zero_extendqisi2"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (zero_extend:SI (match_operand:QI 1 "general_operand" "rmi")))]
  ""
  "")

(define_insn "*zero_extendhisi2"
  [(match_operator 2 "set_operator"
      [(match_operand:SI 0 "nonimmediate_operand" "=rm")
       (zero_extend:SI (match_operand:HI 1 "memory_operand" "m"))])]
  ""
  "%O2;")

(define_insn "*zero_extendhisi2"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (zero_extend:SI (subreg:HI (match_operand:HI 1 "register_operand" "r") 0)))]
  ""
  "%O0 = (%O1)&65535;")

(define_insn "*zero_extendhisi2"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (zero_extend:SI (match_operand:HI 1 "general_operand" "rmi")))]
  ""
  "%O0 = (%O1)&65535;")

(define_expand "zero_extendhisi2"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (zero_extend:SI (match_operand:HI 1 "general_operand" "rmi")))]
  ""
  "")

(define_insn "*truncdfsf"
  [(match_operator 2 "set_operator"
      [(match_operand:SF 0 "nonimmediate_operand" "=rm")
       (float_truncate:SF (match_operand:DF 1 "general_operand" "rmi"))])]
  ""
  "%O2;")

(define_insn "*extendsfdf"
  [(match_operator 2 "set_operator"
      [(match_operand:DF 0 "nonimmediate_operand" "=rm")
       (float_extend:DF (match_operand:SF 1 "general_operand" "rmi"))])]
  ""
  "%O2;")

(define_insn "*movsicc"
   [(match_operator 5 "set_operator"
       [(match_operand:SI 0)
        (if_then_else:SI  (match_operator 6 "ordered_comparison_operator"
                          [(match_operand:SI 1 "general_operand" "rmi")
                           (match_operand:SI 2 "general_operand" "rmi")])
                         (match_operand:SI 3 "general_operand" "rmi")
                         (match_operand:SI 4 "general_operand" "rmi"))])]
   ""
   "%O5;")

;; I attempted to open-code this, but failed.  In theory once
;; open-coded we should be able to combine a call and a subsequent
;; jump in the case that our stack pointer adjustment can be moved
;; past the jump.

(define_insn "*call"
   [(call (mem:QI (match_operand:SI 0 "general_operand" "i,r!m!t!"))
          (match_operand:SI 1 "general_operand" "rmi,rmi"))]
      ""
      "@
      {\n\t\tdpc = $\n\t\t.dpc .LI%=\n\t\t\t;\n\t\trp = f_%O0\n\t(0, sp|0, r0|0, r1|0, (pc0+dpc)|0, %O0>>4)|0;\n\t\tif (rp&3) {\n\t\t\tbreak mainloop;\n\t\t}\n\t}\n\t.labeldef_internal .LI%=
      {\n\t\tdpc = $\n\t\t.dpc .LI%=\n\t\t\t;\n\t\trp = indcall(0, sp|0, r0|0, r1|0, (pc0+dpc)|0, %O0>>4)|0;\n\t\tif (rp&3) {\n\t\t\tbreak mainloop;\n\t\t}\n\t}\n\t.labeldef_internal .LI%=")

(define_insn "*call_value"
   [(set (reg:SI RV_REG)
      (call (mem:QI (match_operand:SI 0 "general_operand" "i,r!m!t!"))
            (match_operand:SI 1 "general_operand" "rmi,rmi")))]
      ""
      "@
      {\n\t\tdpc = $\n\t\t\t.dpc .LI%=\n\t;\n\t\trp = f_%O0\n\t(0, sp|0, r0|0, r1|0, (pc0+dpc)|0, %O0>>4)|0;\n\t\tif (rp&3) {\n\t\t\tbreak mainloop;\n\t\t}\n\t}\n\t.labeldef_internal .LI%=
      {\n\t\tdpc = $\n\t\t\t.dpc .LI%=\n\t\t\t;\n\t\trp = indcall(0, sp|0, r0|0, r1|0, (pc0+dpc)|0, %O0>>4)|0;\n\t\tif (rp&3) {\n\t\t\tbreak mainloop;\n\t\t}\n\t}\n\t.labeldef_internal .LI%=")

(define_expand "call"
  [(parallel [(call (match_operand 0)
         (match_operand:SI 1 "general_operand" "rmi"))
   (set (reg:SI SP_REG) (plus:SI (reg:SI SP_REG) (const_int -4)))
   (set (pc) (match_dup 0))])]
  ""
  {
    asmjs_expand_call (NULL_RTX, operands[0], operands[1]);
    DONE;
  }
  [(set_attr "predicable" "no")])

(define_expand "call_value"
  [(parallel [(set (match_operand:SI 0)
                  (call (match_operand 1)
                        (match_operand:SI 2 "general_operand" "rmi")))
   (set (reg:SI SP_REG) (plus:SI (reg:SI SP_REG) (const_int -4)))
   (set (pc) (match_dup 0))])]
  ""
  {
    asmjs_expand_call (operands[0], operands[1], operands[2]);
    DONE;
  }
  [(set_attr "predicable" "no")])

(define_insn "*jump"
  [(set (pc) (label_ref (match_operand 0)))]
  ""
  "dpc = ($\n\t.dpc %l0\n\t);\n\t.cont_or_break %l0"
  [(set_attr "predicable" "no")])

(define_insn "*jump"
  [(set (pc) (match_operand:SI 0 "general_operand" "rmi"))]
  ""
  "/* indirect jump */\n\tdpc = (((%O0|0)>>4)-(pc0|0))|0;\n\tbreak;"
  [(set_attr "predicable" "no")])

;;(define_expand "jump"
;;   [(set (pc) (label_ref (match_operand 0)))]
;;   ""
;;   "")

;; (define_expand "jump"
;;   [(set (reg:SI PC_REG) (label_ref (match_operand 0)))
;;    (set (pc) (reg:SI PC_REG))]
;;   ""
;;   "")

(define_expand "jump"
  [
;;   (set (pc) (match_operand:SI 0 "general_operand" "rmi"))
   (set (pc) (label_ref (match_operand:SI 0 "general_operand" "rmi")))
  ]
  "1" ;; avoid zero-size array insn_conditions
  "")

(define_expand "indirect_jump"
  [
   (set (pc) (match_operand:SI 0 "general_operand" "rmi"))]
  "1" ;; avoid zero-size array insn_conditions
  "")

(define_predicate "set_operator"
  (match_code "set"))

(define_predicate "binary_operator"
  (ior (match_code "eq")
       (match_code "ne")
       (match_code "lt")
       (match_code "gt")
       (match_code "le")
       (match_code "ge")
       (match_code "ltu")
       (match_code "gtu")
       (match_code "leu")
       (match_code "geu")
       (match_code "and")
       (match_code "ior")
       (match_code "mult")
       (match_code "div")
       (match_code "udiv")
       (match_code "mod")
       (match_code "umod")
       (match_code "plus")
       (match_code "minus")
       (match_code "xor")
       (match_code "ashift")
       (match_code "ashiftrt")
       (match_code "lshiftrt")))

(define_predicate "unary_operator"
  (ior (match_code "not")
       (match_code "neg")
       (match_code "float")
       (match_code "fix")))

(define_insn "*cbranchsi4"
  [(match_operator 4 "set_operator"
     [(pc)
      (if_then_else
          (match_operator 0 "ordered_comparison_operator"
              [(match_operand:SI 1 "general_operand" "rmi")
               (match_operand:SI 2 "general_operand" "rmi")])
              (label_ref (match_operand 3))
              (pc))])]
  "1"
  "if (%O0) { dpc = ($\n\t.dpc %l3\n\t); $\n\t.cont_or_break %l3\n\t }"
  )

(define_expand "cbranchsi4"
  [(set (pc) (if_then_else
              (match_operator 0 "ordered_comparison_operator"
                 [(match_operand:SI 1 "general_operand" "rmi")
                  (match_operand:SI 2 "general_operand" "rmi")])
              (label_ref (match_operand 3))
              (pc)))]
  "1"
  "")

(define_insn "*cbranchdf4"
  [(match_operator 4 "set_operator"
     [(pc)
      (if_then_else
          (match_operator 0 "ordered_comparison_operator"
              [(match_operand:DF 1 "general_operand" "rmi")
               (match_operand:DF 2 "general_operand" "rmi")])
              (label_ref (match_operand 3))
              (pc))])]
  "1"
  "if (%O0) { dpc = ($\n\t.dpc %l3\n\t); $\n\t.cont_or_break %l3\n\t }"
  )

(define_expand "cbranchdf4"
  [(set (pc) (if_then_else
              (match_operator 0 "ordered_comparison_operator"
                 [(match_operand:DF 1 "general_operand" "rmi")
                  (match_operand:DF 2 "general_operand" "rmi")])
              (label_ref (match_operand 3))
              (pc)))]
  "1"
  "")

(define_expand "movqi"
  [(set (match_operand:QI 0 "nonimmediate_operand" "=rmt")
        (match_operand:QI 1 "general_operand" "rmit"))]
  ""
  "")

(define_expand "movhi"
  [(set (match_operand:HI 0 "nonimmediate_operand" "=rmt")
        (match_operand:HI 1 "general_operand" "rmit"))]
  ""
  "")

(define_expand "movsi"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rt,rmt")
        (match_operand:SI 1 "general_operand" "rmit,rit"))]
  ""
  "")

(define_expand "movsf"
  [(set (match_operand:SF 0 "nonimmediate_operand" "=rm")
        (match_operand:SF 1 "general_operand" "rmi"))]
  ""
  "")

(define_expand "movdf"
  [(set (match_operand:DF 0 "nonimmediate_operand" "=rm")
        (match_operand:DF 1 "general_operand" "rmi"))]
  ""
  "")

(define_expand "addsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (plus:SI (match_operand:SI 1 "general_operand" "rmi")
                 (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "subsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (minus:SI (match_operand:SI 1 "general_operand" "rmi")
                  (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "xorsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (xor:SI (match_operand:SI 1 "general_operand" "rmi")
                (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "iorsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (ior:SI (match_operand:SI 1 "general_operand" "rmi")
                (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "andsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (and:SI (match_operand:SI 1 "general_operand" "rmi")
                (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "notsi2"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (not:SI (match_operand:SI 1 "general_operand" "rmi")))]
  ""
  "")

(define_expand "one_cmplsi2"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (not:SI (match_operand:SI 1 "general_operand" "rmi")))]
  ""
  "")

(define_expand "mulsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (mult:SI (match_operand:SI 1 "general_operand" "rmi")
                 (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "divsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (div:SI (match_operand:SI 1 "general_operand" "rmi")
                (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "udivsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (udiv:SI (match_operand:SI 1 "general_operand" "rmi")
                (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "modsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (mod:SI (match_operand:SI 1 "general_operand" "rmi")
                (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "umodsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (umod:SI (match_operand:SI 1 "general_operand" "rmi")
                (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "ashlsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (ashift:SI (match_operand:SI 1 "general_operand" "rmi")
                   (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "ashrsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (ashiftrt:SI (match_operand:SI 1 "general_operand" "rmi")
                     (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "lshrsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (lshiftrt:SI (match_operand:SI 1 "general_operand" "rmi")
                     (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "eqsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (eq:SI (match_operand:SI 1 "general_operand" "rmi")
               (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "nesi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (ne:SI (match_operand:SI 1 "general_operand" "rmi")
               (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "gtsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (gt:SI (match_operand:SI 1 "general_operand" "rmi")
               (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "lesi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (le:SI (match_operand:SI 1 "general_operand" "rmi")
               (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "gesi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (ge:SI (match_operand:SI 1 "general_operand" "rmi")
               (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "ltsi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (lt:SI (match_operand:SI 1 "general_operand" "rmi")
               (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "gtusi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (gtu:SI (match_operand:SI 1 "general_operand" "rmi")
                (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "leusi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (leu:SI (match_operand:SI 1 "general_operand" "rmi")
                (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "geusi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (geu:SI (match_operand:SI 1 "general_operand" "rmi")
                (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "ltusi3"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (ltu:SI (match_operand:SI 1 "general_operand" "rmi")
                (match_operand:SI 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "adddf3"
  [(set (match_operand:DF 0 "nonimmediate_operand" "=rm")
        (plus:DF (match_operand:DF 1 "general_operand" "rmi")
                 (match_operand:DF 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "subdf3"
  [(set (match_operand:DF 0 "nonimmediate_operand" "=rm")
        (minus:DF (match_operand:DF 1 "general_operand" "rmi")
                  (match_operand:DF 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "muldf3"
  [(set (match_operand:DF 0 "nonimmediate_operand" "=rm")
        (mult:DF (match_operand:DF 1 "general_operand" "rmi")
                 (match_operand:DF 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "divdf3"
  [(set (match_operand:DF 0 "nonimmediate_operand" "=rm")
        (div:DF (match_operand:DF 1 "general_operand" "rmi")
                (match_operand:DF 2 "general_operand" "rmi")))]
  ""
  "")

(define_expand "floatsidf2"
  [(set (match_operand:DF 0 "nonimmediate_operand" "=rm")
        (float:DF (match_operand:SI 1 "general_operand" "rmi")))]
  ""
  "")

;; (define_expand "floatundidf"
;;   [(set (match_operand:DF 0 "nonimmediate_operand" "=rm")
;;         (float:DF (match_operand:DI 1 "general_operand" "mi")))]
;;   ""
;;   "")

(define_expand "fixdfsi"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (fix:SI (match_operand:DF 1 "general_operand" "rmi")))]
  ""
  "")

;; XXX
(define_expand "fixunsdfsi"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (fix:SI (match_operand:DF 1 "general_operand" "rmi")))]
  ""
  "")

(define_expand "fixdfsi2"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (fix:SI (match_operand:DF 1 "general_operand" "rmi")))]
  ""
  "")

;; XXX
(define_expand "fixunsdfsi2"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (fix:SI (match_operand:DF 1 "general_operand" "rmi")))]
  ""
  "")

(define_expand "fix_truncdfsi"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (fix:SI (match_operand:DF 1 "general_operand" "rmi")))]
  ""
  "")

(define_expand "extendsfdf2"
  [(set (match_operand:DF 0 "nonimmediate_operand" "=rm")
        (float_extend:DF (match_operand:SF 1 "general_operand" "rmi")))]
  ""
  "")

(define_expand "truncdfsf2"
  [(set (match_operand:SF 0 "nonimmediate_operand" "=rm")
        (float_truncate:SF (match_operand:DF 1 "general_operand" "rmi")))]
  ""
  "")

;; XXX
(define_expand "fixuns_truncdfsi"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (fix:SI (match_operand:DF 1 "general_operand" "rmi")))]
  ""
  "")

(define_expand "fix_truncdfsi2"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (fix:SI (match_operand:DF 1 "general_operand" "rmi")))]
  ""
  "")

(define_expand "movsicc"
   [(set (match_operand:SI 0)
         (if_then_else:SI (match_operand 1 "ordered_comparison_operator")
                          (match_operand:SI 2)
                          (match_operand:SI 3)))]
   "my_use_cmov"
   "")

;; XXX
(define_expand "fixuns_truncdfsi2"
  [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
        (fix:SI (match_operand:DF 1 "general_operand" "rmi")))]
  ""
  "")

;; (define_expand "ffssi2"
;;   [(set (match_operand:SI 0 "nonimmediate_operand" "=rm)
;;         (ffs:SI (match_operand:SI 1 "general_operand" "rmi")))]
;;   ""
;;   "")

(define_expand "prologue"
  [(set (reg:SI SP_REG) (plus:SI (reg:SI SP_REG) (const_int -4)))
   (set (mem:SI (reg:SI SP_REG)) (reg:SI FP_REG))
   (set (reg:SI SP_REG) (plus:SI (reg:SI SP_REG) (const_int -4)))
   (set (mem:SI (reg:SI SP_REG)) (reg:SI AP_REG))
   (set (reg:SI AP_REG) (plus:SI (reg:SI SP_REG) (const_int 12)))
   (set (reg:SI SP_REG) (plus:SI (reg:SI SP_REG) (const_int -4)))
   (set (reg:SI FP_REG) (reg:SI SP_REG))]
  ""
  {
    asmjs_expand_prologue ();
    DONE;
  })

(define_insn "*return"
  [(return)]
  ""
  {
    return asmjs_expand_ret_insn();
  })

(define_expand "epilogue"
  [(set (reg:SI SP_REG) (plus:SI (reg:SI SP_REG) (const_int 4)))
   (set (reg:SI AP_REG) (mem:SI (reg:SI SP_REG)))
   (set (reg:SI SP_REG) (plus:SI (reg:SI SP_REG) (const_int 4)))
   (set (reg:SI FP_REG) (mem:SI (reg:SI SP_REG)))
   (set (reg:SI SP_REG) (plus:SI (reg:SI SP_REG) (const_int 4)))
   (return)]
  ""
  {
    asmjs_expand_epilogue ();
    DONE;
  })

(define_insn "nop"
  [(const_int 0)]
  ""
  "/* nop */;")

(define_cond_exec
   [(match_operator 2 "ordered_comparison_operator"
      [(match_operand:SI 0 "general_operand" "ri")
       (match_operand:SI 1 "general_operand" "ri")])]
   ""
   "%C2")

(define_cond_exec
   [(match_operator 2 "ordered_comparison_operator"
      [(match_operand:DF 0 "general_operand" "rmi")
       (match_operand:DF 1 "general_operand" "rmi")])]
   ""
   "%C2")

;; Special insn to flush register windows.

(define_insn "flush_register_windows"
  [(unspec_volatile [(const_int 0)] UNSPECV_FLUSHW)]
  ""
  ".flush")

;; (define_insn "eh_return"
;;    [(unspec_volatile [(match_operand:SI 0 "register_operand" "r")]
;;      UNSPECV_EH_RETURN)]
;;    ""
;;    "/* eh_return */\n\trp = fp|1;\n\tpc = $\n\t.codetextlabel .LI%=\n\t>>4;\n\tbreak mainloop;\n\t.labeldef_internal .LI%=\n\tr3 = %O0|0;\n\tr0 = (fp|0)+16|0;\n\ta0 = HEAP32[r0>>2]|0;\n\tr0 = (fp|0)+20|0;\n\ta1 = HEAP32[r0>>2]|0;\n\tr0 = (fp|0)+24|0;\n\ta2 = HEAP32[r0>>2]|0;\n\tr0 = (fp|0)+28|0;\n\tif (0) a3 = HEAP32[r0>>2]|0;\n\tr0 = HEAP32[fp+12>>2]|0;\n\tr1 = ((fp|0) + (r0|0))|0;\n\tr2 = HEAP32[r1+4>>2]|0;\n\tHEAP32[r2+4>>2] = r3|0;\n\tdebug2('eh_return: r0 is ' + r0.toString(16));\n\tdebug2('eh_return: r2 is ' + r2.toString(16));\n\tdebug2('eh_return: r3 is ' + r3.toString(16));\n\tdebug2('eh_return: r1 is ' + r1.toString(16));\n\tdebug2('eh_return: a0 is ' + a0.toString(16));\n\tdebug2('eh_return: a1 is ' + a1.toString(16));\n\tdebug2('eh_return: a2 is ' + a2.toString(16));\n\tdebug2('eh_return: a3 is ' + a3.toString(16));\n\tdebug2('eh_return: fp is ' + fp.toString(16));\n\tdebug2('eh_return: sp is ' + sp.toString(16));\n\t")

;; This is tricky enough that we delegate it to a JavaScript function, so we end up doing all of the following:
;;  1. flush
;;  2. restore a0, a1, a2, a3
;;  3. restore fp
;;  4. store %O0 in the PC slot belonging to fp
;;  5. return fp|3

(define_insn "eh_return"
   [(unspec_volatile [(match_operand:SI 0 "register_operand" "r")]
     UNSPECV_EH_RETURN)]
   ""
   "/* eh_return */\n\t.flush\n\treturn foreign_eh_return(fp|0, sp|0, %O0|0)|0;")

(define_insn "*nonlocal_goto"
  [(unspec_volatile [(match_operand:SI 0 "register_operand" "r")
                     (match_operand:SI 1 "register_operand" "r")
                     (match_operand:SI 2 "register_operand" "r")
                     (match_operand:SI 3 "register_operand" "r")]
    UNSPECV_NONLOCAL_GOTO)]
  ""
  "/* nonlocal_goto */\n\t.flush\n\tHEAP32[(HEAP32[fp>>2]|0)+8>>2] = %O1;\n\tHEAP32[(HEAP32[fp>>2]|0)+12>>2] = %O0;\n\trv = %O3; /* unused %O2 */\n\tbreak mainloop;")

;; (define_insn "*nonlocal_goto"
;;   [(unspec_volatile [(match_operand 0)
;;                      (match_operand:SI 2 "general_operand" "rm")
;;                      (match_operand:SI 3 "general_operand" "rm")]
;;     UNSPECV_NONLOCAL_GOTO)
;;     (set (pc) (label_ref (match_operand 1)))]
;;   ""
;;   "/* nonlocal_goto */\n\trp = fp|1;\n\tpc = $\n\t.codetextlabel .LI%=\n\t>>4;\n\tbreak mainloop;\n\t.labeldef_internal .LI%=\n\tHEAP32[fp+HEAP32[fp+12>>2]>>2] = %O1;\n\tHEAP32[fp+HEAP32[fp+12>>2]+4>>2] = %O0;\n\trv = %O3; /* unused %O2 */")

;; (define_insn "nonlocal_goto"
;;   [(unspec_volatile [(match_operand 0)
;;                      (match_operand 2)
;;                      (match_operand 3)]
;;     UNSPECV_NONLOCAL_GOTO)
;;     (set (pc) (match_operand 1))]
;;   ""
;;   "/* nonlocal_goto */\n\t.flush\n\tHEAP32[fp+HEAP32[fp+12>>2]>>2] = %O1;\n\tHEAP32[fp+HEAP32[fp+12>>2]+4>>2] = %O0;\n\trv = %O3; /* unused %O2 */")

(define_expand "builtin_longjmp"
   [(set (pc) (unspec_volatile [(match_operand 0)]
               UNSPECV_BUILTIN_LONGJMP))]
  ""
  "emit_jump_insn (gen_builtin_longjmp_insn (operands[0]));
   emit_barrier ();
   DONE;"
  )

(define_insn "builtin_longjmp_insn"
   [(set (pc) (unspec_volatile [(match_operand 0)]
               UNSPECV_BUILTIN_LONGJMP))]
   ""
   "/* longjmp */\n\t.flush\n\tfp = HEAP32[%O0>>2]|0;\n\tHEAP32[fp+8>>2] = HEAP32[%O0+4>>2]|0;\n\tHEAP32[fp+12>>2] = HEAP32[%O0+8>>2]|0;\n\treturn fp|3;")

(define_expand "nonlocal_goto"
  [(set (pc)
        (unspec_volatile [(match_operand 0 "") ;; fp (ignore)
                          (match_operand 1 "") ;; target
                          (match_operand 2 "") ;; sp
                          (match_operand 3 "") ;; ?
                          ] UNSPECV_NONLOCAL_GOTO))
   ]
  ""
  "emit_jump_insn (gen_nonlocal_goto_insn (operands[0], operands[1], operands[2], operands[3]));
   emit_barrier ();
   DONE;"
  )

(define_insn "nonlocal_goto_insn"
  [(set (pc)
        (unspec_volatile [(match_operand 0 "" "") ;; fp (ignore)
                          (match_operand 1 "" "") ;; target
                          (match_operand 2 "" "") ;; sp
                          (match_operand 3 "" "") ;; ?
                          ] UNSPECV_NONLOCAL_GOTO))
   ]
  ""
  "/* nonlocal_goto */\n\t.flush\n\tHEAP32[(HEAP32[fp>>2]|0)+8>>2] = %O1;\n\tHEAP32[(HEAP32[fp>>2]|0)+12>>2] = %O2\n\trv = %O3;\n\t/* unused %O0 */\n\treturn HEAP32[(HEAP32[fp>>2]|0)+8>>2]|3;")

(define_expand "nonlocal_goto_receiver"
  [(unspec_volatile [(const_int 0)] UNSPECV_NONLOCAL_GOTO_RECEIVER)]
  ""
  "emit_insn (gen_nonlocal_goto_receiver_insn ());
   DONE;")

(define_insn "nonlocal_goto_receiver_insn"
  [(unspec_volatile [(const_int 0)] UNSPECV_NONLOCAL_GOTO_RECEIVER)]
  ""
  "/* nonlocal_goto_receiver */")

(define_insn "thunk_goto_insn"
  [(set (pc)
        (unspec_volatile [(match_operand 0 "" "") ;; target
                          ] UNSPECV_THUNK_GOTO))
   ]
  ""
  "/* thunk_goto */\n\treturn f_$\n\t.codetextlabel %O0\n\t(0, sp+16|0, r0|0, r1|0, (dpc+pc0)|0, %O0>>4);")
