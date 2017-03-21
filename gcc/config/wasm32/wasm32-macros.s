#NO_APP
        .local __wasm_counter
        .local __wasm_block
        .local __wasm_blocks
        .local __wasm_depth
        .local __wasm_in_defun
        .set __wasm_blocks, 0
        .set __wasm_block, 0
        .set __wasm_depth, 0
        .set __wasm_in_defun, 0
        .local $dpc, $sp1, $r0, $r1, $rpc, $pc0
        .local $rp, $fp, $sp
        .local $r2, $r3, $r4, $r5, $r6, $r7
        .local $i0, $i1, $i2, $i3, $i4, $i5, $i6, $i7
        .local $f0, $f1, $f2, $f3, $f4, $f5, $f6, $f7
        .local $rv, $a0, $a1, $a2, $a3, $tp
        .set __wasm_counter, 0
        ;; local/"register" names. The first six are arguments.
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
        .set $f7, 30
        ;; in-memory per-thread globals
        .set $rv, 4096
        .set $a0, 4104
        .set $a1, 4112
        .set $a2, 4120
        .set $a3, 4128
        .set $tp, 8192
        ;; per-instance get_global/set_global globals
        .set $got, 0
        .set $plt, 1
        .set $gpo, 2

        .macro .flush
        .dpc .LFl\@
        set_local $dpc
        get_local $fp
        i32.const 1
        i32.or
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
        .previous
        .pushsection .space.pc.%S
\label:
        .popsection
        .previous
        .endm

        .macro .ndatatextlabel label
        i32.const \label
        .endm

        .macro .ncodetextlabel label
        i32.const \label
        .endm

        .macro .dpc label
        i32.const \label - __wasm_pc_base
        .endm

        .macro .labeldef_internal label
        nextcase
        .previous
        .pushsection .space.pc.%S
\label:
        .popsection
        .previous
        .endm

        .macro rleb128 expr:vararg
        .reloc .,R_WASM32_LEB128,\expr
        .rept 15
        .byte 0x80
        .endr
        .byte 0x00
        .endm

        .macro rleb128_64 expr:vararg
        .reloc .,R_WASM32_LEB128,\expr
        .rept 9
        .byte 0x80
        .endr
        .byte 0x00
        .endm

        .macro rleb128_32 expr:vararg
        .reloc .,R_WASM32_LEB128,\expr
        .rept 4
        .byte 0x80
        .endr
        .byte 0x00
        .endm

        .macro rleb128_8 expr:vararg
        .reloc .,R_WASM32_LEB128,\expr
        .rept 1
        .byte 0x80
        .endr
        .byte 0x00
        .endm

        .macro rleb128_1 expr:vararg
        .reloc .,R_WASM32_LEB128,\expr
        .byte 0x00
        .endm

        .macro lstring str
        rleb128_32 2f - 1f
1:
        .ascii "\str"
2:
        .endm

        .macro createsig sig
        .ifndef __sigchar_\sig
        .pushsection .space.type,"G",@progbits,__sigchar_\sig,comdat
        .weak __sigchar_\sig
__sigchar_\sig:
        .byte 0
        .size __sigchar_\sig, . - __sigchar_\sig
        .popsection
        .pushsection .wasm.type,"G",@progbits,__sigchar_\sig,comdat
        signature \sig
        .popsection
        .endif
        .endm

        ;; input section stack:
        ;; .wasm.code..text       <--      top
        ;; .text
        ;;     OR
        ;; .wasm.code..text.f     <--      top
        ;; .text.f
        .macro defun name, sig:vararg
        .set __wasm_in_defun, 1
        createsig \sig
        .local __wasm_body_blocks_\name\()_sym
        .set __wasm_depth, __wasm_body_blocks_\name\()_sym
        .previous
        ;; right now, .space.function.* and .space.code.* are
        ;; equivalent, and neither index is actually used anywhere.
        .pushsection .space.function.%S
        .reloc .,R_WASM32_CODE_POINTER,__wasm_function_\name
        .reloc .,R_WASM32_INDEX,\name
5:
        .byte 0
        .popsection
        .pushsection .space.code.%S,"x"
        .reloc .,R_WASM32_CODE_POINTER,__wasm_code_\name
        .reloc .,R_WASM32_INDEX,\name
6:
        .byte 0
        .popsection
        .pushsection .space.element.%S
        .reloc .,R_WASM32_CODE_POINTER,__wasm_element_\name
        .reloc .,R_WASM32_INDEX,\name
7:
        .byte 0
        .popsection
        .pushsection .wasm.element.%S
__wasm_element_\()\name:
        rleb128_32 \name
        .popsection
        .pushsection .space.function_index.%S,"x"
        .type \name, @function
        .size \name, 1
        .reloc .,R_WASM32_INDEX,5b
        .reloc .,R_WASM32_INDEX,6b
        .reloc .,R_WASM32_INDEX,7b
        .reloc .,R_WASM32_INDEX,__wasm_name_local_\name
        .reloc .,R_WASM32_INDEX,__wasm_name_function_\name
\name\():
        .byte 0x00
        .set __wasm_function_index, \name\()
        .popsection
        .pushsection .space.pc.%S,""
        .set __wasm_pc_base, .
        .set __wasm_pc_base_\()\name, .
        .popsection
        .if 0
        .ifeqs "\name","main"
        .pushsection .space.export
        .byte 0
        .popsection
        .pushsection .wasm.export
        lstring \name
        .byte 0
        rleb128_32 \name
        .popsection
        .endif
        .endif
        .ifeqs "\name","_start"
        .pushsection .space.global
        .byte 0
        .popsection
        .pushsection .wasm.global
        .byte 0x7f
        .byte 0
        .byte 0x41
        rleb128_32 \name
        .byte 0x0b
        .pushsection .space.export
        .byte 0
        .popsection
        .pushsection .wasm.export
        lstring "entry"
        .byte 3
        .byte 3
        .popsection
        .endif
        .if 0
        .pushsection .space.export
        .byte 0
        .popsection
        .pushsection .wasm.export
        rleb128_8 18
        .ascii "f_"
        .reloc .,R_WASM32_HEX16,\name
        .ascii "0000000000000000"
        .byte 0
        rleb128_32 \name
        .popsection
        .endif
        .if 1
        .pushsection .space.name.function.%S
