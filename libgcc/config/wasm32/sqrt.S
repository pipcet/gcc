        .include "wasm32-macros.s"

        .if 0
        createsig FiiiiiiiE
        .pushsection .wasm.chars.import.ab
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.import.ab
        lstring "libc"
        lstring "dummy"
        .byte 4                 ; inlinable
        rleb128_32 __sigchar_FiiiiiiiE
        .popsection
        .globl $dummy
        defunb $dummy, FiiiiiiiE
        i32.const 1
        return
        end
        endefunb $dummy
        .endif

        createsig FddE
        .pushsection .wasm.chars.stdlib
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.stdlib
        lstring "libc"
        lstring "sqrt"
        .byte 4                 ; inlinable
        rleb128_32 __sigchar_FddE
        .popsection
        .globl $sqrt
        defunb $sqrt, FddE
        f64.const 1.0
        return
        end
        endefunb $sqrt
