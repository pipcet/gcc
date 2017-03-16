        .include "wasm32-macros.s"

        .globl $syscall

        createsig FiiiiiiiE
        .pushsection .space.import
        .byte 0x00
        .popsection
        .pushsection .wasm.import
        lstring "sys"
        lstring "call"
        .byte 0x00
        rleb128 __sigchar_FiiiiiiiE
        .popsection
        .pushsection .space.function_index.import,""
        .type $syscall, @function
        .size $syscall, 1
$syscall:
        .byte 0xff
        .popsection
        .pushsection .space.element.import
        .byte 0x00
        .popsection
        .pushsection .wasm.element.import
        rleb128_32 $syscall
        .popsection
        .if 0
        .pushsection .space.name.function.import
        .byte 0x00
        .popsection
        .pushsection .wasm.name.function.import
        lstring $syscall
        .byte 0
        .popsection
        .endif
