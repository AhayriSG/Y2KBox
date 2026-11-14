system_ss.add(files('core.c'))
system_ss.add(when: 'CONFIG_CPU_CLUSTER', if_true: files())

system_ss.add(when: 'CONFIG_ARM11MPCORE', if_true: files())
system_ss.add(when: 'CONFIG_REALVIEW', if_true: files())
system_ss.add(when: 'CONFIG_A9MPCORE', if_true: files())
system_ss.add(when: 'CONFIG_A15MPCORE', if_true: files())
