#NO_APP
	.globl $syscall
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
	local.get $sp
	i32.const -56
	i32.add
	local.set $sp
	local.get $sp
	local.set $fp
	.labeldef_internal .LL2\()\@
	i32.const __str_\module
	local.set $r0
	i32.const __str_\name
	local.set $r1
	local.get $fp
	.dpc .LLimp\()\@
	local.tee $dpc
	local.get $sp
        i32.const 56
        i32.add
	local.get $r0
	local.get $r1
	local.get $ofp
	call $syscall
	local.set $r0
	.labeldef_internal .LLimp\()\@
        local.get $r0
        local.tee $rp
        i32.const 3
        i32.and
        if[]
        .dpc .LL2\()\@
        local.set $dpc
        throw1
        end
	local.get $r0
	i32.const 3
	i32.and
	local.set $r0
	local.get $r0
	i32.const 0
	i32.ne
	if[]
	.dpc .LL2\()\@
	local.set $dpc
	jump1
	end
	i32.const 56
	local.get $fp
	i32.add
	return
	nextcase
	local.get $sp
	local.set $rp
	local.get $rp
	i32.load a=2 0
	local.set $ofp
	i32.const 16
	local.get $rp
	i32.add
	i32.load a=2 0
	local.set $dpc
	i32.const 24
	local.get $rp
	i32.add
	i32.load a=2 0
	local.set $rpc
	i32.const 32
	local.get $rp
	i32.add
	i32.load a=2 0
	local.set $sp
	i32.const 48
	local.get $rp
	i32.add
	i32.load a=2 0
	local.set $r0
	local.get $rp
	local.set $fp
	jump2
	nextcase
	end
	i32.const 3
	local.get $rp
	i32.and
	i32.const 1
	i32.ne
	if[]
	local.get $rp
	return
	end
	local.get $sp
	i32.const -16
	i32.add
	local.get $fp
	i32.store a=2 0
	i32.const 0
	local.get $fp
	i32.add
	local.get $fp
	i32.const 56
	i32.add
	i32.store a=2 0
	local.get $fp
	local.get $ofp
	i32.store a=2 0
	i32.const 16
	local.get $fp
	i32.add
	local.get $dpc
	i32.store a=2 0
	i32.const 24
	local.get $fp
	i32.add
	local.get $rpc
	i32.store a=2 0
	i32.const 32
	local.get $fp
	i32.add
	local.get $sp
	i32.store a=2 0
	i32.const 40
	local.get $fp
	i32.add
	i32.const 16
	i32.store a=2 0
	i32.const 48
	local.get $fp
	i32.add
	local.get $r0
	i32.store a=2 0
	local.get $rp
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
	local.get $sp
	i32.const -56
	i32.add
	local.set $sp
	local.get $sp
	local.set $fp
	.labeldef_internal .LL2\()\@
        global.get $got
	i32.const __str_\module@got
        i32.add
        i32.load a=2 0
	local.set $r0
        global.get $got
	i32.const __str_\name@got
        i32.add
        i32.load a=2 0
	local.set $r1
	local.get $fp
	.dpc .LLimp\()\@
	local.tee $dpc
	local.get $sp
        i32.const 56
        i32.add
	local.get $r0
	local.get $r1
	i32.const 3
	call $syscall
	local.set $r0
	.labeldef_internal .LLimp\()\@
        local.get $r0
        local.tee $rp
        i32.const 3
        i32.and
        if[]
        .dpc .LL2\()\@
        local.set $dpc
        throw1
        end
	local.get $r0
	i32.const 3
	i32.and
	local.set $r0
	local.get $r0
	i32.const 0
	i32.ne
	if[]
	.dpc .LL2\()\@
	local.set $dpc
	jump1
	end
	i32.const 56
	local.get $fp
	i32.add
	return
	nextcase
	local.get $sp
	local.set $rp
	local.get $rp
	i32.load a=2 0
	local.set $ofp
	i32.const 16
	local.get $rp
	i32.add
	i32.load a=2 0
	local.set $dpc
	i32.const 24
	local.get $rp
	i32.add
	i32.load a=2 0
	local.set $rpc
	i32.const 32
	local.get $rp
	i32.add
	i32.load a=2 0
	local.set $sp
	i32.const 48
	local.get $rp
	i32.add
	i32.load a=2 0
	local.set $r0
	local.get $rp
	local.set $fp
	jump2
	nextcase
	end
	i32.const 3
	local.get $rp
	i32.and
	i32.const 1
	i32.ne
	if[]
	local.get $rp
	return
	end
	local.get $sp
	i32.const -16
	i32.add
	local.get $fp
	i32.store a=2 0
	i32.const 0
	local.get $fp
	i32.add
	local.get $fp
	i32.const 56
	i32.add
	i32.store a=2 0
	i32.const 8
	local.get $fp
	i32.add
        global.get $plt
        i32.const \name
        i32.add
	i32.store a=2 0
	i32.const 16
	local.get $fp
	i32.add
	local.get $dpc
	i32.store a=2 0
	i32.const 24
	local.get $fp
	i32.add
	local.get $rpc
	i32.store a=2 0
	i32.const 32
	local.get $fp
	i32.add
	local.get $sp
	i32.store a=2 0
	i32.const 40
	local.get $fp
	i32.add
	i32.const 16
	i32.store a=2 0
	i32.const 48
	local.get $fp
	i32.add
	local.get $r0
	i32.store a=2 0
	local.get $rp
	return
	end
	endefun \name
        .endm
