common_ss.add(when: 'CONFIG_ALPHA_DIS', if_true: files())
common_ss.add(when: 'CONFIG_HEXAGON_DIS', if_true: files())
common_ss.add(when: 'CONFIG_HPPA_DIS', if_true: files())
common_ss.add(when: 'CONFIG_M68K_DIS', if_true: files())
common_ss.add(when: 'CONFIG_MICROBLAZE_DIS', if_true: files())
common_ss.add(when: 'CONFIG_MIPS_DIS', if_true: files())
common_ss.add(when: 'CONFIG_RISCV_DIS', if_true: files(
    
))
common_ss.add(when: 'CONFIG_SH4_DIS', if_true: files())
common_ss.add(when: 'CONFIG_SPARC_DIS', if_true: files())
common_ss.add(when: 'CONFIG_XTENSA_DIS', if_true: files())
common_ss.add(when: capstone, if_true: [files('capstone.c'), capstone])
common_ss.add(when: 'CONFIG_TCG', if_true: files(
    'disas-host.c',
    'disas-target.c',
    'objdump.c'
))
common_ss.add(files('disas-common.c'))
if have_hmp
  system_ss.add(files('disas-mon.c'))
endif
specific_ss.add(capstone)
