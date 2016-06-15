__attribute__((stackcall)) int funcall(int, int, int, int, int, int);

int funcall(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5)
{
  int ret;
  asm volatile("(call_import $extcall %O0 %O1 %O2 %O3 %O4 %O5)"
	       : "=r" (ret): "r" (arg0), "r" (arg1), "r" (arg2),
	       "r" (arg3), "r" (arg4), "r" (arg5));

  return ret;
}
