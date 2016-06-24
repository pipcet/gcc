#NO_APP
        .set __wasm_counter, 0
        .set $dpc, 0
        .set $sp1, 1
        .set $r0, 2
        .set $r1, 3
        .set $rpc, 4
        .set $pc0, 5
        .set $rp, 6
        .set $fp, 7
        .set $sp, 8
        .set $r2, 9
        .set $r3, 10
        .set $r4, 11
        .set $r5, 12
        .set $r6, 13
        .set $r7, 14
        .set $i0, 15
        .set $i1, 16
        .set $i2, 17
        .set $i3, 18
        .set $i4, 19
        .set $i5, 20
        .set $i6, 21
        .set $i7, 22

        .macro .flush
        .ascii "XXXX"
        .endm

        .macro .wasmtextlabeldef label
        .set \label, __wasm_blocks
        .endm

        .macro .ndatatextlabel label
        i64.const \label
        .endm

        .macro .ncodetextlabel label
        i64.const \label
        .endm

        .macro .dpc label
        i64.const \label
        .endm

        .macro .labeldef_internal label
        nextcase
        .set \label, __wasm_blocks
        .endm

        .macro rleb128 expr:vararg
        .reloc .,R_ASMJS_LEB128,\expr
        .rept 15
        .byte 0x80
        .endr
        .byte 0x00
        .endm

        .macro lstring str
        rleb128 __lstring_\@\()_end - __lstring_\@
__lstring_\@:
        .ascii "\str"
__lstring_\@\()_end:
        .endm

        .macro defun name, sig:vararg
        .set __wasm_depth, __wasm_blocks_\name\()_sym
        .pushsection .wasm.chars.type
__sigchar_\name\():
        .byte 0
        .popsection
        .pushsection .wasm.payload.type
__signature_\name\():
        signature \sig
        .popsection
        .pushsection .wasm.chars.function
\name\():
        .byte 0
        .popsection
        .pushsection .wasm.chars.code
        .byte 0
        .popsection
        .if 1
        .pushsection .wasm.chars.name
        .byte 0
        .popsection
        .pushsection .wasm.payload.name
        rleb128 __name_\name\()_end - __name_\name
__name_\name:
        .ascii "\name"
__name_\name\()_end:
        .byte 23
        lstring dpc
        lstring sp1
        lstring r0
        lstring r1
        lstring rpc
        lstring pc0
        lstring rp
        lstring fp
        lstring sp
        lstring r2
        lstring r3
        lstring r4
        lstring r5
        lstring r6
        lstring r7
        lstring i0
        lstring i1
        lstring i2
        lstring i3
        lstring i4
        lstring i5
        lstring i6
        lstring i7
        .popsection
        .endif
        .pushsection .wasm.payload.function
        rleb128 __sigchar_\name\()
        .popsection

        .set __wasm_counter, __wasm_counter + 1
        .set __wasm_blocks, 0
        .pushsection .wasm.payload.code,2*__wasm_counter
__body_\name\():
        .popsection

        .pushsection .wasm.payload.code,2*__wasm_counter+1
        .endm

        .macro jump
        br __wasm_depth - __wasm_blocks - 1
        .endm

        .macro throw
        .ascii "count these:"
        .long __wasm_depth - __wasm_blocks
        br __wasm_depth - __wasm_blocks
        .endm

        .macro jump1
        br __wasm_depth - __wasm_blocks
        .endm

        .macro throw1
        br __wasm_depth - __wasm_blocks + 1
        .endm

        .macro endefun name
__body_end_\name\():
        .pushsection .wasm.dummy
        .popsection
        .popsection
        .pushsection .wasm.payload.code,2*__wasm_counter
        rleb128 __body_end_\name - __body_start_\name
__body_start_\name\():
__wasm_locals_\name\():
        .byte 0x01
        .byte 0x7f
        .byte 2
__wasm_locals_\name\()_end:
        .ifne __wasm_blocks
__wasm_ast_\name\():
        loop
__wasm_blocks_\name:
        .rept __wasm_blocks
        block
        .endr
__wasm_blocks_\name\()_end:
        .pushsection .wasm.dummy
        .offset __wasm_blocks
