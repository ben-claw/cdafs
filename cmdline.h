#ifndef _CMDLINE_H_
#define _CMDLINE_H_

//TODO Shoul I send raw argc+argv instead fuse_args?
#define FUSE_USE_VERSION 26
#include <fuse/fuse_lowlevel.h>

struct commandline_options {
	const char *argv_0;
	const char *device;
	const char *mountpoint;
	int foreground;
};

void cmdline_parse(int argc, char *argv[], struct commandline_options *options);

#endif
