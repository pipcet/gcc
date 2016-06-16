#NO_APP
        .macro .wasmtextlabeldef label
.LAT\@0:
        .popsection
        .long 0
        .long 0
\label\():
        .long 0
        .long 0
        .pushsection .javascript%S,"a"
        (label)
        .endm

        .macro .wasmtextlabeldeffirst label
.LAT\@0:
        .popsection
\label\():
.LAT\@1:
        .long 0
        .long 0
        .pushsection .javascript%S,"a"
        .set .L__pc0, .LAT\@1
        .endm

        .macro .wasmtextlabeldeflast label
.LAT\@0:
        .popsection
        .long 0
        .long 0
\label\():
        .pushsection .javascript%S,"a"
        .endm

        .macro .rpcr4 a, b
        .ascii ",(string->number (string-delete \""
        .reloc .,R_ASMJS_HEX16R4,\b-\a
        .ascii "0000000000000000"
        .ascii "\" #\\ 0) 16)"
        .endm

        .macro .dpc label
        .rpcr4 .L__pc0, \label
        .endm

        .macro .labeldef_debug label
        .popsection
        .set \label, . - 8
        .pushsection .javascript%S,"a"
        .endm

        .macro .labeldef_internal label
        .codetextlabeldef \label
        .endm

        .macro .textlabel label
        .reloc .+2,R_ASMJS_HEX16,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .codetextlabel label
        .reloc .+2,R_ASMJS_HEX16,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .datatextlabel label
        .reloc .+2,R_ASMJS_HEX16,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .ntextlabel label
        .ascii "(i32.const "
        .ascii ",(string->number (string-delete \""
        .reloc .,R_ASMJS_HEX16,\label
        .ascii "0000000000000000"
        .ascii "\" #\\ 0) 16)"
        .ascii ")"
        .endm

        .macro .ncodetextlabel label
        .ascii "(i32.const "
        .ascii ",(string->number (string-delete \""
        .reloc .,R_ASMJS_HEX16,\label
        .ascii "0000000000000000"
        .ascii "\" #\\ 0) 16)"
        .ascii ")"
        .endm

        .macro .ndatatextlabel label
        .ascii "(i32.const "
        .ascii ",(string->number (string-delete \""
        .reloc .,R_ASMJS_HEX16,\label
        .ascii "0000000000000000"
        .ascii "\" #\\ 0) 16)"
        .ascii ")"
        .endm

        .macro .textlabelr4 label
        .reloc .+2,R_ASMJS_HEX16R4,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .datatextlabelr4 label
        .reloc .+2,R_ASMJS_HEX16R4,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .codetextlabelr4 label
        .reloc .+2,R_ASMJS_HEX16R4,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .codetextlabelr12 label
        .reloc .+2,R_ASMJS_HEX16R12,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .flush
        (set_local $rp (i32.or (get_local $fp) (i32.const 1)))
        (set $dpc $
        .dpc .LFl\@
        )
        (br $mainloop)
        .labeldef_internal .LFl\@
        .endm

        .macro .textlabeldef label
.LAT\@0:
        .popsection
\label\():
.LAT\@1:
        .long .LAT\@0
        .long 0
        .long 0
        .long 0
        .pushsection .javascript%S,"a"
        .dpc .LAT\@1
        .endm

        .macro .codetextlabeldef label
.LAT\@0:
        .if __wasm_fallthrough
        .if __gcc_pc_update
        pc = $
        .textlabelr4 .LAT\@1+8
        ;
        .endif
        .ifge __gcc_pc_update-2
        continue;
        .endif
        .endif
        .popsection
.LAT\@1:
        .long .LAT\@0
        .long 0
\label\():
        .long .LAT\@0
        .long 0
        .pushsection .javascript%S,"a"
        (label)
        .endm

        .macro .codetextlabeldeffirst label
.LAT\@0:
        .popsection
\label\():
.L.\()\label:
.LAT\@1:
        .long .LAT\@0
        .long 0
        .pushsection .javascript%S,"a"
        .set .L__pc0, .LAT\@1
        .rpcr4 .L__pc0, .LAT\@1
        .set __wasm_fallthrough, 1
        .endm

        .macro .codetextlabeldeflast label
.LAT\@0:
        .popsection
.LAT\@1:
        .long .LAT\@0
        .long 0
\label\():
        .pushsection .javascript%S,"a"
        .set __wasm_fallthrough, 1
        .endm

        .macro .codedatalabel label
        .ascii "((global_data|0)+"
        .reloc .+2,R_ASMJS_HEX16,\label
        .ascii "0x0000000000000000"
        .ascii ")"
        .endm

        .macro .lbranch label
        .if __gcc_bogotics_backwards
        .popsection
        .ifle label-.
        .pushsection .javascript%S,"ax"
        .endif
        .pushsection .javascript%S,"ax"
        .endif
        .endm

        .set __wasm_fallthrough, 1
