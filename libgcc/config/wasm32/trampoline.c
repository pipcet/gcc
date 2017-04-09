asm(".include \"wasm32-import-macros.s\"");

asm(".import3_pic thinthin,init_trampoline,__thinthin_init_trampoline");
asm(".import3_pic thinthin,destroy_trampoline,__thinthin_destroy_trampoline");

extern long __thinthin_init_trampoline(void *m_tramp)
  __attribute__((stackcall));

extern long __thinthin_destroy_trampoline(void *m_tramp)
  __attribute__((stackcall));

struct gcc_trampoline {
  unsigned magic;
  unsigned magic1;
  unsigned fnaddr;
  unsigned fnaddr1;
  unsigned static_chain;
  unsigned static_chain1;
  unsigned trampoline_index;
  unsigned trampoline_index1;
};

void __wasm_init_trampoline (struct gcc_trampoline *m_tramp)
{
  m_tramp->magic = "FiiiiiiiE";

  __thinthin_init_trampoline (m_tramp);
}

void __wasm_destroy_trampoline (struct gcc_trampoline *m_tramp)
{
  __thinthin_destroy_trampoline (m_tramp);
}
