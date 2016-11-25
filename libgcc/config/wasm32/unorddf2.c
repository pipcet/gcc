int __unorddf2(double x, double y)
{
  int xnan, ynan;

  asm("%S0\n\t%O1\n\t%O1\n\tf64.ne\n\t%R0" : "=r" (xnan) : "rmi" (x));
  asm("%S0\n\t%O1\n\t%O1\n\tf64.ne\n\t%R0" : "=r" (ynan) : "rmi" (y));

  return xnan | ynan;
}
