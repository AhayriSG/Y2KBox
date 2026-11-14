# XP fork: only the x86 BIOS, VGA BIOSes, NIC PXE ROMs and option ROMs remain.
roms = []

blobs = [
  'bios.bin',
  'bios-256k.bin',
  'vgabios.bin',
  'vgabios-cirrus.bin',
  'vgabios-stdvga.bin',
  'pxe-e1000.rom',
  'pxe-pcnet.rom',
  'pxe-rtl8139.rom',
  'efi-e1000.rom',
  'efi-pcnet.rom',
  'efi-rtl8139.rom',
  'multiboot_dma.bin',
  'linuxboot_dma.bin',
  'kvmvapic.bin',
  'pvh.bin',
]

if get_option('install_blobs')
  install_data(blobs, install_dir: qemu_datadir, install_mode: 'rw-r--r--')
endif

subdir('keymaps')
