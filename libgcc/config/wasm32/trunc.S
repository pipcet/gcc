        .include "wasm32-macros.s"

        .globl $truncdfsi

        createsig FidE
        .pushsection .space.import
        .byte 0x00
        .popsection
        .pushsection .wasm.import
        lstring "sys"
        lstring "truncdfsi"
        .byte 0x00
        rleb128 __sigchar_FidE
        .popsection
        .pushsection .space.function_index.import,"",@nobits
$truncdfsi:
        .byte 0x00
        .popsection
        .pushsection .space.element.import
        .byte 0x00
        .popsection
        .pushsection .wasm.element.import
        rleb128_32 $truncdfsi
        .popsection
        .if 0
        .pushsection .space.name.function.import
        .byte 0x00
        .popsection
        .pushsection .wasm.name.function.import
        lstring $truncdfsi
        .byte 0
        .popsection
        .endif
