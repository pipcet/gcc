long __fixsfsi(float f)
{
  return (long)(double)f;
}

long __fixdfsi(double f)
{
  return (long)(double)f;
}

#if 0
float __floatdisf(long x)
{
  return (float)(double)x;
}
#endif
