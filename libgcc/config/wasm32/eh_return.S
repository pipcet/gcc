        .include "wasm32-macros.s"

        .globl $eh_return

        createsig FiiiiE
        .pushsection .space.import
        .byte 0x00
        .popsection
        .pushsection .wasm.import
        lstring "sys"
        lstring "eh_return"
        .byte 0x00
        rleb128 __sigchar_FiiiiE
        .popsection
        .pushsection .space.function_index.import,""
        .type $eh_return, @function
        .size $eh_return, 1
$eh_return:
        .byte 0x00
        .popsection
        .pushsection .space.element.import
        .byte 0x00
        .popsection
        .pushsection .wasm.element.import
        rleb128_32 $eh_return
        .popsection
        .if 0
        .pushsection .space.name.function.import
        .byte 0x00
        .popsection
        .pushsection .wasm.name.function.import
        lstring $eh_return
        .byte 0
        .popsection
        .endif
