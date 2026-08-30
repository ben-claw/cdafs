#ifndef _INODE_H_
#define _INODE_H_

#include "fuse.h"
#include "../cd.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

enum {
	FILES_ID_START = FUSE_ROOT_ID + 1,
};

static int is_file_ino(fuse_ino_t ino) {
	return (FILES_ID_START < ino) &&
		(ino <= FILES_ID_START + cdrom_track_count());
}

static fuse_ino_t track_to_inode(int trk) {
	return trk + FILES_ID_START;
}

static int inode_to_track(fuse_ino_t ino) {
	return is_file_ino(ino) ? (ino - FILES_ID_START) : -1;
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif
