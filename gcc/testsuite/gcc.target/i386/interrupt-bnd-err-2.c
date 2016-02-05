/* { dg-do compile { target { ! x32 } } } */
/* { dg-options "-O2 -mno-sse -mno-mmx -mno-80387 -mno-cld -mno-iamcu -mmpx" } */

void
__attribute__((no_caller_saved_registers))
fn (void *frame)
{ /* { dg-message "MPX instructions aren't allowed in function with no_caller_saved_registers attribute" } */
}
