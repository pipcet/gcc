#NO_APP
        .macro T a
        i32.const \a
        get_local $sp
        get_local $fp
        get_local $rp
        get_local $dpc
        .ifdef __wasm_blocks
        i32.const __wasm_blocks
        .else
        i32.const -1
        .endif
        call $trace
        drop
        .endm

        .macro .import3 module extname name
        .pushsection .str.lib,"ams",1
        .data
.ifndef __str_\name
__str_\name:
        .ascii "\extname"
        .byte 0
.endif
.ifndef __str_\module
__str_\module:
        .ascii "\module"
        .byte 0
.endif
        .text
        .global \name
	defun \name, FiiiiiiiE
	i32.const -16
	get_local $sp1
	i32.add
	set_local $sp
	get_local $sp
	set_local $fp
        i32.const __str_\module
	set_local $r0
        i32.const __str_\name
	set_local $r1
        get_local $pc0
        get_local $dpc
        get_local $sp
        get_local $r0
        get_local $r1
        i32.const 3
        call $syscall
        set_local $r0
        get_local $fp
        return
        end
	endefun \name
        .popsection
        .endm
