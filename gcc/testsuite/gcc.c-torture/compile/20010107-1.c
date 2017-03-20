/* { dg-xfail-if "separate code/data spaces" { wasm32-*-* } { "*" } { "" } } */

unsigned long x[4];

void foo(void)
{
  ((void (*)())(x+2))();
}
