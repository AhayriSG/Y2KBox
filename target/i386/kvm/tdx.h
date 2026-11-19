/*
 * Y2KBox KVM TDX stubs. Confidential-computing guests are not supported.
 */
#ifndef QEMU_I386_TDX_H
#define QEMU_I386_TDX_H

#include "cpu.h"
#include "qapi/error.h"
struct kvm_run;

#define is_tdx_vm() 0

#define TDVMCALL_GET_TD_VM_CALL_INFO 0x10000
#define TDVMCALL_GET_QUOTE 0x10002
#define TDVMCALL_SETUP_EVENT_NOTIFY_INTERRUPT 0x10004

int tdx_pre_create_vcpu(CPUState *cpu, Error **errp);
int tdx_parse_tdvf(void *flash_ptr, int size);
int tdx_handle_report_fatal_error(X86CPU *cpu, struct kvm_run *run);
void tdx_handle_get_quote(X86CPU *cpu, struct kvm_run *run);
void tdx_handle_get_tdvmcall_info(X86CPU *cpu, struct kvm_run *run);
void tdx_handle_setup_event_notify_interrupt(X86CPU *cpu, struct kvm_run *run);

#endif /* QEMU_I386_TDX_H */
