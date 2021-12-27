        .include "wasm32-macros.s"

        .globl $indcall

        createsig FiiiiiiiE
        .pushsection .space.import
        .byte 0x00
        .popsection
        .pushsection .wasm.import
        lstring "sys"
        lstring "indcall"
        .byte 0x00
        rleb128 __sigchar_FiiiiiiiE
        .popsection
        .pushsection .space.function_index.import,"",@nobits
$indcall:
        .byte 0x00
        .popsection
        .if 0
        .pushsection .space.name.function.import
        .byte 0x00
        .popsection
        .pushsection .wasm.name.function.import
        lstring $indcall
        .byte 0
        .popsection
        .endif
