        .include "wasm32-macros.s"

        .globl $eh_return

        createsig FiiiiE
        .pushsection .wasm.chars.import
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.import
        lstring "sys"
        lstring "eh_return"
        .byte 0x00
        rleb128 __sigchar_FiiiiE
        .popsection
        .pushsection .wasm.chars.function_index.a,"",@nobits
$eh_return:
        .byte 0x00
        .popsection
        .pushsection .wasm.chars.name.a
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.name.a
        lstring $eh_return
        .byte 4
        lstring a
        lstring b
        lstring c
        lstring d
        .popsection
