hw_display_modules = {}

system_ss.add(when: 'CONFIG_DDC', if_true: files())
system_ss.add(when: 'CONFIG_EDID', if_true: files('edid-generate.c', 'edid-region.c'))

system_ss.add(when: 'CONFIG_FW_CFG_DMA', if_true: files('ramfb.c'))
stub_ss.add(files('ramfb-stubs.c'))
system_ss.add(when: 'CONFIG_FW_CFG_DMA', if_true: files('ramfb-standalone.c'))

system_ss.add(when: 'CONFIG_VGA_CIRRUS', if_true: files('cirrus_vga.c'))
system_ss.add(when: ['CONFIG_VGA_CIRRUS', 'CONFIG_VGA_ISA'], if_true: files())
system_ss.add(when: 'CONFIG_G364FB', if_true: files())
system_ss.add(when: 'CONFIG_JAZZ_LED', if_true: files())
system_ss.add(when: 'CONFIG_PL110', if_true: files())
system_ss.add(when: 'CONFIG_IMX6UL_LCDIF', if_true: files())
system_ss.add(when: 'CONFIG_SII9022', if_true: files())
system_ss.add(when: 'CONFIG_SSD0303', if_true: files())
system_ss.add(when: 'CONFIG_SSD0323', if_true: files())

system_ss.add(when: 'CONFIG_VGA_PCI', if_true: files('vga-pci.c'))
system_ss.add(when: 'CONFIG_VGA_ISA', if_true: files())
system_ss.add(when: 'CONFIG_VGA_MMIO', if_true: files())
system_ss.add(when: 'CONFIG_VMWARE_VGA', if_true: files())
system_ss.add(when: 'CONFIG_BOCHS_DISPLAY', if_true: files())

system_ss.add(when: 'CONFIG_EXYNOS4', if_true: files())
system_ss.add(when: 'CONFIG_FRAMEBUFFER', if_true: files())

system_ss.add(when: 'CONFIG_RASPI', if_true: files())
system_ss.add(when: 'CONFIG_SM501', if_true: files())
system_ss.add(when: 'CONFIG_TCX', if_true: files())
system_ss.add(when: 'CONFIG_CG3', if_true: files())
system_ss.add(when: 'CONFIG_MACFB', if_true: files())
system_ss.add(when: 'CONFIG_NEXTCUBE', if_true: files())

system_ss.add(when: 'CONFIG_VGA', if_true: files('vga.c'))
system_ss.add(when: 'CONFIG_VIRTIO', if_true: files())
system_ss.add(when: 'CONFIG_DM163', if_true: files())
system_ss.add(when: 'CONFIG_VHOST_USER_MEDIA', if_true: files())
system_ss.add(when: ['CONFIG_VHOST_USER_MEDIA', 'CONFIG_VIRTIO_PCI'],
  if_true: files())

stub_ss.add([files('acpi-vga-stub.c'), pixman])
if (config_all_devices.has_key('CONFIG_VGA_CIRRUS') or
    config_all_devices.has_key('CONFIG_VGA_PCI') or
    config_all_devices.has_key('CONFIG_VMWARE_VGA') or
    config_all_devices.has_key('CONFIG_ATI_VGA')
   )
  system_ss.add(when: 'CONFIG_ACPI', if_true: files('acpi-vga.c'))
endif

if config_all_devices.has_key('CONFIG_QXL')
  qxl_ss = ss.source_set()
  qxl_ss.add(when: 'CONFIG_QXL', if_true: [files(),
                                           pixman, spice])
  qxl_ss.add(when: 'CONFIG_ACPI', if_true: files('acpi-vga.c'))
  hw_display_modules += {'qxl': qxl_ss}
endif

system_ss.add(when: 'CONFIG_DPCD', if_true: files())
system_ss.add(when: 'CONFIG_XLNX_DISPLAYPORT', if_true: files())

system_ss.add(when: 'CONFIG_ARTIST', if_true: files())

system_ss.add(when: 'CONFIG_ATI_VGA', if_true: [files(), pixman])

system_ss.add(when: [pvg, 'CONFIG_MAC_PVG_PCI'],     if_true: [files('apple-gfx.m', 'apple-gfx-pci.m')])
system_ss.add(when: [pvg, 'CONFIG_MAC_PVG_MMIO'],    if_true: [files('apple-gfx.m', 'apple-gfx-mmio.m')])