__wasm_blocks_\name\()_sym:
        .popsection
        get_local 0
        i32.wrap_i64
        .byte 0x08
        .byte 0x00
        rleb128 __wasm_blocks-1
        .set __wasm_block, 0
        .rept __wasm_blocks-1
        .4byte __wasm_block
        .set __wasm_block, __wasm_block + 1
        .endr
        .4byte __wasm_blocks-1
        end
        .popsection
        .else
        .pushsection .wasm.dummy
        .offset __wasm_blocks
__wasm_blocks_\name\()_sym:
        .popsection
        .endif
        .endm

        .macro nextcase
        end
        .ifndef __wasm_blocks
        .error "nextcase outside of defun"
        .endif
        .set __wasm_blocks, __wasm_blocks + 1
        .endm

        .macro .Xwasmtextlabeldef label
.LAT\@0:
        .popsection
        .long 0
        .long 0
\label\():
        .long 0
        .long 0
        .pushsection .wasm_pwas%S,"a"
        (label)
        .endm

        .macro .Xwasmtextlabeldeffirst label
.LAT\@0:
        .popsection
\label\():
.L.\()\label:
.LAT\@1:
        .long 0
        .long 0
        .pushsection .wasm_pwas%S,"a"
        .set .L__pc0, .LAT\@1
        .endm

        .macro .Xwasmtextlabeldeflast label
.LAT\@0:
        .popsection
        .long 0
        .long 0
\label\():
        .pushsection .wasm_pwas%S,"a"
        .endm

        .macro .Xrpcr4 a, b
        .ascii ",(hex \""
        .reloc .,R_ASMJS_HEX16R4,\b-\a
        .ascii "0000000000000000"
        .ascii "\")"
        .endm

        .macro .Xdpc label
        .ascii "(i64.const "
        .rpcr4 .L__pc0, \label
        .ascii ")"
        .endm

        .macro .labeldef_debug label
        .pushsection .wasm.dummy
        .offset 0
\label:
        .popsection
        .endm

        .macro .Xtextlabel label
        .reloc .+2,R_ASMJS_HEX16,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .Xcodetextlabel label
        .reloc .+2,R_ASMJS_HEX16,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .Xdatatextlabel label
        .reloc .+2,R_ASMJS_HEX16,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .Xntextlabel label
        .ascii "(i64.const "
        .ascii ",(hex \""
        .reloc .,R_ASMJS_HEX16,\label
        .ascii "0000000000000000"
        .ascii "\")"
        .ascii ")"
        .endm

        .macro .Xncodetextlabel label
        .ascii "(i64.const "
        .ascii ",(hex \""
        .reloc .,R_ASMJS_HEX16,\label
        .ascii "0000000000000000"
        .ascii "\")"
        .ascii ")"
        .endm

        .macro .Xndatatextlabel label
        .ascii "(i64.const "
        .ascii ",(hex \""
        .reloc .,R_ASMJS_HEX16,\label
        .ascii "0000000000000000"
        .ascii "\")"
        .ascii ")"
        .endm

        .macro .Xtextlabelr4 label
        .reloc .+2,R_ASMJS_HEX16R4,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .Xdatatextlabelr4 label
        .reloc .+2,R_ASMJS_HEX16R4,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .Xcodetextlabelr4 label
        .reloc .+2,R_ASMJS_HEX16R4,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .Xcodetextlabelr12 label
        .reloc .+2,R_ASMJS_HEX16R12,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .Xflush
        (set_local $rp (i64.or (get_local $fp) (i64.const 1)))
        (set_local $dpc $
        .dpc .LFl\@
        )
        (br $mainloop)
        .labeldef_internal .LFl\@
        .endm

        .macro .Xcodetextlabeldef label
.LAT\@0:
        .popsection
.LAT\@1:
        .long .LAT\@0
        .long 0
\label\():
        .long .LAT\@0
        .long 0
        .pushsection .wasm_pwas%S,"a"
        (label)
        .endm

        .macro .Xcodetextlabeldeffirst label
.LAT\@0:
        .popsection
\label\():
.L.\()\label:
.LAT\@1:
        .long .LAT\@0
        .long 0
        .pushsection .wasm_pwas%S,"a"
        .set .L__pc0, .LAT\@1
        .rpcr4 .L__pc0, .LAT\@1
        .set __wasm_fallthrough, 1
        .endm

        .macro .Xcodedatalabel label
        .ascii "((global_data|0)+"
        .reloc .+2,R_ASMJS_HEX16,\label
        .ascii "0x0000000000000000"
        .ascii ")"
        .endm

        .set __wasm_fallthrough, 1
