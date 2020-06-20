        .include "wasm32-macros.s"

        .globl __one_cmplsi2

	text_section
        defun __one_cmplsi2 FiiiiiiiE
        nextcase
        nextcase
        end
        i32.const 8288
        local.get $r0
        i32.const -1
        i32.xor
        i32.store a=2 0
        i32.const 0
        return
        end
        endefun __one_cmplsi2
