system_ss.add(files('hardware-info.c', 'ovmf-log.c'))

uefi_vars_ss = ss.source_set()
if (config_all_devices.has_key('CONFIG_UEFI_VARS'))
  uefi_vars_ss.add(files())
  uefi_vars_ss.add(when: gnutls,
                   if_true: files(),
                   if_false: files())
  uefi_vars_ss.add(files())
endif

modules += { 'hw-uefi' : {
    'vars'     : uefi_vars_ss,
}}
