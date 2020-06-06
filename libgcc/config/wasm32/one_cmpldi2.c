long long __one_cmpldi2(long long x)
{
  unsigned long y = x >> 32LL;
  unsigned long z = x & 0xffffffff;

  asm volatile("%S0\n\t%1\n\ti32.const -1\n\ti32.xor\n\t%R0"
	       : "=r" (y) : "r" (y));
  asm volatile("%S0\n\t%1\n\ti32.const -1\n\ti32.xor\n\t%R0"
	       : "=r" (z) : "r" (z));
  return ((long long)y << 32LL) | z;
}

