#include <config.h>

#include "root.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "inode.h"

#include "../cd.h"

// return -1 for invalid names.
static int name_to_track(const char *name) {
	int num = 0;
	
	if (strncmp(name, "track", 5) != 0) {
		return -1;
	}

	for (int i=5; i<8; i++){
		char ch = name[i];
		if (('0' <= ch) && (ch <= '9')) {
			num = num*10 + (ch - '0');
		} else {
			return -1;
		}
	}
	
	if (strcmp(name+8, ".wav") != 0){
		return -1;
	}
	
	return num;
}

void fill_root_attr(struct stat *st) {
	//TODO: other fields?
	memset(st, 0, sizeof(*st));
	//st->st_dev
	st->st_ino = FUSE_ROOT_ID;
	st->st_mode = S_IFDIR | 0555; /* r-xr-xr-x */
	st->st_nlink = 2;
	//st->st_uid = /* TODO */
	//st->st_gid = /* TODO */
	//st->st_rdev /* Device number, if device. */
	st->st_size = 0;
	//st->st_blksize;	/* Optimal block size for I/O. */
	st->st_blocks = 0;
	//st->__time_t st_atime;
	//st->__syscall_ulong_t st_atimensec;
	//st->__time_t st_mtime;
	//st->__syscall_ulong_t st_mtimensec;
	//st->__time_t st_ctime;
	//st->__syscall_ulong_t st_ctimensec;
}

void fs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
	//printf("fs_lookup(... parent=%ld, name=\"%s\"\n", parent, name);
	
	struct fuse_entry_param entry;
	int track;
	
	if (parent != FUSE_ROOT_ID) {
		fuse_reply_err(req, ENOENT);
		return;
	}
	
	track = name_to_track(name);
	//printf("track is %d\n", track);
	
	if (track < 0) {
		fuse_reply_err(req, ENOENT);
		return;
	}
	
	entry.ino = track_to_inode(track);
	entry.generation = 1; // Don't reuse inodes.
	
	if (entry.ino < 0) {
		fuse_reply_err(req, ENOENT);
		return;
	}
	
	fill_file_attr(&entry.attr, track);
	
	//TODO:timeout
	entry.attr_timeout = 15.0;
	entry.entry_timeout = 15.0;
	
	fuse_reply_entry(req, &entry);
}

void fs_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi){
	//printf("fs_opendir(... ino=%ld\n", ino);
	
	if (ino != FUSE_ROOT_ID) {
		fuse_reply_err(req, ENOENT);
		return;
	}

	fuse_reply_open(req, fi);
}

// TODO should return "." and ".." too.
void fs_readdir(
	fuse_req_t req,
	fuse_ino_t ino,
	size_t size,
	off_t offset,
	struct fuse_file_info *fi
) {
	//printf("fs_readdir(... ino=%ld, size=%ld, offset=%ld\n", ino, size, offset);
	
	char *buf;
	struct stat st;
	size_t entsize;
	
	if (ino != FUSE_ROOT_ID) {
		fuse_reply_err(req, ENOENT);
		return;
	}
	
	if (offset > cdrom_track_count()) {
		fuse_reply_buf(req, NULL, 0);
		return;
	}
	
	if (offset == 0)
		offset = 1;
	
	buf = malloc(size);
	if (!buf) {
		fuse_reply_err(req, ENOMEM);
		return;
	}
	
	char name[] = "track000.wav";
	name[5] += offset / 100;
	name[6] += offset % 100 / 10;
	name[7] += offset % 10;
	
	// The other fields of 'st' are ignored.
	st.st_mode = S_IFREG;
	st.st_ino = track_to_inode(offset);
	entsize = fuse_add_direntry(req, buf, size, name, &st, offset+1);

	if (entsize > size) { // TODO refactor
		fuse_reply_err(req, ENOMEM);
	} else {
		fuse_reply_buf(req, buf, entsize);
	}

	free(buf);
}

void fs_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	//printf("fs_releasedir(... ino=%ld\n", ino);
	
	//No opoerations required.
	
	fuse_reply_err(req, 0);
}
