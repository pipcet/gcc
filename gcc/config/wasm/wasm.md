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

(define_insn "*assignqi"
  [(match_operator 2 "set_operator"
      [(match_operand:QI 0 "nonimmediate_operand" "=rmt")
       (match_operand:QI 1 "general_operand" "rmit")])]
  ""
  "%O2")

(define_insn "*assignhi"
  [(match_operator 2 "set_operator"
      [(match_operand:HI 0 "nonimmediate_operand" "=rmt")
       (match_operand:HI 1 "general_operand" "rmit")])]
  ""
  "%O2")

(define_insn "*assignsil"
  [(match_operator 2 "set_operator"
      [(match_operand:SI 0 "nonimmediate_operand" "=rm")
       (label_ref (match_operand 1))])]
  ""
  "(set %S0 \n\t.ncodetextlabel %l1\n\t)")

(define_insn "*assignsi"
  [(match_operator 2 "set_operator"
      [(match_operand:SI 0 "nonimmediate_operand" "=rmt")
       (match_operand:SI 1 "general_operand" "rmit")])]
  ""
  "%O2")

(define_insn "*assignsf"
  [(match_operator 2 "set_operator"
      [(match_operand:SF 0 "nonimmediate_operand" "=rm")
       (match_operand:SF 1 "general_operand" "rmi")])]
  ""
  "%O2")

(define_insn "*assigndf"
  [(match_operator 2 "set_operator"
      [(match_operand:DF 0 "nonimmediate_operand" "=rm")
       (match_operand:DF 1 "general_operand" "rmi")])]
  ""
  "%O2")

(define_insn "*assignsi_unop"
  [(match_operator 3 "set_operator"
     [(match_operand:SI 0 "nonimmediate_operand" "=rm")
      (match_operator 1 "unary_operator"
         [(match_operand 2 "general_operand" "rmi")])])]
   ""
   "%O3")

(define_insn "*assigndf_unop"
  [(match_operator 3 "set_operator"
     [(match_operand:DF 0 "nonimmediate_operand" "=rm")
      (match_operator 1 "unary_operator"
         [(match_operand 2 "general_operand" "rmi")])])]
   ""
   "%O3")

(define_insn "*assignsi_binop"
  [(match_operator 4 "set_operator"
     [(match_operand:SI 0 "nonimmediate_operand" "=rm")
      (match_operator 1 "binary_operator"
         [(match_operand:SI 2 "general_operand" "rmi")
          (match_operand:SI 3 "general_operand" "rmi")])])]
   ""
   "%O4")

(define_insn "*assigndf_binop"
  [(match_operator 4 "set_operator"
     [(match_operand:DF 0 "nonimmediate_operand" "=rm")
      (match_operator 1 "binary_operator"
         [(match_operand:DF 2 "general_operand" "rmi")
          (match_operand:DF 3 "general_operand" "rmi")])])]
   ""
   "%O4")

;; (define_insn "*zero_extendqisi2"
;;   [(match_operator 2 "set_operator"
;;       [(match_operand:SI 0 "nonimmediate_operand" "=rm")
;;        (zero_extend:SI (match_operand:QI 1 "memory_operand" "m"))])]
;;   ""
;;   "%O2")

;; (define_insn "*zero_extendqisi2"
;;   [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
;;         (zero_extend:SI (match_operand:QI 1 "general_operand" "rmi")))]
;;   ""
;;   "(set %S0 (zero_extend %O1))")

;; (define_expand "zero_extendqisi2"
;;   [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
;;         (zero_extend:SI (match_operand:QI 1 "general_operand" "rmi")))]
;;   ""
;;   "")

;; (define_insn "*zero_extendhisi2"
;;   [(match_operator 2 "set_operator"
;;       [(match_operand:SI 0 "nonimmediate_operand" "=rm")
;;        (zero_extend:SI (match_operand:HI 1 "memory_operand" "m"))])]
;;   ""
;;   "(set %S0 (zero_extend %O1))")

;; (define_insn "*zero_extendhisi2"
;;   [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
;;         (zero_extend:SI (subreg:HI (match_operand:HI 1 "register_operand" "r") 0)))]
;;   ""
;;   "(set %S0 (i32.and %O1 (i32.const 65535)))")

;; (define_insn "*zero_extendhisi2"
;;   [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
;;         (zero_extend:SI (match_operand:HI 1 "general_operand" "rmi")))]
;;   ""
;;   "(set %S0 (i32.and %O1 (i32.const 65535)))")

