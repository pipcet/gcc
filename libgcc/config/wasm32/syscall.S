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
$syscall:
        .byte 0xff
        .popsection
        .pushsection .wasm.chars.name.a
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.name.a
        lstring $syscall
        .byte 6
        lstring dpc
        lstring sp1
        lstring r0
        lstring r1
        lstring rpc
        lstring pc0
        .popsection
