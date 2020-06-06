        .include "wasm32-macros.s"

        .globl $eh_return

        createsig FiiiiE
        .pushsection .space.import
        .reloc ., R_WASM32_CODE_POINTER, 0f
        .byte 0x00
        .popsection
        .pushsection .wasm.import
0:      lstring "sys"
        lstring "eh_return"
        .byte 0
        rleb128 __sigchar_FiiiiE
        .popsection
        .pushsection .space.function_index.import,""
        .type $eh_return, @function
        .size $eh_return, 1
$eh_return:
        .reloc .,R_WASM32_INDEX,1f
        .byte 0
        .popsection
        .pushsection .space.element.import
        .reloc .,R_WASM32_CODE_POINTER,0f
1:      .byte 0
        .popsection
        .pushsection .wasm.element.import
0:      rleb128_32 $eh_return
        .popsection
        .if 0
        .pushsection .space.name.function.import
        .byte 0
        .popsection
        .pushsection .wasm.name.function.import
        lstring $eh_return
        .byte 0
        .popsection
        .endif
