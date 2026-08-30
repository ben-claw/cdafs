#ifndef _ROOT_H_
#define _ROOT_H_

#include <sys/stat.h>
#include "fuse.h"

void fill_root_attr(struct stat *st);

void fs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name);
void fs_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
void fs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t offset, struct fuse_file_info *fi);
void fs_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

#endif
