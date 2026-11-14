system_ss.add(files('intc.c'))
system_ss.add(when: 'CONFIG_ARM_GIC', if_true: files(
  ))
system_ss.add(when: 'CONFIG_ARM_GICV3', if_true: files(
  ))
system_ss.add(when: 'CONFIG_ARM_GICV5', if_true: files(
  ))
system_ss.add(when: 'CONFIG_ALLWINNER_A10_PIC', if_true: files())
system_ss.add(when: 'CONFIG_ASPEED_SOC', if_true: files())
system_ss.add(when: 'CONFIG_ASPEED_SOC', if_true: files())
system_ss.add(when: 'CONFIG_EXYNOS4', if_true: files())
system_ss.add(when: 'CONFIG_GOLDFISH_PIC', if_true: files())
system_ss.add(when: 'CONFIG_HEATHROW_PIC', if_true: files())
system_ss.add(when: 'CONFIG_I8259', if_true: files('i8259_common.c', 'i8259.c'))
system_ss.add(when: 'CONFIG_IMX_AVIC', if_true: files())
system_ss.add(when: 'CONFIG_IMX_GPCV2', if_true: files())
system_ss.add(when: 'CONFIG_IOAPIC', if_true: files('ioapic_common.c'), if_false: files())
system_ss.add(when: 'CONFIG_OMAP', if_true: files())
system_ss.add(when: 'CONFIG_OPENPIC', if_true: files())
system_ss.add(when: 'CONFIG_PL190', if_true: files())
system_ss.add(when: 'CONFIG_RASPI', if_true: files())
system_ss.add(when: 'CONFIG_REALVIEW', if_true: files())
system_ss.add(when: 'CONFIG_SLAVIO', if_true: files())
system_ss.add(when: 'CONFIG_XILINX', if_true: files())
system_ss.add(when: 'CONFIG_XLNX_ZYNQMP', if_true: files())
system_ss.add(when: 'CONFIG_XLNX_ZYNQMP_PMU', if_true: files())

if config_all_devices.has_key('CONFIG_APIC') or \
   config_all_devices.has_key('CONFIG_I8259') or \
   config_all_devices.has_key('CONFIG_MC146818RTC')
    system_ss.add(files('kvm_irqcount.c'))
endif

specific_ss.add(when: 'CONFIG_APIC', if_true: files('apic.c', 'apic_common.c'))
arm_common_ss.add(when: 'CONFIG_ARM_GIC', if_true: files())
arm_common_ss.add(when: 'CONFIG_ARM_GICV3', if_true: files())
specific_ss.add(when: 'CONFIG_ARM_GIC_KVM', if_true: files())
specific_ss.add(when: ['CONFIG_WHPX', 'CONFIG_ARM_GICV3'], if_true: files())
specific_ss.add(when: ['CONFIG_HVF', 'CONFIG_ARM_GICV3'], if_true: files())
stub_ss.add(files('arm_gicv3_hvf_stub.c'))
specific_ss.add(when: 'CONFIG_ARM_GIC_KVM', if_true: files())
arm_common_ss.add(when: 'CONFIG_ARM_V7M', if_true: files())
specific_ss.add(when: 'CONFIG_GRLIB', if_true: files())
specific_ss.add(when: 'CONFIG_IOAPIC', if_true: files('ioapic.c'))
specific_ss.add(when: 'CONFIG_LOONGSON_LIOINTC', if_true: files())
specific_ss.add(when: 'CONFIG_MIPS_CPS', if_true: files())
specific_ss.add(when: 'CONFIG_OMPIC', if_true: files())
specific_ss.add(when: ['CONFIG_KVM', 'CONFIG_OPENPIC'],
		if_true: files())
specific_ss.add(when: 'CONFIG_POWERNV', if_true: files())
specific_ss.add(when: 'CONFIG_PPC_UIC', if_true: files())
specific_ss.add(when: 'CONFIG_RX_ICU', if_true: files())
specific_ss.add(when: 'CONFIG_S390_FLIC', if_true: files())
specific_ss.add(when: 'CONFIG_S390_FLIC_KVM', if_true: files())
specific_ss.add(when: 'CONFIG_SH_INTC', if_true: files())
specific_ss.add(when: 'CONFIG_RISCV_ACLINT', if_true: files())
specific_ss.add(when: 'CONFIG_RISCV_APLIC', if_true: files())
specific_ss.add(when: 'CONFIG_RISCV_IMSIC', if_true: files())
specific_ss.add(when: 'CONFIG_SIFIVE_PLIC', if_true: files())
specific_ss.add(when: 'CONFIG_XICS', if_true: files())
specific_ss.add(when: ['CONFIG_KVM', 'CONFIG_XICS'],
		if_true: files())
specific_ss.add(when: 'CONFIG_PSERIES', if_true: files())
specific_ss.add(when: 'CONFIG_XIVE', if_true: files())
specific_ss.add(when: ['CONFIG_KVM', 'CONFIG_XIVE'],
		if_true: files())

specific_ss.add(when: 'CONFIG_HEX_L2VIC', if_true: files())
specific_ss.add(when: 'CONFIG_M68K_IRQC', if_true: files())
specific_ss.add(when: 'CONFIG_LOONGSON_IPI_COMMON', if_true: files())
specific_ss.add(when: 'CONFIG_LOONGSON_IPI', if_true: files())
specific_ss.add(when: 'CONFIG_LOONGARCH_IPI', if_true: files())
specific_ss.add(when: ['CONFIG_KVM', 'CONFIG_LOONGARCH_IPI'],
                if_true: files())
specific_ss.add(when: 'CONFIG_LOONGARCH_PCH_PIC', if_true: files())
specific_ss.add(when: ['CONFIG_KVM', 'CONFIG_LOONGARCH_PCH_PIC'],
                if_true: files())
specific_ss.add(when: 'CONFIG_LOONGARCH_PCH_MSI', if_true: files())
specific_ss.add(when: 'CONFIG_LOONGARCH_EXTIOI', if_true: files())
specific_ss.add(when: ['CONFIG_KVM', 'CONFIG_LOONGARCH_EXTIOI'],
               if_true: files())
specific_ss.add(when: 'CONFIG_LOONGARCH_DINTC', if_true: files())
specific_ss.add(when: ['CONFIG_KVM', 'CONFIG_LOONGARCH_DINTC'],
               if_true: files())
