long __fixsfsi(float f)
{
  return (long)(double)f;
}

long __fixdfsi(double f)
{
  long ret;
  asm("%S0\n\t"
      "%1\n\t"
      "i32.trunc_sat_f64_s\n\t"
      "%R0"
      : "=r" (ret) : "r" (f));
  return ret;
}

#if 0
float __floatdisf(long x)
{
  return (float)(double)x;
}
#endif
