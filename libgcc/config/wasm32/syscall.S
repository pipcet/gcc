        .include "wasm32-macros.s"

        .globl $syscall

        createsig FiiiiiiiE
        .pushsection .wasm.chars.import
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.import
        lstring "sys"
        lstring "call"
        .byte 0x00
        rleb128 __sigchar_FiiiiiiiE
        .popsection
        .pushsection .wasm.chars.function_index.a,""
$syscall:
        .byte 0xff
        .popsection
