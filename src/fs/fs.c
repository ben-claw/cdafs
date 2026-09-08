#include <config.h>

#include "fs.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <sys/statvfs.h>

#include "file.h"
#include "inode.h"
#include "root.h"

#include "../cd.h"
#include "../debug.h"

static void fs_forget(fuse_req_t req, fuse_ino_t ino, unsigned long nlookup){
	DEBUG_BEGIN_FN();
	DEBUG_PRINT("fs_forget(...\n");
	DEBUG_END_FN();
}

static void fs_statfs(fuse_req_t req, fuse_ino_t ino){
	/* ino == 0 means "undefined" */
	DEBUG_BEGIN_FN();
	DEBUG_PRINT("fs_statfs(... ino=%ld\n", ino);
	
	struct statvfs st;
	
	//TODO: other fields?
	memset(&st, 0, sizeof(st));
	
	st.f_bsize = CD_AUDIO_FRAME_SIZE;
	st.f_frsize = CD_AUDIO_FRAME_SIZE;
	st.f_blocks = cdrom_frames_count();
	/*st.f_bfree = 0;
	st.f_bavail = 0;*/
	st.f_files = cdrom_track_count() + 1;
	/*st.f_ffree = 0;
	st.f_favail = 0;
	st.f_fsid = 0;*/
	st.f_flag = ST_NOSUID | ST_RDONLY;
#ifdef _GNU_SOURCE
	st.f_flag |= ST_NODEV | ST_NOEXEC;
#endif
	st.f_namemax = 12;
	
	fuse_reply_statfs(req, &st);
	DEBUG_END_FN();
}

static void fs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi){
	DEBUG_BEGIN_FN();
	DEBUG_PRINT("fs_getattr(... ino=%ld\n", ino);

	struct stat st;
	
	if (ino == FUSE_ROOT_ID){
		fill_root_attr(&st);
	} else 
	if (is_file_ino(ino)) {
		fill_file_attr(&st, ino - FILES_ID_START);
	} else {
		fuse_reply_err(req, ENOENT);
		goto cleanup;
	}
	
	fuse_reply_attr(req, &st, 15.0); //TODO:timeout

cleanup:
	DEBUG_END_FN();
}

static void fs_init (void *userdata, struct fuse_conn_info *conn){
	DEBUG_BEGIN_FN();
	DEBUG_PRINT("fs_init(...\n");
	DEBUG_END_FN();
}

static void fs_destroy (void *userdata){
	DEBUG_BEGIN_FN();
	DEBUG_PRINT("fs_destroy(...\n");
	DEBUG_END_FN();
}

/*static void fs_setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, size_t size, int flags){
	DEBUG_PRINT("fs_setxattr(...\n");
}
static void fs_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size){
	DEBUG_PRINT("fs_getxattr(...\n");
}
static void fs_listxattr(fuse_req_t req, fuse_ino_t ino, size_t size){
	DEBUG_PRINT("fs_listxattr(...\n");
}*/


struct fuse_lowlevel_ops fs = {
	.init		= fs_init,
	.destroy	= fs_destroy,
	.lookup		= fs_lookup,
	.forget		= fs_forget,
	.getattr	= fs_getattr,
	.open		= fs_open,
	.read		= fs_read,
	.release	= fs_release,
	.opendir	= fs_opendir,
	.readdir	= fs_readdir,
	.releasedir	= fs_releasedir,
	.statfs		= fs_statfs,
	//.setxattr	= fs_setxattr,
	//.getxattr	= fs_getxattr,
	//.listxattr	= fs_listxattr,
};
