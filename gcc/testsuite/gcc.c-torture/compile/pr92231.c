/* PR middle-end/92231 */
/* { dg-skip-if "cannot call data pointers" { wasm32-*-* } } */

extern int bar (void);

int
foo (void)
{
  return (&bar + 4096) ();
}
