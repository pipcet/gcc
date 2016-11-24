#NO_APP
        .macro .import3 smodule sextname name
        .pushsection .str.lib,"ams",1
        .data
.ifndef __str_\name
__str_\name:
        .ascii "\sextname"
        .byte 0
.endif
.ifndef __str_\smodule
__str_\smodule:
        .ascii "\smodule"
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
        i32.const __str_\smodule
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
        return[1]
        end
	endefun \name
        .popsection
        .endm
