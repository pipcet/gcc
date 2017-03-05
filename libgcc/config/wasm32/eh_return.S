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
        .pushsection .wasm.chars.function_index.a,""
        .type $eh_return, @function
        .size $eh_return, 1
$eh_return:
        .byte 0x00
        .popsection
        .pushsection .wasm.chars.element.a
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.element.a
        rleb128_32 $eh_return
        .popsection
        .if 0
        .pushsection .wasm.chars.name.a
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.name.a
        lstring $eh_return
        .byte 0
        .popsection
        .endif
