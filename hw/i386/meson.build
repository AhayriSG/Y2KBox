i386_ss = ss.source_set()
i386_ss.add(files(
  'fw_cfg.c',
  'e820_memory_layout.c',
  'monitor.c',
  'multiboot.c',
  'x86.c',
  'x86-cpu.c',
))

i386_ss.add(when: 'CONFIG_APIC', if_true: files('vapic.c'))
i386_ss.add(when: 'CONFIG_X86_IOMMU', if_true: files(),
                                      if_false: files('x86-iommu-stub.c'))
i386_ss.add(when: 'CONFIG_I440FX', if_true: files('pc_piix.c'))
i386_ss.add(when: 'CONFIG_ISAPC', if_true: files())
i386_ss.add(when: 'CONFIG_MICROVM', if_true: files('x86-common.c'))
i386_ss.add(when: 'CONFIG_NITRO_ENCLAVE', if_true: files())
i386_ss.add(when: 'CONFIG_Q35', if_true: files())
i386_ss.add(when: 'CONFIG_VMMOUSE', if_true: files('vmmouse.c'))
i386_ss.add(when: 'CONFIG_VMPORT', if_true: files('vmport.c'))
i386_ss.add(when: 'CONFIG_VTD', if_true: files())
i386_ss.add(when: 'CONFIG_VTD_ACCEL', if_true: files())
i386_ss.add(when: 'CONFIG_SGX', if_true: files(),
                                if_false: files('sgx-stub.c'))
if have_hmp
  stub_ss.add(files('sgx-hmp-stub.c'))
endif

i386_ss.add(when: 'CONFIG_ACPI', if_true: files('acpi-common.c'))
i386_ss.add(when: 'CONFIG_PC', if_true: files(
  'x86-common.c',
  'pc.c',
  'pc_sysfw.c',
  'acpi-build.c',
  'port92.c'))
i386_ss.add(when: 'CONFIG_X86_FW_OVMF', if_true: files(),
                                        if_false: files('pc_sysfw_ovmf-stubs.c'))
i386_ss.add(when: 'CONFIG_TDX', if_true: files())



subdir('kvm')

hw_arch += {'i386': i386_ss}
