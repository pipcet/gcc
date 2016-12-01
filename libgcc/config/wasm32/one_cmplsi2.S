        .include "wasm32-macros.s"

        .globl __one_cmplsi2

        defun __one_cmplsi2 FiiiiiiiE
        i32.const 8288
        get_local $r0
        i32.const -1
        i32.xor
        i32.store a=2 0
        i32.const 0
        return
        end
        endefun __one_cmplsi2
