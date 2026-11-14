system_ss.add(when: 'CONFIG_APPLESMC', if_true: files())

system_ss.add(when: 'CONFIG_EDU', if_true: files())
system_ss.add(when: 'CONFIG_FW_CFG_DMA', if_true: files('vmcoreinfo.c'))
system_ss.add(when: 'CONFIG_ISA_DEBUG', if_true: files('debugexit.c'))
system_ss.add(when: 'CONFIG_ISA_TESTDEV', if_true: files())
system_ss.add(when: 'CONFIG_PCI_TESTDEV', if_true: files())
system_ss.add(when: 'CONFIG_IOMMU_TESTDEV', if_true: files())
system_ss.add(when: 'CONFIG_UNIMP', if_true: files())
system_ss.add(when: 'CONFIG_EMPTY_SLOT', if_true: files())
system_ss.add(when: 'CONFIG_LED', if_true: files())
system_ss.add(when: 'CONFIG_PVPANIC_COMMON', if_true: files())

# ARM devices
system_ss.add(when: 'CONFIG_PL310', if_true: files())
system_ss.add(when: 'CONFIG_INTEGRATOR_DEBUG', if_true: files())
system_ss.add(when: 'CONFIG_A9SCU', if_true: files())
system_ss.add(when: 'CONFIG_ARM11SCU', if_true: files())

system_ss.add(when: 'CONFIG_ARM_V7M', if_true: files())

# Mac devices
system_ss.add(when: 'CONFIG_MOS6522', if_true: files())
stub_ss.add(files('mos6522-stub.c'))
system_ss.add(when: 'CONFIG_DJMEMC', if_true: files())
system_ss.add(when: 'CONFIG_IOSB', if_true: files())

# virt devices
system_ss.add(when: 'CONFIG_VIRT_CTRL', if_true: files())

# RISC-V devices
system_ss.add(when: 'CONFIG_K230', if_true: files())
system_ss.add(when: 'CONFIG_MCHP_PFSOC_DMC', if_true: files())
system_ss.add(when: 'CONFIG_MCHP_PFSOC_IOSCB', if_true: files())
system_ss.add(when: 'CONFIG_MCHP_PFSOC_SYSREG', if_true: files())
system_ss.add(when: 'CONFIG_SIFIVE_TEST', if_true: files())
system_ss.add(when: 'CONFIG_SIFIVE_E_PRCI', if_true: files())
system_ss.add(when: 'CONFIG_SIFIVE_E_AON', if_true: files())
system_ss.add(when: 'CONFIG_SIFIVE_U_OTP', if_true: files())
system_ss.add(when: 'CONFIG_SIFIVE_U_PRCI', if_true: files())
system_ss.add(when: 'CONFIG_K230_DECOMP_GZIP', if_true: files())
system_ss.add(when: 'CONFIG_SOPHGO_CV1800B_CLK', if_true: files())
system_ss.add(when: 'CONFIG_TENSTORRENT', if_true: files())

subdir('macio')

# ivshmem devices
system_ss.add(when: 'CONFIG_IVSHMEM_DEVICE', if_true: files())
system_ss.add(when: 'CONFIG_IVSHMEM_FLAT_DEVICE', if_true: files())

system_ss.add(when: 'CONFIG_ALLWINNER_SRAMC', if_true: files())
system_ss.add(when: 'CONFIG_ALLWINNER_A10_CCM', if_true: files())
system_ss.add(when: 'CONFIG_ALLWINNER_A10_DRAMC', if_true: files())
system_ss.add(when: 'CONFIG_ALLWINNER_H3', if_true: files())
system_ss.add(when: 'CONFIG_ALLWINNER_H3', if_true: files())
system_ss.add(when: 'CONFIG_ALLWINNER_H3', if_true: files())
system_ss.add(when: 'CONFIG_ALLWINNER_H3', if_true: files())
system_ss.add(when: 'CONFIG_ALLWINNER_H3', if_true: files())
system_ss.add(when: 'CONFIG_ALLWINNER_R40', if_true: files())
system_ss.add(when: 'CONFIG_ALLWINNER_R40', if_true: files())
system_ss.add(when: 'CONFIG_AXP2XX_PMU', if_true: files())
system_ss.add(when: 'CONFIG_REALVIEW', if_true: files())
system_ss.add(when: 'CONFIG_ECCMEMCTL', if_true: files())
system_ss.add(when: 'CONFIG_EXYNOS4', if_true: files())
system_ss.add(when: 'CONFIG_FSL_IMX25_CCM', if_true: files())
system_ss.add(when: 'CONFIG_FSL_IMX31_CCM', if_true: files())
system_ss.add(when: 'CONFIG_FSL_IMX6_CCM', if_true: files())
system_ss.add(when: 'CONFIG_FSL_IMX6_SRC', if_true: files())
system_ss.add(when: 'CONFIG_FSL_IMX6UL_CCM', if_true: files())
system_ss.add(when: 'CONFIG_FSL_IMX7_CCM', if_true: files())
system_ss.add(when: 'CONFIG_FSL_IMX7_GPR', if_true: files())
system_ss.add(when: 'CONFIG_FSL_IMX7_SNVS', if_true: files())
system_ss.add(when: 'CONFIG_FSL_IMX7_SRC', if_true: files())
system_ss.add(when: 'CONFIG_FSL_IMX8MP_ANALOG', if_true: files())
system_ss.add(when: 'CONFIG_FSL_IMX8MP_CCM', if_true: files())
system_ss.add(when: 'CONFIG_IMX_CCM', if_true: files())
system_ss.add(when: 'CONFIG_IMX_RNG', if_true: files())
system_ss.add(when: 'CONFIG_MAX78000_AES', if_true: files())
system_ss.add(when: 'CONFIG_MAX78000_GCR', if_true: files())
system_ss.add(when: 'CONFIG_MAX78000_ICC', if_true: files())
system_ss.add(when: 'CONFIG_MAX78000_TRNG', if_true: files())
system_ss.add(when: 'CONFIG_NPCM7XX', if_true: files(
  ))
