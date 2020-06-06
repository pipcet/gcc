int __unordsf2(float x, float y)
{
  int xnan, ynan;
  double xd = x;
  double yd = y;

  asm("%S0\n\t%O1\n\t%O1\n\tf64.ne\n\t%R0" : "=r" (xnan) : "rmi" (xd));
  asm("%S0\n\t%O1\n\t%O1\n\tf64.ne\n\t%R0" : "=r" (ynan) : "rmi" (yd));

  return xnan | ynan;
}
