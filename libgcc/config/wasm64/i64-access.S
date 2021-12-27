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

        createsig FlllllllE
        defun $indcall FlllllllE
        get_local $pc0
        get_local $rpc
        get_local $pc0
        i64.const 32
        i64.shr_u
        i32.wrap_i64
        get_local $dpc
        get_local $sp1
        get_local $r0
        get_local $r1
        get_local $rpc
        get_local $pc0
        call_indirect[6] __sigchar_FlllllllE
        return[1]
        endefun $indcall

        .if 0
        defun $extcall FlllllllE
        i64.const 0
        return[1]
        endefun $extcall

        defun $eh_return FlllllE
        i64.const 0
        return[1]
        endefun $eh_return
        .endif

        defun $i64_store8 FvilE
        get_local 0
        get_local 1
        i32.wrap_i64
        i32.store8 a=0 0
        endefun $i64_store8

        defun $i64_store16 FvilE
        get_local 0
        get_local 1
        i32.wrap_i64
        i32.store16 a=0 0
        endefun $i64_store16

        defun $i64_store32 FvilE
        get_local 0
        get_local 1
        i32.wrap_i64
        i32.store a=0 0
        endefun $i64_store32

        defun $i64_store FvilE
        get_local 0
        get_local 1
        f64.reinterpret_i64
        f64.store a=0 0
        endefun $i64_store

        defun $i64_load8_s FliE
        get_local 0
        i32.load8_s a=0 0
        i64.extend_s_i32
        return[1]
        endefun $i64_load8_s

        defun $i64_load16_s FliE
        get_local 0
        i32.load16_s a=0 0
        i64.extend_s_i32
        return[1]
        endefun $i64_load16_s

        defun $i64_load32_s FliE
        get_local 0
        i32.load a=0 0
        i64.extend_s_i32
        return[1]
        endefun $i64_load32_s

        defun $i64_load8_u FliE
        get_local 0
        i32.load8_u a=0 0
        i64.extend_u_i32
        return[1]
        endefun $i64_load8_u

        defun $i64_load16_u FliE
        get_local 0
        i32.load16_u a=0 0
        i64.extend_u_i32
        return[1]
        endefun $i64_load16_u

        defun $i64_load32_u FliE
        get_local 0
        i32.load a=0 0
        i64.extend_u_i32
        return[1]
        endefun $i64_load32_u

        defun $i64_load FliE
        get_local 0
        f64.load a=0 0
        i64.reinterpret_f64
        return[1]
        endefun $i64_load

        defun $shl FlllE
        get_local 0
        get_local 1
        i64.shl
        return[1]
        endefun $shl

        defun $shr_s FlllE
        get_local 0
        get_local 1
        i64.shr_s
        return[1]
        endefun $shr_s

        defun $shr_u FlllE
        get_local 0
        get_local 1
        i64.shr_u
        return[1]
        endefun $shr_u

        defun __one_cmpldi2 FlllllllE
        i32.const 4096
        get_local $r0
        i64.const -1
        i64.xor
        call[2] $i64_store
        i64.const 0
        return[1]
        endefun __one_cmpldi2

        .macro export symbol, name
        .pushsection .wasm.chars.export
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.export
        rleb128r32 \symbol
        lstring \name
        .popsection
        .endm

        export _start, f_0x4000000000025000
        export $indcall, indcall
        export startindex, startindex

        defun startindex FlE
        i64.const _start
        return[1]
        endefun startindex

        .macro import symbol, module, name, sig
        createsig \sig
        .pushsection .wasm.chars.import
\symbol\():
        .byte 0x00
        .popsection
        .pushsection .wasm.payload.import
        rleb128 __sigchar_\sig
        lstring \module
        lstring \name
        .popsection
        .endm

        import __wasm_import_extcall, thinthin, extcall, FlllllE
        import $eh_return, thinthin, eh_return, FllllE
        import $foreign_indcall, thinthin, indcall, FllllllllE
        import $print, console, print, FvlE

        .globl __wasm_import_extcall
        .set $extcall, __wasm_import_extcall