;; (define_expand "zero_extendhisi2"
;;   [(set (match_operand:SI 0 "nonimmediate_operand" "=rm")
;;         (zero_extend:SI (match_operand:HI 1 "general_operand" "rmi")))]
;;   ""
;;   "")

(define_insn "*truncdfsf"
  [(match_operator 2 "set_operator"
      [(match_operand:SF 0 "nonimmediate_operand" "=rm")
       (float_truncate:SF (match_operand:DF 1 "general_operand" "rmi"))])]
  ""
  "%O2")

(define_insn "*extendsfdf"
  [(match_operator 2 "set_operator"
      [(match_operand:DF 0 "nonimmediate_operand" "=rm")
       (float_extend:DF (match_operand:SF 1 "general_operand" "rmi"))])]
  ""
  "%O2")

;; (define_insn "*movsicc"
;;    [(match_operator 5 "set_operator"
;;        [(match_operand:SI 0)
;;         (if_then_else:SI  (match_operator 6 "ordered_comparison_operator"
;;                           [(match_operand:SI 1 "general_operand" "rmi")
;;                            (match_operand:SI 2 "general_operand" "rmi")])
;;                          (match_operand:SI 3 "general_operand" "rmi")
;;                          (match_operand:SI 4 "general_operand" "rmi"))])]
;;    ""
;;    "%O5")

;; I attempted to open-code this, but failed.  In theory once
;; open-coded we should be able to combine a call and a subsequent
;; jump in the case that our stack pointer adjustment can be moved
;; past the jump.

(define_insn "*call"
   [(call (mem:QI (match_operand:SI 0 "general_operand" "i,r!m!t!"))
          (match_operand:SI 1 "general_operand" "rmi,rmi"))]
      ""
      "@ 
      (set $dpc \n\t.dpc .LI%=\n\t)\n\t(set $rp (call $f_%L0\n\t (i32.const 0) (get_local $sp) (get_local $r0) (get_local $r1) (i32.add (get_local $pc0) \n\t.ncodetextlabel .LI%=\n\t) (i32.shr_u %O0 (i32.const 4))))\n\t(if (i32.and (get_local $rp) (i32.const 3)) (br $mainloop))\n\t.wasmtextlabeldef .LI%=
      (set $dpc \n\t.dpc .LI%=\n\t)\n\t(set $rp (call_import $indcall (i32.const 0) (get_local $sp) (get_local $r0) (get_local $r1) (i32.add (get_local $pc0) \n\t.ncodetextlabel .LI%=\n\t) (i32.shr_u %O0 (i32.const 4))))\n\t(if (i32.and (get_local $rp) (i32.const 3)) (br $mainloop))\n\t.wasmtextlabeldef .LI%=")

(define_insn "*call_value"
   [(set (reg:SI RV_REG)
      (call (mem:QI (match_operand:SI 0 "general_operand" "i,r!m!t!"))
            (match_operand:SI 1 "general_operand" "rmi,rmi")))]
      ""
      "@
      (set $dpc \n\t.dpc .LI%=\n\t)\n\t(set $rp (call $f_%L0\n\t (i32.const 0) (get_local $sp) (get_local $r0) (get_local $r1) (i32.add (get_local $pc0) \n\t.ncodetextlabel .LI%=\n\t) (i32.shr_u %O0 (i32.const 4))))\n\t(if (i32.and (get_local $rp) (i32.const 3)) (br $mainloop))\n\t.wasmtextlabeldef .LI%=
      (set $dpc \n\t.dpc .LI%=\n\t)\n\t(set $rp (call_import $indcall (i32.const 0) (get_local $sp) (get_local $r0) (get_local $r1) (i32.add (get_local $pc0) \n\t.ncodetextlabel .LI%=\n\t) (i32.shr_u %O0 (i32.const 4))))\n\t(if (i32.and (get_local $rp) (i32.const 3)) (br $mainloop))\n\t.wasmtextlabeldef .LI%=")

(define_expand "call"
  [(parallel [(call (match_operand 0)
         (match_operand:SI 1 "general_operand" "rmi"))
   (set (reg:SI SP_REG) (plus:SI (reg:SI SP_REG) (const_int -4)))
   (set (pc) (match_dup 0))])]
  ""
  {
    wasm_expand_call (NULL_RTX, operands[0], operands[1]);
    DONE;
  })

(define_expand "call_value"
  [(parallel [(set (match_operand:SI 0)
                  (call (match_operand 1)
                        (match_operand:SI 2 "general_operand" "rmi")))
   (set (reg:SI SP_REG) (plus:SI (reg:SI SP_REG) (const_int -4)))
   (set (pc) (match_dup 0))])]
  ""
  {
    wasm_expand_call (operands[0], operands[1], operands[2]);
    DONE;
  })

(define_insn "*jump"
  [(set (pc) (label_ref (match_operand 0)))]
  ""
  "(set $dpc $\n\t.dpc %l0\n\t) (jump)")

(define_insn "*jump"
  [(set (pc) (match_operand:SI 0 "general_operand" "rmi"))]
  ""
  "(set_local $dpc (i32.sub (i32.shr_u %O0 (i32.const 4)) (get_local $pc0)))\n\t(jump)")

(define_expand "jump"
  [
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
  "(if %O0 (then (set $dpc $\n\t.dpc %l3\n\t) (jump)))"
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
  "(if %O0 (then (set $dpc $\n\t.dpc %l3\n\t) (jump)))")

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

;; (define_expand "movsicc"
;;    [(set (match_operand:SI 0)
;;          (if_then_else:SI (match_operand 1 "ordered_comparison_operator")
;;                           (match_operand:SI 2)
;;                           (match_operand:SI 3)))]
;;    "my_use_cmov"
;;    "")

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
    wasm_expand_prologue ();
    DONE;
  })

(define_insn "*return"
  [(return)]
  ""
  {
    return wasm_expand_ret_insn();
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
    wasm_expand_epilogue ();
    DONE;
  })

(define_insn "nop"
  [(const_int 0)]
  ""
  "(nop)")

;; Special insn to flush register windows.

(define_insn "flush_register_windows"
  [(unspec_volatile [(const_int 0)] UNSPECV_FLUSHW)]
  ""
  ".flush")

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
   "(return (call_import $eh_return (get_local $fp) (get_local $sp) %O0))")

