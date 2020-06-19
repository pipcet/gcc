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
        .local $rv
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
        .set $rv, 8288
        ;; per-instance immutable global.get globals
        .set $got, 0
        .set $plt, 1
        .set $gpo, 2

        .macro text_section
        .text
	.ifdef __wasm_counter
	.pushsection .wasm.code.%S,2*__wasm_counter+1,"ax"
	.else
	.pushsection .wasm.code.%S,"ax"
	.endif
	ensure_text_section
	.endm

	.macro ensure_text_section
	.previous
	.pushsection .space.name.function.%S,"a"
	.popsection
	.pushsection .space.name.local.%S,"a"
	.popsection
	.pushsection .wasm.name.function.%S,"a"
	.popsection
	.pushsection .wasm.name.local.%S,"a"
	.popsection
	.pushsection .wasm.function.%S,"a"
	.popsection
	.pushsection .space.pc.%S,"a"
	.popsection
	.pushsection .space.code.%S,"x"
	.popsection
	.pushsection .space.function_index.%S,"x"
	.popsection
	.pushsection .space.function.%S,"x"
	.popsection
	.pushsection .space.element.%S,"x"
	.popsection
	.pushsection .wasm.element.%S,"x"
	.popsection
	.previous
        .endm

        .macro .flush
        .dpc .LFl\@
        local.set $dpc
        local.get $fp
        i32.const 1
        i32.or
        local.set $rp
        throw
        .wasmtextlabeldpcdef .LFl\@
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
        .macro defun name, sig, raw = 0
	ensure_text_section
        .set __wasm_in_defun, 1
        createsig \sig
        .local __wasm_body_blocks_\name\()_sym
        .set __wasm_depth, __wasm_body_blocks_\name\()_sym
        .previous
        ;; right now, .space.function.* and .space.code.* are
        ;; equivalent, and neither index is actually used anywhere.
        .pushsection .space.function.%S
        .reloc .,R_WASM32_CODE_POINTER,__wasm_function__\name
        .reloc .,R_WASM32_INDEX,\name
5:
        .byte 0
        .popsection
        .pushsection .space.code.%S,"x"
        .reloc .,R_WASM32_CODE_POINTER,__wasm_code_\name
        .reloc .,R_WASM32_INDEX,__wasm_local_name_\()\name
6:
        .byte 0
        .popsection
        .pushsection .space.element.%S
        .reloc .,R_WASM32_CODE_POINTER,__wasm_element_\name
        .reloc .,R_WASM32_INDEX,__wasm_local_name_\()\name
7:
        .byte 0
        .popsection
        .pushsection .wasm.element.%S
__wasm_element_\()\name:
        rleb128_32 __wasm_local_name_\()\name
        .popsection
        .pushsection .space.function_index.%S,"x"
        .type \name, @function
        .size \name, 1
        .reloc .,R_WASM32_INDEX,5b
        .reloc .,R_WASM32_INDEX,6b
        .reloc .,R_WASM32_INDEX,7b
        .ifeq \raw
        .reloc .,R_WASM32_INDEX,__wasm_name_local_\name
        .endif
        .reloc .,R_WASM32_INDEX,__wasm_name_function_\name
\name\():
__wasm_local_name_\()\name:
        .byte 0x00
        .set __wasm_function_index, __wasm_local_name_\name\()
        .popsection
        .pushsection .space.pc.%S,""
        .set __wasm_pc_base, .
        .set __wasm_pc_base_\()\name, .
        .popsection
        .ifeqs "\name","_start"
        .pushsection .space.global_index
3:
        .byte 0
        .popsection
        .pushsection .space.global
        .byte 0
        .popsection
        .pushsection .wasm.global
        .byte 0x7f		; type i32
        .byte 0			; not mutable
        .byte 0x41		; i32.const
        rleb128_32 __wasm_local_name_\name	; value
        .byte 0x0b		; end of block
	.popsection
        .pushsection .space.export
        .byte 0
        .popsection
        .pushsection .wasm.export
        lstring "entry"
        .byte 3 		; global
        rleb128_32 3b		; index of this global
        .popsection
        .endif
        .if 1
        .pushsection .space.name.function.%S
__wasm_name_function_\name:
        .reloc .,R_WASM32_CODE_POINTER,__wasm_name_function2_\name
        .reloc .,R_WASM32_INDEX,__wasm_local_name_\name
        .byte 0
        .popsection
        .pushsection .wasm.name.function.%S
