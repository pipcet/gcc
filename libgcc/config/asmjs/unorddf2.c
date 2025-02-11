int __unorddf2(double x, double y)
{
  int xnan, ynan;

#ifdef __ASMJS__
  asm("%O0 =(+%O1 != +%O1)|0;" : "=r" (xnan) : "rmi" (x));
  asm("%O0 =(+%O1 != +%O1)|0;" : "=r" (ynan) : "rmi" (y));
#else
#ifdef __WASM64__
  asm("%S0\n\t%O1\n\t%O1\n\tf64.ne\n\ti64.extend_u_i32\n\t%R0" : "=r" (xnan) : "rmi" (x));
  asm("%S0\n\t%O1\n\t%O1\n\tf64.ne\n\ti64.extend_u_i32\n\t%R0" : "=r" (ynan) : "rmi" (y));
#else
  asm("(%S0 (f64.ne %O1 %O1))" : "=r" (xnan) : "rmi" (x));
  asm("(%S0 (f64.ne %O1 %O1))" : "=r" (ynan) : "rmi" (y));
#endif
#endif

  return xnan | ynan;
}
