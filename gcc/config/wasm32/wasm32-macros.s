#NO_APP
        .local __wasm_counter
        .local __wasm_block
        .local __wasm_blocks
        .local __wasm_depth
        .local $dpc, $sp1, $r0, $r1, $rpc, $pc0
        .local $rp, $fp, $sp
        .local $r2, $r3, $r4, $r5, $r6, $r7
        .local $i0, $i1, $i2, $i3, $i4, $i5, $i6, $i7
        .local $f0, $f1, $f2, $f3, $f4, $f5, $f6
        .local $rv, $a0, $a1, $a2, $a3, $tp
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
        .set $f0, 23
        .set $f1, 24
        .set $f2, 25
        .set $f3, 26
        .set $f4, 27
        .set $f5, 28
        .set $f6, 29
        .eqv $f7, 30

        .set $rv, 4096
        .set $a0, 4104
        .set $a1, 4112
        .set $a2, 4120
        .set $a3, 4128
        .set $tp, 8192

        .macro .flush
        i64.const .LFl\@
        set_local $dpc
        get_local $fp
        i64.const 1
        i64.or
        set_local $rp
        throw
        .wasmtextlabeldpcdef .LFl\@
        .endm

        .macro .wasmtextlabelpc0def label
        nextcase
        .set \label, __wasm_function_index
        .endm

        .macro .wasmtextlabeldpcdef label
        nextcase
        .set \label, __wasm_blocks
        .endm

        .macro .ndatatextlabel label
        i32.const \label
        .endm

        .macro .ncodetextlabel label
        i32.const \label
        .endm

        .macro .dpc label
        i32.const \label
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
        rleb128 2f - 1f
1:
        .ascii "\str"
2:
        .endm

        .macro createsig sig
        .ifndef __sigchar_\sig
        .pushsection .wasm.chars.type,"G",@progbits,__sigchar_\sig,comdat
        .weak __sigchar_\sig
__sigchar_\sig:
        .byte 0
        .size __sigchar_\sig, . - __sigchar_\sig
        .popsection
        .pushsection .wasm.payload.type,"G",@progbits,__sigchar_\sig,comdat
        signature \sig
        .popsection
        .endif
        .endm

        .macro defun name, sig:vararg
        createsig \sig
        .local __wasm_blocks_\name\()_sym
        .local __name_\name, __name_\name\()_end
        .set __wasm_depth, __wasm_blocks_\name\()_sym
        .pushsection .wasm.chars.function
        .byte 0
        .popsection
        .pushsection .wasm.chars.function_index.b,""
\name\():
        .byte 0x5a
        .set __wasm_function_index, \name\()
        .popsection
        .pushsection .wasm.chars.export
        .byte 0
        .popsection
        .pushsection .wasm.payload.export
        lstring \name
        .byte 0
        rleb128 \name
        .popsection
        .pushsection .wasm.chars.code
        .byte 0
        .popsection
        .if 1
        .pushsection .wasm.chars.name
        .byte 0
        .popsection
        .pushsection .wasm.payload.name
        lstring \name
        .byte 31
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
        lstring f0
        lstring f1
        lstring f2
        lstring f3
        lstring f4
        lstring f5
        lstring f6
        lstring f7
        .popsection
        .endif
        .pushsection .wasm.payload.function
        rleb128 __sigchar_\sig
        .popsection

        .set __wasm_counter, __wasm_counter + 1
        .set __wasm_blocks, 0
        .pushsection .wasm.payload.code,2*__wasm_counter+1
__wasm_code_\name\():
        .endm

        .macro jump
        br __wasm_depth - __wasm_blocks - 1
        .endm

        .macro throw
        br __wasm_depth - __wasm_blocks
        .endm

        .macro jump1
        br __wasm_depth - __wasm_blocks
        .endm

        .macro throw1
        br __wasm_depth - __wasm_blocks
        .endm

        .macro endefun name
        .local __wasm_locals_\name\()
        .local __wasm_locals_\name\()_end
        .local __wasm_ast_\name\()
        .local __wasm_blocks_\name\()
        .local __wasm_blocks_\name\()_end
        .local __wasm_blocks_\name\()_sym
2:
        .pushsection .wasm.dummy
        .popsection
        .popsection
        .pushsection .wasm.payload.code,2*__wasm_counter
        rleb128 2b - 1f
1:
__wasm_locals_\name\():
        .byte 0x02
        .byte 17
        .byte 0x7f
        .byte 8
        .byte 0x7c
__wasm_locals_\name\()_end:
        i32.const -16
        get_local $sp1
        i32.add
        set_local $sp
        .ifne __wasm_blocks
__wasm_ast_\name\():
        loop[]
__wasm_blocks_\name:
        .rept __wasm_blocks
        block[]
        .endr
__wasm_blocks_\name\()_end:
        .pushsection .wasm.dummy
        .offset __wasm_blocks
__wasm_blocks_\name\()_sym:
        .popsection
        get_local $dpc
        .byte 0x0e
        rleb128 __wasm_blocks-1
        .set __wasm_block, 0
        .rept __wasm_blocks-1
        rleb128 __wasm_block
        .set __wasm_block, __wasm_block + 1
        .endr
        rleb128 __wasm_blocks-1
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

        .ifne 1
        .macro .labeldef_debug label
        .ifndef __wasm_function_index
        .pushsection .wasm.chars.function_index.b,""
0:
        .byte 0xa5
        .popsection
        .set __wasm_function_index, 0b
        .endif
        .ifndef __wasm_blocks
        .set __wasm_blocks, 0
        .endif
        .set \label, __wasm_blocks + 1
        .endm
        .else
        .macro .labeldef_debug label
        .labeldef_internal \label
        .endm
        .endif
        .set __wasm_fallthrough, 1
