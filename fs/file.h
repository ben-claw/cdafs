#ifndef _FILE_H_
#define _FILE_H_

#include <sys/stat.h>
#include "fuse.h"

void fill_file_attr(struct stat *st, int track);

void fs_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
void fs_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t offset, struct fuse_file_info *fi);
void fs_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

#endif
