# SPDX-License-Identifier: GPL-2.0-or-later

vfio_ss = ss.source_set()
vfio_ss.add(files(
  ))
vfio_ss.add(when: 'CONFIG_KVM', if_true: files())
stub_ss.add(files('kvm-stubs.c'))
vfio_ss.add(when: 'CONFIG_PSERIES', if_true: files())
vfio_ss.add(when: ['CONFIG_KVM', 'CONFIG_PSERIES'], if_true: files())
vfio_ss.add(when: 'CONFIG_VFIO_PCI', if_true: files(
  ))
vfio_ss.add(when: 'CONFIG_VFIO_CCW', if_true: files())
vfio_ss.add(when: 'CONFIG_VFIO_AP', if_true: files())
vfio_ss.add(when: 'CONFIG_VFIO_IGD', if_true: files())
stub_ss.add(files('igd-stubs.c'))

system_ss.add_all(when: 'CONFIG_VFIO', if_true: vfio_ss)

system_ss.add(when: 'CONFIG_VFIO', if_true: files(
  ))
system_ss.add(when: ['CONFIG_VFIO', 'CONFIG_IOMMUFD'], if_true: files(
  ))
stub_ss.add(files('iommufd-stubs.c'))
system_ss.add(when: 'CONFIG_VFIO_PCI', if_true: files(
  ))
