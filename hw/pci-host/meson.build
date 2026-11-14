pci_ss = ss.source_set()
pci_ss.add(when: 'CONFIG_PAM', if_true: files('pam.c'))
pci_ss.add(when: 'CONFIG_PCI_BONITO', if_true: files())
pci_ss.add(when: 'CONFIG_GT64120', if_true: files())
pci_ss.add(when: 'CONFIG_PCI_EXPRESS_ASPEED', if_true: files())
pci_ss.add(when: 'CONFIG_PCI_EXPRESS_DESIGNWARE', if_true: files())
pci_ss.add(when: 'CONFIG_PCI_EXPRESS_GENERIC_BRIDGE', if_true: files())
pci_ss.add(when: ['CONFIG_PCI_EXPRESS_GENERIC_BRIDGE', 'CONFIG_ACPI'], if_true: files())
pci_ss.add(when: 'CONFIG_PCI_EXPRESS_Q35', if_true: files())
pci_ss.add(when: 'CONFIG_PCI_EXPRESS_XILINX', if_true: files())
pci_ss.add(when: 'CONFIG_PCI_I440FX', if_true: files('i440fx.c'))
pci_ss.add(when: 'CONFIG_PCI_SABRE', if_true: files())
pci_ss.add(when: 'CONFIG_REMOTE_PCIHOST', if_true: files())
pci_ss.add(when: 'CONFIG_SH_PCI', if_true: files())

# PPC devices
pci_ss.add(when: 'CONFIG_PPC4XX_PCI', if_true: files())
pci_ss.add(when: 'CONFIG_PPC440_PCIX', if_true: files())
pci_ss.add(when: 'CONFIG_RAVEN_PCI', if_true: files())
pci_ss.add(when: 'CONFIG_GRACKLE_PCI', if_true: files())
# NewWorld PowerMac
pci_ss.add(when: 'CONFIG_UNIN_PCI', if_true: files())
# PowerPC E500 boards
pci_ss.add(when: 'CONFIG_PPCE500_PCI', if_true: files())
# AmigaOne
pci_ss.add(when: 'CONFIG_ARTICIA', if_true: files())
# Pegasos2
pci_ss.add(when: 'CONFIG_MV64361', if_true: files())

# ARM devices
pci_ss.add(when: 'CONFIG_PCI_EXPRESS_FSL_IMX8M_PHY', if_true: files())
pci_ss.add(when: 'CONFIG_VERSATILE_PCI', if_true: files())

# HPPA devices
specific_ss.add(when: 'CONFIG_ASTRO', if_true: files())
pci_ss.add(when: 'CONFIG_DINO', if_true: files())

system_ss.add_all(when: 'CONFIG_PCI', if_true: pci_ss)

specific_ss.add(when: 'CONFIG_PCI_POWERNV', if_true: files(
  ))
