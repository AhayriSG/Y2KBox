nubus_ss = ss.source_set()
nubus_ss.add(files())
nubus_ss.add(files())
nubus_ss.add(files())
nubus_ss.add(files())
nubus_ss.add(when: 'CONFIG_Q800', if_true: files())

system_ss.add_all(when: 'CONFIG_NUBUS', if_true: nubus_ss)
