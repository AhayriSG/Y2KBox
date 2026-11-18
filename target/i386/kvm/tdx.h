/*
 * XP fork: Intel TDX support was removed. Keep the disabled predicate for
 * shared x86 CPU feature gating.
 */
#ifndef QEMU_I386_TDX_H
#define QEMU_I386_TDX_H

#define is_tdx_vm() 0

#endif /* QEMU_I386_TDX_H */