(define_insn "*nonlocal_goto"
  [(unspec_volatile [(match_operand:SI 0 "register_operand" "r")
                     (match_operand:SI 1 "register_operand" "r")
                     (match_operand:SI 2 "register_operand" "r")
                     (match_operand:SI 3 "register_operand" "r")]
    UNSPECV_NONLOCAL_GOTO)]
  ""
  ".flush\n\t(i32.store (i32.add (i32.load (get_local $fp)) (i32.const 8)) %O1)\n\t(i32.store (i32.add (i32.load (get_local $fp)) (i32.const 12)) %O0)\n\t(set $rv %O3)\n\t(br $mainloop)")

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
   ".flush\n\t(set_local $fp (i32.load %O0))\n\t(i32.store (i32.add (get_local $fp) (i32.const 8)) (i32.load (i32.add %O0 (i32.const 4))))\n\t(i32.store (i32.add (get_local $fp) (i32.const 12)) (i32.load (i32.add %O0 (i32.const 8))))\n\t(return (i32.or (get_local $fp) (i32.const 3)))")

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
  ".flush\n\t(i32.store (i32.add (i32.load (get_local $fp)) (i32.const 8)) %O1)\n\t(i32.store (i32.add (i32.load (get_local $fp)) (i32.const 12)) %O2)\n\t(set $rv %O3)\n\t(return (i32.or (i32.load (i32.add (i32.load (get_local $fp)) (i32.const 8))) (i32.const 3)))")

(define_expand "nonlocal_goto_receiver"
  [(unspec_volatile [(const_int 0)] UNSPECV_NONLOCAL_GOTO_RECEIVER)]
  ""
  "emit_insn (gen_nonlocal_goto_receiver_insn ());
   DONE;")

(define_insn "nonlocal_goto_receiver_insn"
  [(unspec_volatile [(const_int 0)] UNSPECV_NONLOCAL_GOTO_RECEIVER)]
  ""
  "")

(define_insn "thunk_goto_insn"
  [(set (pc)
        (unspec_volatile [(match_operand 0 "" "") ;; target
                          ] UNSPECV_THUNK_GOTO))
   ]
  ""
  "(return (call_import f_$\n\t.ncodetextlabel %O0\n\t(i32.const 0) (i32.add (get_local $sp) (i32.const 16)) (get_local $r0) (get_local $r1) (i32.add (get_local $dpc) (get_local $pc0)) (i32.shr_u %O0 (i32.const 4))))")
