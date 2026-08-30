#ifndef _CMDLINE_H_
#define _CMDLINE_H_

struct commandline_options {
	const char *argv_0;
	const char *device;
	const char *mountpoint;
	int foreground;
};

void cmdline_parse(int argc, char *argv[], struct commandline_options *options);

#endif
