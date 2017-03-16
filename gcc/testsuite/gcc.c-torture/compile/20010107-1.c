/* xfail this, it doesn't work with separate code and data spaces. */

unsigned long x[4];

void foo(void)
{
  ((void (*)())(x+2))();
}
