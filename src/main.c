#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <sys/select.h>

#include <linux/cdrom.h>

#include "cd.h"
#include "cmdline.h"
#include "fs/fs.h"

volatile static int main_terminated = 0;

static void sig_handler(int sig_num) {
	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
	main_terminated = (sig_num != 0);
}

static void init_signals(sigset_t *orig_mask) {
	sigset_t new_mask;

	signal(SIGHUP, SIG_IGN); /* ? */

	sigemptyset(&new_mask);
	sigaddset(&new_mask, SIGTERM);
	sigaddset(&new_mask, SIGINT);
	sigprocmask(SIG_BLOCK, &new_mask, orig_mask);

	sig_handler(0); /* set first signal handlers */
}

static void dump_args(const char *when, struct fuse_args *args) {
	printf("%s : argc = %d\n", when, args->argc);
	for (int i = 0; i < args->argc; i++) {
		printf("argv[%d] = %s\n", i, args->argv[i]);
	}
}

static int init_fuse(const struct commandline_options *options,
	struct fuse_session **session,
	struct fuse_chan **chan)
{
	size_t optstr_size;
	char *optstr = NULL;
	int res = 1;

	char *session_argv[] = { (char *)options->argv_0, NULL };
	struct fuse_args session_args = { 1, session_argv, 0 };
	dump_args("Session", &session_args);
	*session = fuse_lowlevel_new(&session_args, &fs, sizeof(fs), NULL);
	if (session == NULL) {
		printf("Error creating FUSE session\n");
		return 1;
	}
	
	char *optfmtstr = "-oro,nonempty,fsname=%s\0";
	optstr_size = snprintf(NULL, 0, optfmtstr, options->device) + 1;
	if (optstr_size < 1)
		goto cleanup;
	optstr = malloc(optstr_size);
	if (!optstr)
		goto cleanup;
	snprintf(optstr, optstr_size, optfmtstr, options->device);
	
	//fuse_opt_insert_arg(&args, 1, "-odefault_permissions,nonempty,big_writes,nodev,nosuid");
	
	char *mount_argv[] = { (char *)options->argv_0, optstr, NULL };
	struct fuse_args mount_args = { 2, mount_argv, 0 };
	dump_args("Mount", &mount_args);
	*chan = fuse_mount(options->mountpoint, &mount_args);
	if (chan == NULL) {
		printf("Error creating mountpoint at %s\n", options->mountpoint);
		goto cleanup;
	}
	
	// <fuse/fuse_lowlevel.h> Note: currently only a single channel may be 
	// assigned. This may change in the future
	fuse_session_add_chan(*session, *chan);
	
	res = fuse_daemonize(options->foreground);
	if (res != 0) {
		printf("Error fuse_daemonize(): %d\n", res);
		goto cleanup;
	}
	
	res = 0;
	
cleanup:
	if (optstr)
		free(optstr);
	
	return res;
}

int main(int argc, char *argv[]) {
	struct commandline_options options;
	struct fuse_session *session = NULL;
	struct fuse_chan *chan = NULL;
	size_t buf_size;
	char *buf_data = NULL;
	int chan_fd;
	fd_set ready_fds;
	sigset_t sig_mask;
	int res = 1;
	
	cmdline_parse(argc, argv, &options);
	cdrom_init(options.device);
	
	res = init_fuse(&options, &session, &chan);
	if (res != 0)
		goto cleanup;
	
	buf_size = fuse_chan_bufsize(chan);
	buf_data = malloc(buf_size);
	if (buf_data == NULL)
		goto cleanup;

	chan_fd = fuse_chan_fd(chan);
	FD_ZERO(&ready_fds);
	
	init_signals(&sig_mask);

	while (!main_terminated) {
		FD_SET(chan_fd, &ready_fds);
		int res = pselect(chan_fd + 1, &ready_fds, NULL, NULL, NULL, &sig_mask);
		if (res == 1 && FD_ISSET(chan_fd, &ready_fds)) {
			if (fuse_session_exited(session))
				break;
				
			res = fuse_chan_recv(&chan, buf_data, buf_size);
			//printf("Event recived, fd=%d size=%d/%ld\n", fd, res, buf_size);
			if (res < 0){
				printf("Error fuse_chan_recv(): %d\n", res);
				goto cleanup;
			}
			fuse_session_process(session, buf_data, res, chan);
			
			if (cdrom_is_disk_changed())
				break;
		} else if (res == -1 && errno == EINTR) {
			continue; /* signal caught */
		} else {
			printf("Something strange happens!\n");
		}
	}
	
	res = 0;
	
cleanup:
	if (buf_data)
		free(buf_data);

	if (chan != NULL) {
		fuse_session_remove_chan(chan);
		fuse_unmount(options.mountpoint, chan);
	}

	if (session != NULL)
		fuse_session_destroy(session);

	return res;
}
