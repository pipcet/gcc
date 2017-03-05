        .include "wasm32-macros.s"

        .globl $indcall

        createsig FiiiiiiiE
        .pushsection .wasm.chars.import
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.import
        lstring "sys"
        lstring "indcall"
        .byte 0x00
        rleb128 __sigchar_FiiiiiiiE
        .popsection
        .pushsection .wasm.chars.function_index.a,"",@nobits
$indcall:
        .byte 0x00
        .popsection
        .if 0
        .pushsection .wasm.chars.name.a
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.name.a
        lstring $indcall
        .byte 0
        .popsection
        .endif
