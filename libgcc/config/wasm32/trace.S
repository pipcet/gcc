        .include "wasm32-macros.s"

        .globl $trace

        createsig FiiiiiiiE
        .pushsection .space.import
        .byte 0x00
        .popsection
        .pushsection .wasm.import
        lstring "sys"
        lstring "trace"
        .byte 0x00
        rleb128 __sigchar_FiiiiiiiE
        .popsection
        .pushsection .space.function_index.import,"",@nobits
$trace:
        .byte 0x00
        .popsection
        .pushsection .space.element.import
        .byte 0x00
        .popsection
        .pushsection .wasm.element.import
        rleb128_32 $trace
        .popsection
        .if 0
        .pushsection .space.name.function.import
        .byte 0x00
        .popsection
        .pushsection .wasm.name.function.import
        lstring $trace
        .byte 0
        .popsection
        .endif
