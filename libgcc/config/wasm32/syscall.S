        .include "wasm32-macros.s"

        .globl $syscall

        createsig FiiiiiiiE
        .pushsection .wasm.chars.import
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.import
        lstring "sys"
        lstring "call"
        .byte 0x00
        rleb128 __sigchar_FiiiiiiiE
        .popsection
        .pushsection .wasm.chars.function_index.a,""
        .type $syscall, @function
        .size $syscall, 1
$syscall:
        .byte 0xff
        .popsection
        .pushsection .wasm.chars.element.a
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.element.a
        rleb128_32 $syscall
        .popsection
        .if 0
        .pushsection .wasm.chars.name.a
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.name.a
        lstring $syscall
        .byte 0
        .popsection
        .endif
