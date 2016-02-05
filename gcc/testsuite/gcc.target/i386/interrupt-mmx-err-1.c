/* { dg-do compile } */
/* { dg-options "-O2 -mno-mpx -mno-sse -mno-80387 -mmmx -mno-cld -mno-iamcu" } */

typedef unsigned int uword_t __attribute__ ((mode (__word__)));

void
__attribute__((interrupt))
fn1 (void *frame)
{ /* { dg-message "MMX/3Dnow instructions aren't allowed in interrupt service routine" } */
}

void
__attribute__((interrupt))
fn2 (void *frame, uword_t error)
{ /* { dg-message "MMX/3Dnow instructions aren't allowed in exception service routine" } */
}
