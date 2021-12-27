#NO_APP
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

        .macro .datatextlabel label
        .reloc .+2,R_ASMJS_HEX16,\label
        .ascii "0x0000000000000000"
        .endm

        .macro .codetextlabel label
        .reloc .+2,R_ASMJS_HEX16,\label
        .ascii "0x0000000000000000"
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

        .macro .rpcr4 a, b
        .reloc .+2,R_ASMJS_HEX16R4,\b-\a
        .ascii "0x0000000000000000"
        .endm

        .macro .dpc label
        .rpcr4 .L__pc0, \label
        .endm

        .macro .flush
        rp = fp|1;
        dpc = $
        .dpc .LFl\@
        ;
        break mainloop;
        .labeldef_internal .LFl\@
        .endm

        .macro .textlabeldef label
.LAT\@0:
        case $
        .popsection
\label\():
.LAT\@1:
        .long .LAT\@0
        .long 0
        .long 0
        .long 0
        .pushsection .javascript%S,"a"
        .dpc .LAT\@1
        .ascii ":\n"
        .endm

        .macro .codetextlabeldef label
.LAT\@0:
        .if __asmjs_fallthrough
        .if __gcc_pc_update
        pc = $
        .textlabelr4 .LAT\@1+8
        ;
        .endif
        .ifge __gcc_pc_update-2
        continue;
        .endif
        .endif
        .ascii "case "
        .popsection
.LAT\@1:
        .long .LAT\@0
        .long 0
\label\():
        .long .LAT\@0
        .long 0
        .pushsection .javascript%S,"a"
        .dpc .LAT\@1+8
        .ascii ":\n"
        .endm

        .macro .codetextlabeldeffirst label
.LAT\@0:
        .ascii "case "
        .popsection
\label\():
.L.\()\label:
.LAT\@1:
        .long .LAT\@0
        .long 0
        .pushsection .javascript%S,"a"
        .set .L__pc0, .LAT\@1
        .rpcr4 .L__pc0, .LAT\@1
        .ascii ":\n"
        .set __asmjs_fallthrough, 1
        .endm

        .macro .codetextlabeldeflast label
.LAT\@0:
        .popsection
.LAT\@1:
        .long .LAT\@0
        .long 0
\label\():
        .pushsection .javascript%S,"a"
        .set __asmjs_fallthrough, 1
        .endm

        .macro .codedatalabel label
        .ascii "((global_data|0)+"
        .reloc .+2,R_ASMJS_HEX16,\label
        .ascii "0x0000000000000000"
        .ascii ")"
        .endm

        .macro .cont_or_break label
        .ifdef \label
        break;
        .else
        continue;
        .endif
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

        .set __asmjs_fallthrough, 1
