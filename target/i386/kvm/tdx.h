/*
 * XP fork: Intel TDX support was removed. This header only keeps the
 * symbols that shared x86 code still names; they are all dead code.
 */
#ifndef QEMU_I386_TDX_H
#define QEMU_I386_TDX_H

#define is_tdx_vm() 0

int tdx_parse_tdvf(void *flash_ptr, int size);
void tdx_set_tdvf_region(MemoryRegion *tdvf_mr);

#endif /* QEMU_I386_TDX_H */
