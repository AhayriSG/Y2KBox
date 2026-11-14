pci_ss = ss.source_set()
pci_ss.add(when: 'CONFIG_PCI_BRIDGE', if_true: files('pci_bridge_dev.c'))
pci_ss.add(when: 'CONFIG_I82801B11', if_true: files())
pci_ss.add(when: 'CONFIG_IOH3420', if_true: files())
pci_ss.add(when: 'CONFIG_PCIE_PORT', if_true: files())
pci_ss.add(when: 'CONFIG_PCIE_PCI_BRIDGE', if_true: files())
pci_ss.add(when: 'CONFIG_PXB', if_true: files())
pci_ss.add(when: 'CONFIG_CXL', if_true: files())
pci_ss.add(when: 'CONFIG_XIO3130', if_true: files())
pci_ss.add(when: 'CONFIG_CXL', if_true: files())

# Sun4u
pci_ss.add(when: 'CONFIG_SIMBA', if_true: files())

system_ss.add_all(when: 'CONFIG_PCI', if_true: pci_ss)
