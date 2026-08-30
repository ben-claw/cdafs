#include "cmdline.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FUSE_USE_VERSION 26
#include <fuse/fuse_lowlevel.h>

enum {
	KEY_VERSION,
	KEY_HELP,
	KEY_foreground
};

#define CMDLINE_OPT(templ, field, value)\
	{ templ, offsetof(struct commandline_options, field), value }

struct fuse_opt cdfs_help_options[] = {
	 FUSE_OPT_KEY("-V",		KEY_VERSION),
	 FUSE_OPT_KEY("--version",	KEY_VERSION),
	 FUSE_OPT_KEY("-h",		KEY_HELP),
	 FUSE_OPT_KEY("--help",		KEY_HELP),
	 CMDLINE_OPT("-f",		foreground, 1),
	 CMDLINE_OPT("--foreground",	foreground, 1),
	 FUSE_OPT_END
};

static void fall_through_fuse_and_exit(char* arg0, char* arg1, int status) {
	char *argv[] = {arg0, arg1, NULL};
	struct fuse_args args =  FUSE_ARGS_INIT(2, argv);
	
	fflush(stdout);
	dup2(1, 2); // 2>&1
	fuse_lowlevel_new(&args, NULL, 0, NULL);
	//fuse_mount("", &args);
	exit(status);
}

static void print_version_and_exit(char* arg0, int status) {
	/***/
	fall_through_fuse_and_exit(arg0, "--version", status);
}

static void print_help_and_exit(char* arg0, int status) {
	/***/
	fall_through_fuse_and_exit(arg0, "--help", status);
}

static int opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs) {
	char *arg0 = outargs->argv[0];
	struct commandline_options *options = data;
	
	switch (key) {
	case KEY_VERSION:
		print_version_and_exit(arg0, 0);
	case KEY_HELP:
		print_help_and_exit(arg0, 0);
	case FUSE_OPT_KEY_NONOPT:
		if (options->device == NULL)
			options->device = arg;
		else if (options->mountpoint == NULL)
			options->mountpoint = arg;
		else 
			print_help_and_exit(arg0, 1);
		return 1;
	default:
		return -1;
	}
}

void cmdline_parse(int argc, char *argv[], struct commandline_options *options) {
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	options->argv_0 = argv[0];
	options->device = NULL;
	options->mountpoint = NULL;
	options->foreground = 0;

	fuse_opt_parse(&args, options, cdfs_help_options, opt_proc);
	if (options->device == NULL || options->mountpoint == NULL)
		print_help_and_exit(argv[0], 1);
}
