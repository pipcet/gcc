/* { dg-do compile } */
/* { dg-options "-O2 -mno-mpx -mno-sse -mno-80387 -mmmx -mno-cld -mno-iamcu" } */

void
__attribute__((no_caller_saved_registers))
fn1 (void)
{ /* { dg-message "MMX/3Dnow instructions aren't allowed in function with no_caller_saved_registers attribute" } */
}
