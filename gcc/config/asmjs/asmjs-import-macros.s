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
        .pushsection .javascript%S,"a"
function f_$
        .codetextlabel \name
        /*
        \name
        */
    (dpc,sp1,r0,r1,rpc,pc0) {
        dpc = dpc|0;
        sp1 = sp1|0;
        r0 = r0|0;
        r1 = r1|0;
        rpc = rpc|0;
        pc0 = pc0|0;
        var pc = 0;
        var fp = 0;
        var rp = 0;
        var a = 0;
        var sp = 0;
        pc = (dpc+pc0)|0;
        sp = (sp1-16)|0;
        mainloop:
        while(1) {
        switch (dpc|0) {
        .codetextlabeldeffirst \name
        .labeldef_debug .LFB0
        .cfi_startproc
        sp = (sp|0)-24|0;
        fp = sp|0;
        rp = fp|1;
        dpc = $
        .dpc \name\().0
        ;
        break mainloop;
        .codetextlabeldef \name\().0
        rp = foreign_extcall(
                .datatextlabel __str_\module
                |0,
                .datatextlabel __str_\name
                |0, pc|0, sp+24|0)|0;
        if (rp & 3) {
            rp = fp|1;
            dpc = $
            .dpc \name\().0
            ;
            break mainloop;
        } else {
            rp = fp|1;
            dpc = $
            .dpc \name\().1
            ;
            break mainloop;
        }
        .codetextlabeldef \name\().1
        return fp+24|0;
        .set __asmjs_fallthrough, 0
        .cfi_endproc
        .labeldef_debug .LFE0
        .codetextlabeldeflast .ends.\name
 default:
        if ((dpc+pc0)|0 == 0) {
            rp = sp|0;
            pc0 = HEAP32[rp+4>>2]>>4;
            pc = HEAP32[rp+8>>2]>>4 ;
            dpc = (pc - pc0)|0;
            rpc = HEAP32[rp+12>>2]|0;
            sp = HEAP32[rp+16>>2]|0;
            fp = rp|0;
            continue;
        } else {
            foreign_abort(0, dpc|0, pc0|0, 0, 0);
        }
        }
        {
        bogotics = (bogotics|0)-1|0;
        a = ((bp_hit)|0) | ((bogotics|0)<0);
        if ((a|0) != 0) {
                if (fp|0) {
                        rp = fp|1;
                        break mainloop;
                }
        }
        }
        }
        if ((rp & 3) == 1) {
        HEAP32[fp+0>>2] = (fp+24)|0;
        HEAP32[fp+4>>2] = pc0<<4;
        HEAP32[fp+8>>2] = (pc0+dpc)<<4;
        HEAP32[fp+12>>2] = rpc|0;
        HEAP32[fp+16>>2] = sp|0;
        HEAP32[fp+20>>2] = 0x00000000;
        }
        return rp|0;}

        .popsection
        .p2align 4+8

        .pushsection .special.export,"a"
        .pushsection .javascript%S,"a"
        .ascii "f_"
        .codetextlabel \name
        .ascii ": f_"
        .codetextlabel \name
        .ascii ",\n"
        .popsection
        .popsection
        .pushsection .special.define,"a"
        .pushsection .javascript%S,"a"
        .ascii "\tdeffun({na"
        .ascii "me: \"f_"
        .codetextlabel \name
        .ascii "\", symbol: \"\name\", pc0: "
        .codetextlabel \name
        .ascii ", pc1: "
        .codetextlabel .ends.\name
        .ascii "});\n"
        .popsection
        .popsection
        .endm

