system_ss.add(when: 'CONFIG_DP8393X', if_true: files())
system_ss.add(when: 'CONFIG_NE2000_COMMON', if_true: files())

# PCI network cards
system_ss.add(when: 'CONFIG_NE2000_PCI', if_true: files())
system_ss.add(when: 'CONFIG_EEPRO100_PCI', if_true: files())
system_ss.add(when: 'CONFIG_PCNET_PCI', if_true: files('pcnet-pci.c'))
system_ss.add(when: 'CONFIG_PCNET_COMMON', if_true: files('pcnet.c'))
system_ss.add(when: 'CONFIG_E1000_PCI', if_true: files('e1000.c', 'e1000x_common.c'))
system_ss.add(when: 'CONFIG_E1000E_PCI_EXPRESS', if_true: files())
system_ss.add(when: 'CONFIG_E1000E_PCI_EXPRESS', if_true: files('e1000x_common.c'))
system_ss.add(when: 'CONFIG_IGB_PCI_EXPRESS', if_true: files())
system_ss.add(when: 'CONFIG_IGB_PCI_EXPRESS', if_true: files())
system_ss.add(when: 'CONFIG_RTL8139_PCI', if_true: files('rtl8139.c'))
system_ss.add(when: 'CONFIG_TULIP', if_true: files())
system_ss.add(when: 'CONFIG_VMXNET3_PCI', if_true: files())
system_ss.add(when: 'CONFIG_VMXNET3_PCI', if_true: files())

system_ss.add(when: 'CONFIG_SMC91C111', if_true: files())
system_ss.add(when: 'CONFIG_LAN9118', if_true: files())
system_ss.add(when: 'CONFIG_LAN9118_PHY', if_true: files())
system_ss.add(when: 'CONFIG_NE2000_ISA', if_true: files())
system_ss.add(when: 'CONFIG_OPENCORES_ETH', if_true: files())
system_ss.add(when: 'CONFIG_XILINX_AXI', if_true: files())
system_ss.add(when: 'CONFIG_ALLWINNER_EMAC', if_true: files())
system_ss.add(when: 'CONFIG_ALLWINNER_SUN8I_EMAC', if_true: files())
system_ss.add(when: 'CONFIG_IMX_FEC', if_true: files())
system_ss.add(when: 'CONFIG_MSF2', if_true: files())

system_ss.add(when: 'CONFIG_CADENCE', if_true: files())
system_ss.add(when: 'CONFIG_STELLARIS_ENET', if_true: files())
system_ss.add(when: 'CONFIG_LANCE', if_true: files())
system_ss.add(when: 'CONFIG_LASI_82596', if_true: files())
system_ss.add(when: 'CONFIG_I82596_COMMON', if_true: files())
system_ss.add(when: 'CONFIG_SUNHME', if_true: files())
system_ss.add(when: 'CONFIG_FTGMAC100', if_true: files())
system_ss.add(when: 'CONFIG_SUNGEM', if_true: files())
system_ss.add(when: 'CONFIG_NPCM7XX', if_true: files())
system_ss.add(when: 'CONFIG_NPCM8XX', if_true: files())

system_ss.add(when: 'CONFIG_COLDFIRE', if_true: files())
specific_ss.add(when: 'CONFIG_PSERIES', if_true: files())
system_ss.add(when: 'CONFIG_XILINX_ETHLITE', if_true: files())

system_ss.add(when: 'CONFIG_VIRTIO_NET', if_true: files())
system_ss.add(when: 'CONFIG_VIRTIO_NET', if_true: files())

if have_vhost_net
  system_ss.add(when: 'CONFIG_VIRTIO_NET', if_true: files())
endif
stub_ss.add(files('vhost_net-stub.c'))

system_ss.add(when: 'CONFIG_ETSEC', if_true: files(
  ))

system_ss.add(when: 'CONFIG_ROCKER', if_true: files(
  ))
stub_ss.add(files('rocker/rocker-stubs.c'))
if have_hmp
  system_ss.add(files('rocker/rocker-hmp-cmds.c'))
endif

system_ss.add(when: 'CONFIG_RASPI', if_true: files())

subdir('can')
