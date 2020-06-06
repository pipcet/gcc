#NO_APP
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
        .pushsection .wasm.%S,2*__wasm_counter+1
        .global \name
	defun \name, FiiiiiiiE
        nextcase
	get_local $sp
	i32.const -56
	i32.add
	set_local $sp
	get_local $sp
	set_local $fp
	.labeldef_internal .LL2\()\@
	i32.const __str_\module
	set_local $r0
	i32.const __str_\name
	set_local $r1
	get_local $pc0
	.dpc .LLimp\()\@
	tee_local $dpc
	get_local $sp
        i32.const 56
        i32.add
	get_local $r0
	get_local $r1
	i32.const 3
	call $syscall
	set_local $r0
	.labeldef_internal .LLimp\()\@
        get_local $r0
        tee_local $rp
        i32.const 3
        i32.and
        if[]
        .dpc .LL2\()\@
        set_local $dpc
        throw1
        end
	get_local $r0
	i32.const 3
	i32.and
	set_local $r0
	get_local $r0
	i32.const 0
	i32.ne
	if[]
	.dpc .LL2\()\@
	set_local $dpc
	jump1
	end
	i32.const 56
	get_local $fp
	i32.add
	return
	nextcase
	get_local $sp
	set_local $rp
	i32.const 8
	get_local $rp
	i32.add
	i32.load a=2 0
	set_local $pc0
	i32.const 16
	get_local $rp
	i32.add
	i32.load a=2 0
	set_local $dpc
	i32.const 24
	get_local $rp
	i32.add
	i32.load a=2 0
	set_local $rpc
	i32.const 32
	get_local $rp
	i32.add
	i32.load a=2 0
	set_local $sp
	i32.const 48
	get_local $rp
	i32.add
	i32.load a=2 0
	set_local $r0
	get_local $rp
	set_local $fp
	jump2
	nextcase
	end
	i32.const 3
	get_local $rp
	i32.and
	i32.const 1
	i32.ne
	if[]
	get_local $rp
	return
	end
	get_local $sp
	i32.const -16
	i32.add
	get_local $fp
	i32.store a=2 0
	i32.const 0
	get_local $fp
	i32.add
	get_local $fp
	i32.const 56
	i32.add
	i32.store a=2 0
	i32.const 8
	get_local $fp
	i32.add
	get_local $pc0
	i32.store a=2 0
	i32.const 16
	get_local $fp
	i32.add
	get_local $dpc
	i32.store a=2 0
	i32.const 24
	get_local $fp
	i32.add
	get_local $rpc
	i32.store a=2 0
	i32.const 32
	get_local $fp
	i32.add
	get_local $sp
	i32.store a=2 0
	i32.const 40
	get_local $fp
	i32.add
	i32.const 16
	i32.store a=2 0
	i32.const 48
	get_local $fp
	i32.add
	get_local $r0
	i32.store a=2 0
	get_local $rp
	return
	end
	endefun \name
        .endm

        .macro .import3_pic module extname name
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
        .pushsection .wasm.%S,2*__wasm_counter+1
        .global \name
	defun \name, FiiiiiiiE
        nextcase
	get_local $sp
	i32.const -56
	i32.add
	set_local $sp
	get_local $sp
	set_local $fp
	.labeldef_internal .LL2\()\@
        get_global $got
	i32.const __str_\module@got
        i32.add
        i32.load a=2 0
	set_local $r0
        get_global $got
	i32.const __str_\name@got
        i32.add
        i32.load a=2 0
	set_local $r1
	get_local $pc0
	.dpc .LLimp\()\@
	tee_local $dpc
	get_local $sp
        i32.const 56
        i32.add
	get_local $r0
	get_local $r1
	i32.const 3
	call $syscall
	set_local $r0
	.labeldef_internal .LLimp\()\@
        get_local $r0
        tee_local $rp
        i32.const 3
        i32.and
        if[]
        .dpc .LL2\()\@
        set_local $dpc
        throw1
        end
	get_local $r0
	i32.const 3
	i32.and
	set_local $r0
	get_local $r0
	i32.const 0
	i32.ne
	if[]
	.dpc .LL2\()\@
	set_local $dpc
	jump1
	end
	i32.const 56
	get_local $fp
	i32.add
	return
	nextcase
	get_local $sp
	set_local $rp
	i32.const 8
	get_local $rp
	i32.add
	i32.load a=2 0
	set_local $pc0
	i32.const 16
	get_local $rp
	i32.add
	i32.load a=2 0
	set_local $dpc
	i32.const 24
	get_local $rp
	i32.add
	i32.load a=2 0
	set_local $rpc
	i32.const 32
	get_local $rp
	i32.add
	i32.load a=2 0
	set_local $sp
	i32.const 48
	get_local $rp
	i32.add
	i32.load a=2 0
	set_local $r0
	get_local $rp
	set_local $fp
	jump2
	nextcase
	end
	i32.const 3
	get_local $rp
	i32.and
	i32.const 1
	i32.ne
	if[]
	get_local $rp
	return
	end
	get_local $sp
	i32.const -16
	i32.add
	get_local $fp
	i32.store a=2 0
	i32.const 0
	get_local $fp
	i32.add
	get_local $fp
	i32.const 56
	i32.add
	i32.store a=2 0
	i32.const 8
	get_local $fp
	i32.add
        get_global $plt
        i32.const \name
        i32.add
	i32.store a=2 0
	i32.const 16
	get_local $fp
	i32.add
	get_local $dpc
	i32.store a=2 0
	i32.const 24
	get_local $fp
	i32.add
	get_local $rpc
	i32.store a=2 0
	i32.const 32
	get_local $fp
	i32.add
	get_local $sp
	i32.store a=2 0
	i32.const 40
	get_local $fp
	i32.add
	i32.const 16
	i32.store a=2 0
	i32.const 48
	get_local $fp
	i32.add
	get_local $r0
	i32.store a=2 0
	get_local $rp
	return
	end
	endefun \name
        .endm
