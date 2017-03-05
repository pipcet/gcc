        .include "wasm32-macros.s"

        .globl $trace

        createsig FiiiiiiiE
        .pushsection .wasm.chars.import
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.import
        lstring "sys"
        lstring "trace"
        .byte 0x00
        rleb128 __sigchar_FiiiiiiiE
        .popsection
        .pushsection .wasm.chars.function_index.a,"",@nobits
$trace:
        .byte 0x00
        .popsection
        .pushsection .wasm.chars.element.a
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.element.a
        rleb128_32 $trace
        .popsection
        .if 0
        .pushsection .wasm.chars.name.a
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.name.a
        lstring $trace
        .byte 0
        .popsection
        .endif
