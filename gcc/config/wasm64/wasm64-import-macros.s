        .macro .import3 module, extname, name
        .pushsection .str.lib,"ams",1
        .data
.ifndef __str_\name
__str_\name:
        .asciz "\extname"
.endif
.ifndef __str_\module
__str_\module:
        .asciz "\module"
.endif
        .text
        .global \name
	defun \name, i64 i64 i64 i64 i64 i64 result i64
	i64.const -16
	get_local $sp1
	i64.add
	set_local $sp
	.labeldef_debug .LFB0
	get_local $sp
	i64.const -96
	i64.add
	set_local $sp
	get_local $sp
	set_local $fp
        i64.const __str_\module
	set_local $r0
        i64.const __str_\name
	set_local $r1
        get_local $pc0
        get_local $dpc
        i64.add
	set_local $r2
        get_local $sp
        i64.const 96
        i64.add
	set_local $r3
	get_local $fp
	i64.const 128
	i64.add
	i32.wrap_i64
	call[1] $i64_load32_s
	set_local $r4
	get_local $r4
	i64.const 32
	call[2] $shl
	set_local $r4
	get_local $r4
	i64.const 32
	call[2] $shr_s
	set_local $r4
	get_local $fp
	i64.const 132
	i64.add
	i32.wrap_i64
	call[1] $i64_load32_s
	set_local $r5
	get_local $r5
	i64.const 32
	call[2] $shl
	set_local $r5
	get_local $r5
	i64.const 32
	call[2] $shr_s
	set_local $r5
#APP
;; 6 "./subrepos/gcc/gcc/config/wasm64/import.c" 1
	get_local $r0
	get_local $r1
	get_local $r2
	get_local $r3
        .if 0
	get_local $r4
	get_local $r5
        .endif
	call_import[4] $extcall
	set_local $r0
;; 0 "" 2
#APP
	get_local $r0
	i64.const 32
	call[2] $shl
	set_local $r0
	get_local $r0
	i64.const 32
	call[2] $shr_s
	set_local $r0
	i32.const 4096
	get_local $r0
	call[2] $i64_store
	i64.const 96
	get_local $fp
	i64.add
	return[1]
	.set __wasm64_fallthrough, 0
	.labeldef_debug .LFE0
	nextcase
	i64.const -16
	get_local $sp1
	i64.add
	set_local $rp
	get_local $rp
	set_local $fp
	i64.const 8
	get_local $rp
	i64.add
	i32.wrap_i64
	call[1] $i64_load
	i64.const 4
	i64.shr_u
	set_local $pc0
	i64.const 16
	get_local $rp
	i64.add
	i32.wrap_i64
	call[1] $i64_load
	i64.const 4
	i64.shr_u
	get_local $pc0
	i64.sub
	set_local $dpc
	i64.const 24
	get_local $rp
	i64.add
	i32.wrap_i64
	call[1] $i64_load
	set_local $rpc
	i64.const 32
	get_local $rp
	i64.add
	i32.wrap_i64
	call[1] $i64_load
	set_local $sp
	i64.const 48
	get_local $rp
	i64.add
	i32.wrap_i64
	call[1] $i64_load
	set_local $r0
	i64.const 56
	get_local $rp
	i64.add
	i32.wrap_i64
	call[1] $i64_load
	set_local $r1
	i64.const 64
	get_local $rp
	i64.add
	i32.wrap_i64
	call[1] $i64_load
	set_local $r2
	i64.const 72
	get_local $rp
	i64.add
	i32.wrap_i64
	call[1] $i64_load
	set_local $r3
	i64.const 80
	get_local $rp
	i64.add
	i32.wrap_i64
	call[1] $i64_load
	set_local $r4
	i64.const 88
	get_local $rp
	i64.add
	i32.wrap_i64
	call[1] $i64_load
	set_local $r5
	nextcase
	i64.const 3
	get_local $rp
	i64.and
	i64.const 1
	i64.ne
	if
	get_local $rp
	return[1]
	end
	i64.const 0
	get_local $fp
	i64.add
	i32.wrap_i64
	get_local $fp
	i64.const 96
	i64.add
	call[2] $i64_store
	i64.const 8
	get_local $fp
	i64.add
	i32.wrap_i64
	get_local $pc0
	i64.const 4
	i64.shl
	call[2] $i64_store
	i64.const 16
	get_local $fp
	i64.add
	i32.wrap_i64
	get_local $pc0
	get_local $dpc
	i64.add
	i64.const 4
	i64.shl
	call[2] $i64_store
	i64.const 24
	get_local $fp
	i64.add
	i32.wrap_i64
	get_local $rpc
	call[2] $i64_store
	i64.const 32
	get_local $fp
	i64.add
	i32.wrap_i64
	get_local $sp
	call[2] $i64_store
	i64.const 40
	get_local $fp
	i64.add
	i32.wrap_i64
	i64.const -769794195
	call[2] $i64_store
	i64.const 48
	get_local $fp
	i64.add
	i32.wrap_i64
	get_local $r0
	call[2] $i64_store
	i64.const 56
	get_local $fp
	i64.add
	i32.wrap_i64
	get_local $r1
	call[2] $i64_store
	i64.const 64
	get_local $fp
	i64.add
	i32.wrap_i64
	get_local $r2
	call[2] $i64_store
	i64.const 72
	get_local $fp
	i64.add
	i32.wrap_i64
	get_local $r3
	call[2] $i64_store
	i64.const 80
	get_local $fp
	i64.add
	i32.wrap_i64
	get_local $r4
	call[2] $i64_store
	i64.const 88
	get_local $fp
	i64.add
	i32.wrap_i64
	get_local $r5
	call[2] $i64_store
	get_local $rp
	return[1]
	endefun \name
        .popsection
        .endm
