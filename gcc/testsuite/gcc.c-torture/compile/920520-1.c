/* { dg-do compile } */
/* { dg-skip-if "" { pdp11-*-* } } */
/* { dg-skip-if "no SF regs" { wasm32-*-* } { "*" } { "" } } */

f(){asm("%0"::"r"(1.5F));}g(){asm("%0"::"r"(1.5));}