__wasm_name_function2_\name:
        rleb128_32 __wasm_local_name_\name
        lstring \name
        .popsection
        .ifeq \raw
        .pushsection .space.name.local.%S
__wasm_name_local_\name:
        .reloc .,R_WASM32_CODE_POINTER,__wasm_name_local2_\name
        .reloc .,R_WASM32_INDEX,\name
        .byte 0
        .popsection
        .pushsection .wasm.name.local.%S
__wasm_name_local2_\name:
        rleb128_32 __wasm_local_name_\name
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
        .endif
        .pushsection .wasm.function.%S
__wasm_function__\name\():
        rleb128_32 __sigchar_\sig
        .popsection

        .set __wasm_counter, __wasm_counter + 1
        .set __wasm_blocks, 0
        .popsection
        .pushsection .wasm.code.%S,2*__wasm_counter+1,"ax"
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

        .macro function_header i, f, l, d
        .byte -((\i != 0) + (\f != 0) + (\l != 0) + (\d != 0))
        .if \i
        rleb128_32 \i
        .byte 0x7f
        .endif
        .if \l
        rleb128_32 \l
        .byte 0x7e
        .endif
        .if \f
        rleb128_32 \f
        .byte 0x7d
        .endif
        .if \d
        rleb128_32 \d
        .byte 0x7c
        .endif
        .endm

        .macro endefun name, raw = 0, ints = 17, floats = 8
        .local __wasm_body_header_\name\()
        .local __wasm_body_ast_\name\()
2:
        .popsection
        .pushsection .wasm.code.%S,2*__wasm_counter,"ax"

__wasm_body_header_\name\():
        .type __wasm_body_header_\name\(), @object
        rleb128_32 2b - 1f
1:
        function_header \ints, 0, 0, \floats
        .size __wasm_body_header_\name\(), . - __wasm_body_header_\name\()
__wasm_body_ast_\name\():
        .ifne __wasm_blocks
        i32.const -16
        local.get $sp1
        i32.add
        local.set $sp
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
        .set __wasm_body_blocks_\name\()_sym, __wasm_blocks
        local.get $dpc
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
        .set __wasm_body_blocks_\name\()_sym, __wasm_blocks
        .endif
        .set __wasm_in_defun, 0
        .endm

        .macro nextcase
        .ifne __wasm_in_defun
        .dpc 1f
        local.set $dpc
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

        .macro import_function symbol, module, field, sigsym
        .pushsection .space.import
        .reloc ., R_WASM32_CODE_POINTER, 0f
        .byte 0
        .popsection
        .pushsection .wasm.import
0:      lstring \module
        lstring \field
        .byte 0
        rleb128 \sigsym
        .popsection
        .pushsection .space.function_index.import,""
        .type \symbol, @function
        .size \symbol, 1
\symbol:
        .reloc ., R_WASM32_INDEX, 1f
        .byte 0
        .popsection
        .pushsection .space.element.import
        .reloc ., R_WASM32_CODE_POINTER, 0f
1:
        .byte 0
        .popsection
        .pushsection .wasm.element.import
0:
        rleb128_32 \symbol
        .popsection
        .endm

        .macro import_global symbol, module, field, mutable
        .pushsection .space.import
        .reloc ., R_WASM32_CODE_POINTER, 0f
        .byte 0
        .popsection
        .pushsection .wasm.import
0:
        lstring \module
        lstring \field
        .byte 3
        .byte 0x7f              ; fixed at i32 for now
        .byte \mutable
        .popsection
        .pushsection .space.global_index.import
        .type __wasm_import_global_\symbol, @object
        .size __wasm_import_global_\symbol, 1
__wasm_import_global_\symbol:
        .byte 0
        .popsection
        .endm

        .macro export_function symbol, field
        .pushsection .space.export
        .byte 0
        .popsection
        .pushsection .wasm.export
        lstring \field
        .byte 0
        rleb128_32 \symbol
        .popsection
        .endm

        .macro export_global symbol, field
        .pushsection .space.global
        .byte 0
        .popsection
        .pushsection .space.global_index
0:      .byte 0
        .popsection
        .pushsection .wasm.global
        .byte 0x7f
        .byte 0
        i32.const \symbol
        end
        .popsection
        .pushsection .space.export
        .byte 0
        .popsection
        .pushsection .wasm.export
        lstring \field
        .byte 3
        rleb128_32 0b
        .endm
