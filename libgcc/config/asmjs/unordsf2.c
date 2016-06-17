int __unordsf2(float x, float y)
{
  int xnan, ynan;
  double xd = x;
  double yd = y;

#ifdef __ASMJS__
  asm("%O0 =(+%O1 != +%O1)|0;" : "=r" (xnan) : "rmi" (x));
  asm("%O0 =(+%O1 != +%O1)|0;" : "=r" (ynan) : "rmi" (y));
#else
  asm("(%S0 (f64.ne %O1 %O1))" : "=r" (xnan) : "rmi" (xd));
  asm("(%S0 (f64.ne %O1 %O1))" : "=r" (ynan) : "rmi" (yd));
#endif

  return xnan | ynan;
}
