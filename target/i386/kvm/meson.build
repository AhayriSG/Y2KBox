# Linux KVM support for the x86 system emulator. Confidential guests and Xen
# emulation remain disabled in the XP fork.
i386_kvm_ss = ss.source_set()
i386_kvm_ss.add(files(
  'kvm.c',
  'kvm-cpu.c',
  'tdx-stub.c',
))
i386_system_ss.add(files('hyperv-stub.c'))
i386_system_ss.add_all(when: 'CONFIG_KVM', if_true: i386_kvm_ss)
