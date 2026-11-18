common_ss.add(files('accel-common.c'))
system_ss.add(files('accel-system.c', 'accel-blocker.c', 'accel-qmp.c', 'accel-irq.c'))
user_ss.add(files('accel-user.c'))

subdir('tcg')
if have_system
  subdir('qtest')
  subdir('kvm')
  subdir('stubs')
endif

# qtest
system_ss.add(files('dummy-cpus.c'))
