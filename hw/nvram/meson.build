system_ss.add(files('fw_cfg-interface.c'))
system_ss.add(files('fw_cfg.c'))
system_ss.add(when: 'CONFIG_RASPI', if_true: files())
system_ss.add(when: 'CONFIG_CHRP_NVRAM', if_true: files())
system_ss.add(when: 'CONFIG_DS1225Y', if_true: files())
system_ss.add(when: 'CONFIG_NMC93XX_EEPROM', if_true: files())
system_ss.add(when: 'CONFIG_AT24C', if_true: files())
system_ss.add(when: 'CONFIG_MAC_NVRAM', if_true: files())
system_ss.add(when: 'CONFIG_NPCM7XX', if_true: files())
system_ss.add(when: 'CONFIG_NRF51_SOC', if_true: files())
system_ss.add(when: 'CONFIG_XLNX_EFUSE_CRC', if_true: files())
system_ss.add(when: 'CONFIG_XLNX_EFUSE', if_true: files())
system_ss.add(when: 'CONFIG_XLNX_EFUSE_VERSAL', if_true: files(
                                                   ))
system_ss.add(when: 'CONFIG_XLNX_EFUSE_ZYNQMP', if_true: files(
                                                   ))
system_ss.add(when: 'CONFIG_XLNX_BBRAM', if_true: files())

specific_ss.add(when: 'CONFIG_PSERIES', if_true: files())
system_ss.add(when: 'CONFIG_ACPI', if_true: files('fw_cfg-acpi.c'))

system_ss.add(when: 'CONFIG_ASPEED_SOC', if_true: files(
  ))