__wasm_name_function_\name:
        .reloc .,R_WASM32_CODE_POINTER,__wasm_name_function2_\name
        .reloc .,R_WASM32_INDEX,\name
        .byte 0
        .popsection
        .pushsection .wasm.name.function.%S
__wasm_name_function2_\name:
        rleb128_32 \name
        lstring \name
        .popsection
        .pushsection .space.name.local.%S
__wasm_name_local_\name:
        .reloc .,R_WASM32_CODE_POINTER,__wasm_name_local2_\name
        .reloc .,R_WASM32_INDEX,\name
        .byte 0
        .popsection
        .pushsection .wasm.name.local.%S
__wasm_name_local2_\name:
        rleb128_32 \name
        .byte 31
        .byte 0
        lstring dpc
        .byte 1
        lstring sp1
        .byte 2
        lstring r0
        .byte 3
        lstring r1
        .byte 4
        lstring rpc
        .byte 5
        lstring pc0
        .byte 6
        lstring rp
        .byte 7
        lstring fp
        .byte 8
        lstring sp
        .byte 9
        lstring r2
        .byte 10
        lstring r3
        .byte 11
        lstring r4
        .byte 12
        lstring r5
        .byte 13
        lstring r6
        .byte 14
        lstring r7
        .byte 15
        lstring i0
        .byte 16
        lstring i1
        .byte 17
        lstring i2
        .byte 18
        lstring i3
        .byte 19
        lstring i4
        .byte 20
        lstring i5
        .byte 21
        lstring i6
        .byte 22
        lstring i7
        .byte 23
        lstring f0
        .byte 24
        lstring f1
        .byte 25
        lstring f2
        .byte 26
        lstring f3
        .byte 27
        lstring f4
        .byte 28
        lstring f5
        .byte 29
        lstring f6
        .byte 30
        lstring f7
        .popsection
        .endif
        .pushsection .wasm.function.%S
__wasm_function_\name\():
        rleb128_32 __sigchar_\sig
        .popsection

        .set __wasm_counter, __wasm_counter + 1
        .set __wasm_blocks, 0
        .popsection
        .pushsection .wasm.code.%S,2*__wasm_counter+1
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
        br __wasm_depth - __wasm_blocks + 1
        .endm

        .macro jump2
        br __wasm_depth - __wasm_blocks - 1
        .endm

        .macro endefun name
        .local __wasm_body_header_\name\()
        .local __wasm_body_ast_\name\()
2:
        .popsection
        .pushsection .wasm.code.%S,2*__wasm_counter

__wasm_body_header_\name\():
        .type __wasm_body_header_\name\(), @object
        rleb128_32 2b - 1f
1:
        .byte 0x02
        .byte 17
        .byte 0x7f
        .byte 8
        .byte 0x7c
        .size __wasm_body_header_\name\(), . - __wasm_body_header_\name\()
__wasm_body_ast_\name\():
        .ifne __wasm_blocks
        i32.const -16
        get_local $sp1
        i32.add
        set_local $sp
        .endif
        .ifne __wasm_blocks
        .local __wasm_body_blocks_\name\()
        .local __wasm_body_blocks_\name\()_sym
        block[]
        loop[]
__wasm_body_blocks_\name:
        .rept __wasm_blocks
        block[]
        .endr
        .pushsection .wasm.dummy
        .offset __wasm_blocks
__wasm_body_blocks_\name\()_sym:
        .popsection
        .if 0
        i32.const 0
        i32.load a=2 0
        if[]
        i32.const -1
        get_local $dpc
        i32.ne
        i32.const 0
        get_local $dpc
        i32.ne
        i32.and
        if[]
        i32.const 1
        get_local $sp
        i32.or
        set_local $rp
        br __wasm_blocks+3
        end
        end
        .endif
        get_local $dpc
        .byte 0x0e
        rleb128_32 __wasm_blocks-1
        rleb128_32 __wasm_blocks-1
        .set __wasm_block, 1
        .rept __wasm_blocks-2
        rleb128_32 __wasm_block
        .set __wasm_block, __wasm_block + 1
        .endr
        rleb128_32 0
        end
        .else
        .pushsection .wasm.dummy
        .offset __wasm_blocks
__wasm_body_blocks_\name\()_sym:
        .popsection
        .endif
        .set __wasm_in_defun, 0
        .endm

        .macro nextcase
        .ifne __wasm_in_defun
        .dpc 1f
        set_local $dpc
        jump
        end
        .set __wasm_blocks, __wasm_blocks + 1
        .previous
        .pushsection .space.pc.%S
        .byte 0x00
1:
        .popsection
        .previous
        .endif
        .endm

        .ifne 0
        .macro .labeldef_debug label
        .previous
        .ifndef __wasm_function_index
        .pushsection .space.function_index.%S,"x"
0:
        .popsection
        .set __wasm_function_index, 0b
        .endif
        .ifndef __wasm_blocks
        .set __wasm_blocks, 0
        .endif
        .pushsection .space.pc.%S
\label:
        .popsection
        .previous
        .endm
        .else
        .macro .labeldef_debug label
        .labeldef_internal \label
        .endm
        .endif
        .set __wasm_fallthrough, 1
