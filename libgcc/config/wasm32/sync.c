extern unsigned __atomic_fetch_add_4(unsigned *mem,
				   unsigned addend,
				   int mem_type)
{
  unsigned ret = *mem;
  *mem += addend;
  return ret;
}
