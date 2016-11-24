        .include "wasm32-macros.s"

        .globl $eh_return

        createsig FiiiiiiiE
        .pushsection .wasm.chars.import
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.import
        lstring "sys"
        lstring "eh_return"
        .byte 0x00
        rleb128 __sigchar_FiiiiiiiE
        .popsection
        .pushsection .wasm.chars.function_index.a,"",@nobits
$eh_return:
        .byte 0x00
        .popsection