system_ss.add(when: 'CONFIG_OMAP', if_true: files(
  ))
system_ss.add(when: 'CONFIG_RASPI', if_true: files(
  ))
system_ss.add(when: 'CONFIG_SLAVIO', if_true: files())
system_ss.add(when: 'CONFIG_ZYNQ', if_true: files())
system_ss.add(when: 'CONFIG_XLNX_ZYNQ_DDRC', if_true: files())
system_ss.add(when: 'CONFIG_XLNX_ZYNQMP_ARM', if_true: files())
system_ss.add(when: 'CONFIG_XLNX_ZYNQMP_ARM', if_true: files())
system_ss.add(when: 'CONFIG_XLNX_VERSAL', if_true: files(
  ))
system_ss.add(when: 'CONFIG_XLNX_VERSAL_TRNG', if_true: files(
  ))
system_ss.add(when: 'CONFIG_STM32_RCC', if_true: files())
system_ss.add(when: 'CONFIG_STM32F2XX_SYSCFG', if_true: files())
system_ss.add(when: 'CONFIG_STM32F4XX_SYSCFG', if_true: files())
system_ss.add(when: 'CONFIG_STM32F4XX_EXTI', if_true: files())
system_ss.add(when: 'CONFIG_STM32L4X5_EXTI', if_true: files())
system_ss.add(when: 'CONFIG_STM32L4X5_SYSCFG', if_true: files())
system_ss.add(when: 'CONFIG_STM32L4X5_RCC', if_true: files())
system_ss.add(when: 'CONFIG_MPS2_FPGAIO', if_true: files())
system_ss.add(when: 'CONFIG_MPS2_SCC', if_true: files())

system_ss.add(when: 'CONFIG_TZ_MPC', if_true: files())
system_ss.add(when: 'CONFIG_TZ_MSC', if_true: files())
system_ss.add(when: 'CONFIG_TZ_PPC', if_true: files())
system_ss.add(when: 'CONFIG_IOTKIT_SECCTL', if_true: files())
system_ss.add(when: 'CONFIG_IOTKIT_SYSCTL', if_true: files())
system_ss.add(when: 'CONFIG_IOTKIT_SYSINFO', if_true: files())
system_ss.add(when: 'CONFIG_ARMSSE_CPU_PWRCTRL', if_true: files())
system_ss.add(when: 'CONFIG_ARMSSE_CPUID', if_true: files())
system_ss.add(when: 'CONFIG_ARMSSE_MHU', if_true: files())

system_ss.add(when: 'CONFIG_PVPANIC_ISA', if_true: files())
system_ss.add(when: 'CONFIG_PVPANIC_PCI', if_true: files())
system_ss.add(when: 'CONFIG_PVPANIC_MMIO', if_true: files())
system_ss.add(when: 'CONFIG_AUX', if_true: files())
system_ss.add(when: 'CONFIG_ASPEED_SOC', if_true: files(
  ))

system_ss.add(when: 'CONFIG_MSF2', if_true: files())
system_ss.add(when: 'CONFIG_NRF51_SOC', if_true: files())

system_ss.add(when: 'CONFIG_GRLIB', if_true: files())

system_ss.add(when: 'CONFIG_I2C_ECHO', if_true: files())

system_ss.add(when: 'CONFIG_AVR_POWER', if_true: files())

system_ss.add(when: 'CONFIG_MAC_VIA', if_true: files())

specific_ss.add(when: 'CONFIG_MIPS_CPS', if_true: files())
specific_ss.add(when: 'CONFIG_MIPS_ITU', if_true: files())

specific_ss.add(when: 'CONFIG_RISCV_MIPS_CMGCR', if_true: files())
specific_ss.add(when: 'CONFIG_RISCV_MIPS_CPC', if_true: files())
if igvm.found()
   system_ss.add(when: 'CONFIG_FW_CFG_DMA', if_true: files())
endif

system_ss.add(when: 'CONFIG_SBSA_REF', if_true: files())

# HPPA devices
system_ss.add(when: 'CONFIG_LASI', if_true: files())

system_ss.add(when: 'CONFIG_AXIADO_CLK', if_true: files())

# Qualcomm Hexagon DSP subsystems
system_ss.add(when: 'CONFIG_QCOM_QDSP6SS_CLK', if_true: files())
