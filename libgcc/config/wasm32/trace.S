        .include "wasm32-macros.s"

        .globl $trace

        createsig FiiiiE
        .pushsection .wasm.chars.import
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.import
        lstring "sys"
        lstring "trace"
        .byte 0x00
        rleb128 __sigchar_FiiiiE
        .popsection
        .pushsection .wasm.chars.function_index.a,"",@nobits
$trace:
        .byte 0x00
        .popsection
        .pushsection .wasm.chars.name.a
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.name.a
        lstring $indcall
        .byte 3
        lstring a
        lstring b
        lstring c
        .popsection
