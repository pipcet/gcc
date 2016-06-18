#NO_APP
        .macro .import3 module, extname, name
        .section .str.lib,"ams",1
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
        .p2align 4+8
        .global \name
        .pushsection .wasm_pwas%S,"a"
(
	.wasmtextlabeldeffirst \name
	.ascii "\""
        f_$
	.textlabel \name
	.ascii "\""
	(block
		(if (i64.ne (i64.and (get_local $rp) (i64.const 3)) (i64.const 1)) (return (get_local $rp)))
		(call $i64_store (i32.wrap_i64 (i64.add (get_local $fp) (i64.const 0))) (i64.add (get_local $fp) (i64.const 96)))
		(call $i64_store (i32.wrap_i64 (i64.add (get_local $fp) (i64.const 8))) (i64.shl (get_local $pc0) (i64.const 4)))
		(call $i64_store (i32.wrap_i64 (i64.add (get_local $fp) (i64.const 16))) (i64.shl (i64.add (get_local $pc0) (get_local $dpc)) (i64.const 4)))
		(call $i64_store (i32.wrap_i64 (i64.add (get_local $fp) (i64.const 24))) (get_local $rpc))
		(call $i64_store (i32.wrap_i64 (i64.add (get_local $fp) (i64.const 32))) (get_local $sp))
		(call $i64_store (i32.wrap_i64 (i64.add (get_local $fp) (i64.const 40))) (i64.const 1008))
		(return (get_local $rp))
	)
	(set_local $sp (i64.add (get_local $sp1) (i64.const -16)))
	.labeldef_debug .LFB0
	(set_local $sp (i64.add (get_local $sp) (i64.const -96)))
	(set_local $fp (get_local $sp))
	.labeldef_debug .LCFI0
	(set_local $sp (i64.add (get_local $sp) (i64.const -16)))
        .labeldef_internal .LSC0_\module\()_\name
;; 6 "subrepos/gcc/gcc//config/wasm/import.c" 1
	(set_local $rp (call_import $extcall
                .ndatatextlabel __str_\module
                .ndatatextlabel __str_\name
                (i64.add (get_local $pc0) (get_local $dpc)) (i64.add (get_local $sp) (i64.const 96))))
        (if (i32.and (i32.wrap_i64 (get_local $rp)) (i32.const 3)) (then
                (set_local $dpc
                .dpc .LSC0_\module\()_\name
                )
                (set_local $rp (i64.or (get_local $fp) (i64.const 1)))
                (br $mainloop)
        ) (else
                (set_local $dpc
                .dpc .LSC1_\module\()_\name
                )
        ))
        .labeldef_internal .LSC1_\module\()_\name
	(return (i64.add (get_local $fp) (i64.const 48)))
	.set __wasm_fallthrough, 0
	.labeldef_debug .LFE0
	.wasmtextlabeldeflast .ends.\name
	(set_local $rp (i64.add (get_local $sp1) (i64.const -16)))
		(set_local $pc0 (i64.shr_u (call $i64_load (i32.wrap_i64 (i64.add (get_local $rp) (i64.const 4)))) (i64.const 4)))
		(set_local $dpc (i64.sub (i64.shr_u (call $i64_load (i32.wrap_i64 (i64.add (get_local $rp) (i64.const 8)))) (i64.const 4)) (get_local $pc0)))
		(set_local $rpc (call $i64_load (i32.wrap_i64 (i64.add (get_local $rp) (i64.const 12)))))
		(set_local $sp (call $i64_load (i32.wrap_i64 (i64.add (get_local $rp) (i64.const 16)))))
	(set_local $fp (get_local $rp))
 )
        .pushsection .special.define,"a"
        .pushsection .javascript%S,"a"
        .ascii "\tdeffun({name: \"f_"
        .codetextlabel .L.\name
        .ascii "\", symbol: \"\name\", pc0: "
        .codetextlabel .L.\name
        .ascii ", pc1: "
        .codetextlabel .ends.\name
        .ascii ", regsize: 0, regmask: 0});\n"
        .popsection
        .popsection

        .pushsection .special.fpswitch,"a"
        .pushsection .wasm_pwas%S,"a"
        (return (call $f_$
        .codetextlabel \name
        (get_local $dpc) (get_local $sp1) (get_local $r0) (get_local $r1) (get_local $rpc) (get_local $pc0)))
        .popsection
        .popsection
        .endm
