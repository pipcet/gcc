__attribute__((stackcall)) int funcall(int, int, int, int, int, int);

int funcall(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5)
{
  int ret;
  asm volatile("%S0\n\t%1\n\t%2\n\t%3\n\t%4\n\t%5\n\t%6\n\tcall_import[6] $extcall\n\t%R0"
	       : "=r" (ret): "r" (arg0), "r" (arg1), "r" (arg2),
	       "r" (arg3), "r" (arg4), "r" (arg5));

  return ret;
}
