int __unordsf2(float x, float y)
{
  int xnan, ynan;

  asm("%O0 =(+%O1 != +%O1)|0;" : "=r" (xnan) : "rmi" (x));
  asm("%O0 =(+%O1 != +%O1)|0;" : "=r" (ynan) : "rmi" (y));

  return xnan | ynan;
}
