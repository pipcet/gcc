        .include "wasm64-macros.s"

        .globl $indcall
        .globl $extcall
        .globl $eh_return
        .globl $foreign_indcall

        .globl $i64_store8
        .globl $i64_store16
        .globl $i64_store32
        .globl $i64_store

        .globl $i64_load8_s
        .globl $i64_load16_s
        .globl $i64_load32_s
        .globl $i64_load8_u
        .globl $i64_load16_u
        .globl $i64_load32_u
        .globl $i64_load

        .globl $shl
        .globl $shr_s
        .globl $shr_u
        .globl __one_cmpldi2

        defun $indcall i64 i64 i64 i64 i64 i64 result i64
        get_local $pc0
        call_import[1] $print
        get_local $rpc
        call_import[1] $print
        get_local $pc0
        i32.wrap_i64
        get_local $dpc
        get_local $sp1
        get_local $r0
        get_local $r1
        get_local $rpc
        get_local $pc0
        call_indirect[6] __wasm_chars_type_std
        return[1]
        endefun $indcall

        .if 0
        defun $extcall i64 i64 i64 i64 i64 i64 result i64
        i64.const 0
        return[1]
        endefun $extcall

        defun $eh_return i64 i64 i64 i64 result i64
        i64.const 0
        return[1]
        endefun $eh_return
        .endif

        defun $i64_store8 i32 i64
        get_local 0
        get_local 1
        i32.wrap_i64
        i32.store8 a=0 0
        endefun $i64_store8

        defun $i64_store16 i32 i64
        get_local 0
        get_local 1
        i32.wrap_i64
        i32.store16 a=0 0
        endefun $i64_store16

        defun $i64_store32 i32 i64
        get_local 0
        get_local 1
        i32.wrap_i64
        i32.store a=0 0
        endefun $i64_store32

        defun $i64_store i32 i64
        get_local 0
        get_local 1
        f64.reinterpret_i64
        f64.store a=0 0
        endefun $i64_store

        defun $i64_load8_s i32 result i64
        get_local 0
        i32.load8_s a=0 0
        i64.extend_s_i32
        return[1]
        endefun $i64_load8_s

        defun $i64_load16_s i32 result i64
        get_local 0
        i32.load16_s a=0 0
        i64.extend_s_i32
        return[1]
        endefun $i64_load16_s

        defun $i64_load32_s i32 result i64
        get_local 0
        i32.load a=0 0
        i64.extend_s_i32
        return[1]
        endefun $i64_load32_s

        defun $i64_load8_u i32 result i64
        get_local 0
        i32.load8_u a=0 0
        i64.extend_u_i32
        return[1]
        endefun $i64_load8_u

        defun $i64_load16_u i32 result i64
        get_local 0
        i32.load16_u a=0 0
        i64.extend_u_i32
        return[1]
        endefun $i64_load16_u

        defun $i64_load32_u i32 result i64
        get_local 0
        i32.load a=0 0
        i64.extend_u_i32
        return[1]
        endefun $i64_load32_u

        defun $i64_load i32 result i64
        get_local 0
        f64.load a=0 0
        i64.reinterpret_f64
        return[1]
        endefun $i64_load

        defun $shl i64 i64 result i64
        get_local 0
        get_local 1
        i64.shl
        return[1]
        endefun $shl

        defun $shr_s i64 i64 result i64
        get_local 0
        get_local 1
        i64.shr_s
        return[1]
        endefun $shr_s

        defun $shr_u i64 i64 result i64
        get_local 0
        get_local 1
        i64.shr_u
        return[1]
        endefun $shr_u

        defun __one_cmpldi2 i64 i64 i64 i64 i64 i64 result i64
        i32.const 4096
        get_local $r0
        i64.const -1
        i64.xor
        call[2] $i64_store
        i64.const 0
        return[1]
        endefun __one_cmpldi2

        .pushsection .wasm.chars.export
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.export
        rleb128 _start
        .byte 20
        .ascii "f_0x4000000000025000"
        .popsection

        .pushsection .wasm.chars.type
        .global __wasm_chars_type_std
__wasm_chars_type_std:
__wasm_type_indcall:
        .byte 0x00
        .local __wasm_type_extcall
__wasm_type_extcall:
        .byte 0x00
__wasm_type_eh_return:
        .byte 0x00
__wasm_type_print:
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.type
        .byte 0x40
        .byte 6
        .rept 6
        .byte 2
        .endr
        .byte 1
        .byte 2
        .byte 0x40
        .byte 0x4
        .byte 0x2
        .byte 0x2
        .byte 0x2
        .byte 0x2
        .byte 0x1
        .byte 0x2
        .byte 0x40
        .byte 0x3
        .byte 0x2
        .byte 0x2
        .byte 0x2
        .byte 0x1
        .byte 0x2
        .byte 0x40
        .byte 1
        .byte 2
        .byte 0
        .popsection

        .globl __wasm_import_extcall
        .pushsection .wasm.chars.import
        .local __wasm_import_extcall
__wasm_import_extcall:
$extcall:
        .byte 0x00
$eh_return:
        .byte 0x00
$foreign_indcall:
        .byte 0x00
$print:
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.import
        rleb128 __wasm_type_extcall
        lstring thinthin
        lstring extcall
        rleb128 __wasm_type_eh_return
        lstring thinthin
        lstring eh_return
        rleb128 __wasm_type_indcall
        lstring thinthin
        lstring indcall
        rleb128 __wasm_type_print
        lstring console
        lstring print
        .popsection
