        .include "wasm32-macros.s"

        .globl $syscall

        createsig FiiiiiiiE
        .pushsection .space.import,"ax"
        .reloc .,R_WASM32_CODE_POINTER,0f
        .byte 0x00
        .popsection
        .pushsection .wasm.import,"ax"
0:      lstring "sys"
        lstring "call"
        .byte 0x00
        rleb128 __sigchar_FiiiiiiiE
        .popsection
        .pushsection .space.function_index.import,"ax"
        .type $syscall, @function
        .size $syscall, 1
$syscall:
        .reloc .,R_WASM32_INDEX,1f
        .byte 0xff
        .popsection
        .pushsection .space.element.import,"ax"
        .reloc .,R_WASM32_CODE_POINTER,0f
1:      .byte 0x00
        .popsection
        .pushsection .wasm.element.import,"ax"
0:      rleb128_32 $syscall
        .popsection
        .if 0
        .pushsection .space.name.function.import,"ax"
        .byte 0x00
        .popsection
        .pushsection .wasm.name.function.import,"ax"
        lstring $syscall
        .byte 0
        .popsection
        .endif

        .globl $debug

        createsig FiiE
        .pushsection .space.import,"ax"
        .reloc .,R_WASM32_CODE_POINTER,0f
        .byte 0x00
        .popsection
        .pushsection .wasm.import,"ax"
0:      lstring "sys"
        lstring "debug"
        .byte 0x00
        rleb128 __sigchar_FiiE
        .popsection
        .pushsection .space.function_index.import,"ax"
        .type $debug, @function
        .size $debug, 1
$debug:
        .reloc .,R_WASM32_INDEX,1f
        .byte 0xff
        .popsection
        .pushsection .space.element.import,"ax"
        .reloc .,R_WASM32_CODE_POINTER,0f
1:      .byte 0x00
        .popsection
        .pushsection .wasm.element.import,"ax"
0:      rleb128_32 $debug
        .popsection
        .if 0
        .pushsection .space.name.function.import,"ax"
        .byte 0x00
        .popsection
        .pushsection .wasm.name.function.import,"ax"
        lstring $debug
        .byte 0
        .popsection
        .endif
