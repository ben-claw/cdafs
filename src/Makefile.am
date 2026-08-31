## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = subdir-objects

sbin_PROGRAMS = mount.cdafs

mount_cdafs_SOURCES = \
	fs/file.h fs/file.c \
	fs/fs.h fs/fs.c \
	fs/inode.h \
	fs/root.h fs/root.c \
	cd.h cd.c \
	cmdline.h cmdline.c \
	main.c
mount_cdafs_LDADD =  @FUSE_LIBS@
mount_cdafs_CFLAGS = -Wall @FUSE_CFLAGS@
