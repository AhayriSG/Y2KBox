# Default configuration for i386-softmmu -- XP fork.
#
# Target: Windows XP (32-bit) guests on the i440fx "pc" machine.
# Everything below is switched off because XP cannot use it, or it only
# exists for other guests (modern PCIe machines, confidential computing,
# paravirtual Xen/KVM-only bits, UEFI).

CONFIG_Q35=n
CONFIG_MICROVM=n
CONFIG_NITRO_ENCLAVE=n
CONFIG_ISAPC=n
CONFIG_XEN_EMU=n
CONFIG_SEV=n
CONFIG_SGX=n
CONFIG_TDX=n
CONFIG_VTD=n
CONFIG_VTD_ACCEL=n
CONFIG_AMD_IOMMU=n
CONFIG_X86_FW_OVMF=n
