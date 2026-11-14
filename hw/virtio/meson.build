system_virtio_ss = ss.source_set()
system_virtio_ss.add(files())
system_virtio_ss.add(files())
system_virtio_ss.add(files())
system_virtio_ss.add(when: 'CONFIG_VIRTIO_PCI', if_true: files())
system_virtio_ss.add(when: 'CONFIG_VIRTIO_MMIO', if_true: files())
system_virtio_ss.add(when: 'CONFIG_VIRTIO_CRYPTO', if_true: files())
system_virtio_ss.add(when: 'CONFIG_VHOST_VSOCK_COMMON', if_true: files())
system_virtio_ss.add(when: 'CONFIG_VIRTIO_IOMMU', if_true: files())
system_virtio_ss.add(when: 'CONFIG_VHOST_VDPA_DEV', if_true: files())

system_virtio_ss.add(files())
system_virtio_ss.add(files())

if have_vhost
  system_virtio_ss.add(files())
  system_virtio_ss.add(files())
  if have_vhost_user
    system_virtio_ss.add(files())
    system_virtio_ss.add(files())

    # MMIO Stubs
    system_virtio_ss.add(when: 'CONFIG_VHOST_USER_TEST', if_true: files())
    system_virtio_ss.add(when: 'CONFIG_VHOST_USER_GPIO', if_true: files())
    system_virtio_ss.add(when: 'CONFIG_VHOST_USER_I2C', if_true: files())
    system_virtio_ss.add(when: 'CONFIG_VHOST_USER_RNG', if_true: files())
    system_virtio_ss.add(when: 'CONFIG_VHOST_USER_SND', if_true: files())
    system_virtio_ss.add(when: 'CONFIG_VHOST_USER_INPUT', if_true: files())
    system_virtio_ss.add(when: 'CONFIG_VHOST_USER_SPI', if_true: files())
    system_virtio_ss.add(when: 'CONFIG_VHOST_USER_RTC', if_true: files())

    # PCI Stubs
    system_virtio_ss.add(when: ['CONFIG_VIRTIO_PCI', 'CONFIG_VHOST_USER_TEST'],
                         if_true: files())
    system_virtio_ss.add(when: ['CONFIG_VIRTIO_PCI', 'CONFIG_VHOST_USER_GPIO'],
                         if_true: files())
    system_virtio_ss.add(when: ['CONFIG_VIRTIO_PCI', 'CONFIG_VHOST_USER_I2C'],
                         if_true: files())
    system_virtio_ss.add(when: ['CONFIG_VIRTIO_PCI', 'CONFIG_VHOST_USER_RNG'],
                         if_true: files())
    system_virtio_ss.add(when: ['CONFIG_VIRTIO_PCI', 'CONFIG_VHOST_USER_SND'],
                         if_true: files())
    system_virtio_ss.add(when: ['CONFIG_VIRTIO_PCI', 'CONFIG_VHOST_USER_INPUT'],
                         if_true: files())
    system_virtio_ss.add(when: ['CONFIG_VIRTIO_PCI', 'CONFIG_VHOST_USER_SPI'],
                         if_true: files())
    system_virtio_ss.add(when: ['CONFIG_VIRTIO_PCI', 'CONFIG_VHOST_USER_RTC'],
                         if_true: files())
  endif
  if have_vhost_vdpa
    system_virtio_ss.add(files())
    system_virtio_ss.add(files())
  endif
endif
system_virtio_ss.add(when: 'CONFIG_VHOST_USER_VSOCK', if_true: files())
system_virtio_ss.add(when: 'CONFIG_VIRTIO_RNG', if_true: files())
system_virtio_ss.add(when: 'CONFIG_VIRTIO_RTC', if_true: files())

system_virtio_ss.add(when: 'CONFIG_VIRTIO_BALLOON', if_true: files())
system_virtio_ss.add(when: 'CONFIG_VHOST_USER_FS', if_true: files())
system_virtio_ss.add(when: 'CONFIG_VIRTIO_PMEM', if_true: files())
system_virtio_ss.add(when: 'CONFIG_VHOST_VSOCK', if_true: files())
system_virtio_ss.add(when: 'CONFIG_VIRTIO_MEM', if_true: files())
system_virtio_ss.add(when: 'CONFIG_VIRTIO_NSM', if_true: files())
system_virtio_ss.add(when: 'CONFIG_VIRTIO_NSM', if_true: [files(), libcbor])
system_virtio_ss.add(when: 'CONFIG_VHOST_USER_SCMI', if_true: files())

virtio_pci_ss = ss.source_set()
virtio_pci_ss.add(when: 'CONFIG_VHOST_VSOCK', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VHOST_USER_VSOCK', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VHOST_USER_BLK', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VHOST_USER_SCSI', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VHOST_SCSI', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VHOST_USER_FS', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VHOST_USER_SCMI', if_true: files())

virtio_pci_ss.add(when: 'CONFIG_VIRTIO_CRYPTO', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_INPUT_HOST', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_INPUT', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_RNG', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_RTC', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_NSM', if_true: [files(), libcbor])
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_BALLOON', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_9P', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_SCSI', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_BLK', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_NET', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_SERIAL', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_PMEM', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_IOMMU', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_MEM', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VHOST_VDPA_DEV', if_true: files())
virtio_pci_ss.add(when: 'CONFIG_VIRTIO_MD', if_true: files())

system_virtio_ss.add_all(when: 'CONFIG_VIRTIO_PCI', if_true: virtio_pci_ss)

system_ss.add_all(when: 'CONFIG_VIRTIO', if_true: system_virtio_ss)
stub_ss.add(files('vhost-stub.c'))
stub_ss.add(files('vhost-user-stub.c'))
stub_ss.add(files('virtio-stub.c'))
stub_ss.add(files('virtio-md-stubs.c'))

if have_hmp
  system_ss.add(files('virtio-hmp-cmds.c'))
endif

system_ss.add(when: 'CONFIG_ACPI', if_true: files('virtio-acpi.c'))
