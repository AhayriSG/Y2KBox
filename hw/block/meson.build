system_ss.add(files(
  'block.c',
  'cdrom.c',
  'hd-geometry.c'
))
system_ss.add(when: 'CONFIG_FDC', if_true: files('fdc.c'))
system_ss.add(when: 'CONFIG_FDC_ISA', if_true: files('fdc-isa.c'))
system_ss.add(when: 'CONFIG_FDC_SYSBUS', if_true: files())
system_ss.add(when: 'CONFIG_PFLASH_CFI01', if_true: files('pflash_cfi01.c'))
system_ss.add(when: 'CONFIG_PFLASH_CFI02', if_true: files())
system_ss.add(when: 'CONFIG_SSI_M25P80', if_true: files())
system_ss.add(when: 'CONFIG_SSI_M25P80', if_true: files())
system_ss.add(when: 'CONFIG_SWIM', if_true: files())

system_ss.add(when: 'CONFIG_VIRTIO_BLK', if_true: files())
system_ss.add(when: 'CONFIG_VIRTIO_BLK', if_true: files())
system_ss.add(when: 'CONFIG_VHOST_USER_BLK', if_true: files())
system_ss.add(when: 'CONFIG_VHOST_USER_BLK', if_true: files())

subdir('dataplane')