if config_all_devices.has_key('CONFIG_VIRTIO_GPU')
  virtio_gpu_ss = ss.source_set()
  virtio_gpu_ss.add(when: 'CONFIG_VIRTIO_GPU',
                    if_true: [files(), pixman])
  if host_os == 'linux'
    virtio_gpu_ss.add(files())
  else
    virtio_gpu_ss.add(files())
  endif
  virtio_gpu_ss.add(when: 'CONFIG_VHOST_USER_GPU', if_true: files())
  hw_display_modules += {'virtio-gpu': virtio_gpu_ss}

  if virgl.found() and opengl.found()
    virtio_gpu_gl_ss = ss.source_set()
    virtio_gpu_gl_ss.add(when: ['CONFIG_VIRTIO_GPU', virgl, opengl],
                         if_true: [files(), pixman, virgl])
    hw_display_modules += {'virtio-gpu-gl': virtio_gpu_gl_ss}
  endif

  if rutabaga.found()
    virtio_gpu_rutabaga_ss = ss.source_set()
    virtio_gpu_rutabaga_ss.add(when: ['CONFIG_VIRTIO_GPU', rutabaga],
                               if_true: [files(), pixman])
    hw_display_modules += {'virtio-gpu-rutabaga': virtio_gpu_rutabaga_ss}
  endif
endif

if config_all_devices.has_key('CONFIG_VIRTIO_PCI')
  virtio_gpu_pci_ss = ss.source_set()
  virtio_gpu_pci_ss.add(when: ['CONFIG_VIRTIO_GPU', 'CONFIG_VIRTIO_PCI'],
                        if_true: [files(), pixman])
  virtio_gpu_pci_ss.add(when: ['CONFIG_VHOST_USER_GPU', 'CONFIG_VIRTIO_PCI'],
                        if_true: files())
  hw_display_modules += {'virtio-gpu-pci': virtio_gpu_pci_ss}

  if virgl.found() and opengl.found()
    virtio_gpu_pci_gl_ss = ss.source_set()
    virtio_gpu_pci_gl_ss.add(when: ['CONFIG_VIRTIO_GPU', 'CONFIG_VIRTIO_PCI', virgl, opengl],
                             if_true: [files(), pixman])
    hw_display_modules += {'virtio-gpu-pci-gl': virtio_gpu_pci_gl_ss}
  endif
  if rutabaga.found()
    virtio_gpu_pci_rutabaga_ss = ss.source_set()
    virtio_gpu_pci_rutabaga_ss.add(when: ['CONFIG_VIRTIO_GPU', 'CONFIG_VIRTIO_PCI', rutabaga],
                                   if_true: [files(), pixman])
    hw_display_modules += {'virtio-gpu-pci-rutabaga': virtio_gpu_pci_rutabaga_ss}
  endif
endif

if config_all_devices.has_key('CONFIG_VIRTIO_VGA')
  virtio_vga_ss = ss.source_set()
  virtio_vga_ss.add(when: 'CONFIG_VIRTIO_VGA',
                    if_true: [files(), pixman])
  virtio_vga_ss.add(when: 'CONFIG_VHOST_USER_VGA',
                    if_true: files())
  virtio_vga_ss.add(when: 'CONFIG_ACPI', if_true: files('acpi-vga.c'))
  hw_display_modules += {'virtio-vga': virtio_vga_ss}

  if virgl.found() and opengl.found()
    virtio_vga_gl_ss = ss.source_set()
    virtio_vga_gl_ss.add(when: ['CONFIG_VIRTIO_VGA', virgl, opengl],
                         if_true: [files(), pixman])
    virtio_vga_gl_ss.add(when: 'CONFIG_ACPI', if_true: files('acpi-vga.c'))
    hw_display_modules += {'virtio-vga-gl': virtio_vga_gl_ss}
  endif

  if rutabaga.found()
    virtio_vga_rutabaga_ss = ss.source_set()
    virtio_vga_rutabaga_ss.add(when: ['CONFIG_VIRTIO_VGA', rutabaga],
                               if_true: [files(), pixman])
    virtio_vga_rutabaga_ss.add(when: 'CONFIG_ACPI', if_true: files('acpi-vga.c'))
    hw_display_modules += {'virtio-vga-rutabaga': virtio_vga_rutabaga_ss}
  endif
endif

system_ss.add(when: 'CONFIG_OMAP', if_true: files())

modules += { 'hw-display': hw_display_modules }
