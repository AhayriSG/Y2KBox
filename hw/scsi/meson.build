scsi_ss = ss.source_set()
specific_scsi_ss = ss.source_set()
virtio_scsi_ss = ss.source_set()

scsi_ss.add(files(
  'emulation.c',
  'scsi-bus.c',
  'scsi-disk.c',
  'scsi-generic.c',
))
scsi_ss.add(when: 'CONFIG_ESP', if_true: files())
scsi_ss.add(when: 'CONFIG_ESP_PCI', if_true: files())
scsi_ss.add(when: 'CONFIG_LSI_SCSI_PCI', if_true: files())
scsi_ss.add(when: 'CONFIG_NCR710_SCSI', if_true: files())
scsi_ss.add(when: 'CONFIG_MEGASAS_SCSI_PCI', if_true: files())
scsi_ss.add(when: 'CONFIG_MPTSAS_SCSI_PCI', if_true: files())
scsi_ss.add(when: 'CONFIG_VMW_PVSCSI_SCSI_PCI', if_true: files())

virtio_scsi_ss.add(files())
virtio_scsi_ss.add(when: 'CONFIG_VHOST_SCSI', if_true: files())
virtio_scsi_ss.add(when: 'CONFIG_VHOST_USER_SCSI', if_true: files())

virtio_scsi_ss.add(files())
virtio_scsi_ss.add(when: 'CONFIG_VHOST_SCSI_COMMON', if_true: files())

scsi_ss.add_all(when: 'CONFIG_VIRTIO_SCSI', if_true: virtio_scsi_ss)

specific_scsi_ss.add(when: 'CONFIG_SPAPR_VSCSI', if_true: files())

system_ss.add_all(when: 'CONFIG_SCSI', if_true: scsi_ss)
specific_ss.add_all(when: 'CONFIG_SCSI', if_true: specific_scsi_ss)
