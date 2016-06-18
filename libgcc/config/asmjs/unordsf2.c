int __unordsf2(float x, float y)
{
  int xnan, ynan;
  double xd = x;
  double yd = y;

#ifdef __ASMJS__
  asm("%O0 =(+%O1 != +%O1)|0;" : "=r" (xnan) : "rmi" (x));
  asm("%O0 =(+%O1 != +%O1)|0;" : "=r" (ynan) : "rmi" (y));
#else
#ifdef __WASM64__
  asm("(%S0 (i64.extend_u_i32 (f64.ne %O1 %O1)))" : "=r" (xnan) : "rmi" (xd));
  asm("(%S0 (i64.extend_u_i32 (f64.ne %O1 %O1)))" : "=r" (ynan) : "rmi" (yd));
#else
  asm("(%S0 (f64.ne %O1 %O1))" : "=r" (xnan) : "rmi" (xd));
  asm("(%S0 (f64.ne %O1 %O1))" : "=r" (ynan) : "rmi" (yd));
#endif
#endif

  return xnan | ynan;
}
