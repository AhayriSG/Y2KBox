system_ss.add(files('hid.c'))
system_ss.add(when: 'CONFIG_ADB', if_true: files())
system_ss.add(when: 'CONFIG_PCKBD', if_true: files('pckbd.c'))
system_ss.add(when: 'CONFIG_PL050', if_true: files())
system_ss.add(when: 'CONFIG_PS2', if_true: files('ps2.c'))
system_ss.add(when: 'CONFIG_STELLARIS_GAMEPAD', if_true: files())

system_ss.add(when: 'CONFIG_VIRTIO_INPUT', if_true: files())
system_ss.add(when: 'CONFIG_VIRTIO_INPUT', if_true: files())
system_ss.add(when: 'CONFIG_VIRTIO_INPUT_HOST', if_true: files())

system_ss.add(when: 'CONFIG_LASIPS2', if_true: files())
