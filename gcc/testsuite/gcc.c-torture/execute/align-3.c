/* { dg-skip-if "small alignment" { pdp11-*-* } } */
/* { dg-skip-if "no alignment of functions" { wasm32-*-* } } */

void func(void) __attribute__((aligned(256)));

void func(void) 
{
}

int main()
{
  if (__alignof__(func) != 256)
    abort ();
  return 0;
}
