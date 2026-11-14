fs_ss = ss.source_set()
fs_ss.add(files(
  ))
if host_os == 'darwin'
  fs_ss.add(files())
elif host_os == 'freebsd'
  fs_ss.add(files())
elif host_os == 'linux'
  fs_ss.add(files())
endif
system_ss.add_all(when: 'CONFIG_FSDEV_9P', if_true: fs_ss)

system_ss.add(when: 'CONFIG_VIRTIO_9P', if_true: files())
