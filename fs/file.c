#include "file.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inode.h"

#include "../cd.h"

enum {
	FRAMES_IN_BLOCK = 75,
	BLOCK_SIZE = CD_AUDIO_FRAME_SIZE * FRAMES_IN_BLOCK,
};

enum {
	WAV_HEADER_LEN = 44,
	WAV_FORMAT = 1, //PCM
	WAV_CHANNELS = 2,
	WAV_SAMPLE_RATE = 44100,
	WAV_BITS_PER_SAMPLE = 16,
};

struct cda_file_handle {
	int track_num;
	int track_len;
	int cached_block_num;
	unsigned char cached_block[BLOCK_SIZE];
};

static size_t min_sz(size_t a, size_t b){
    if (a<b) {
        return a;
    } else {
        return b;
    }
}

void fill_file_attr(struct stat *st, int track) {
	//TODO: other fields?
	memset(st, 0, sizeof(*st));
	//st->st_dev =
	st->st_ino = track_to_inode(track);
	st->st_mode = S_IFREG | 0444; /* r--r--r-- */
	st->st_nlink = 1;
	//st->st_uid = /*TODO*/
	//st->st_gid = /*TODO*/
	//st->st_rdev =
	st->st_size = WAV_HEADER_LEN + cdrom_track_len(track) * CD_AUDIO_FRAME_SIZE;
	
	/* Seems that everyone ignores this value. */
	st->st_blksize = CD_AUDIO_FRAME_SIZE;
	
	/* Not quite correct but better than 0. */
	st->st_blocks = (st->st_size + 511) / 512;
	
	//st->__time_t st_atime;
	//st->__syscall_ulong_t st_atimensec;
	//st->__time_t st_mtime;
	//st->__syscall_ulong_t st_mtimensec;
	//st->__time_t st_ctime;
	//st->__syscall_ulong_t st_ctimensec;
}

void fs_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
	//printf("fs_open(... ino=%ld\n", ino);
	
	struct cda_file_handle *fh;

	int track = inode_to_track(ino);
	if (track < 0) {
		fuse_reply_err(req, ENOENT);
		return;
	}

	// flags (with the exception of O_CREAT, O_EXCL, O_NOCTTY and O_TRUNC)
	if ((fi->flags & O_ACCMODE) != O_RDONLY) {
		fuse_reply_err(req, EACCES);
		return;
	}
	
	fh = malloc(sizeof(*fh));
	if (!fh) {
		fuse_reply_err(req, ENOMEM);
		return;
	}
	
	fh->track_num = track;
	fh->track_len = cdrom_track_len(track);
	fh->cached_block_num = -1;
	
	if (fh->track_len < 0) {
		fuse_reply_err(req, EIO);
		return;
	}
	
	fi->fh = (uint64_t)fh;

	fuse_reply_open(req, fi);
}

#define LE16(x) (x) & 0xff, (x)>> 8 & 0xff
#define LE32(x) (x) & 0xff, (x)>> 8 & 0xff, (x)>> 16 & 0xff, (x)>> 24 & 0xff

static size_t fill_wav_header(
	char *buf_pos, 
	off_t *offset,
	size_t *to_send,
	size_t data_size
) {
	char header[WAV_HEADER_LEN] = {
		'R', 'I', 'F', 'F',
		LE32(data_size + WAV_HEADER_LEN - 8), // file size
		'W', 'A', 'V', 'E',
		
		'f', 'm', 't', ' ',
		LE32(16), // fmt chunk size
		LE16(WAV_FORMAT),
		LE16(WAV_CHANNELS),
		LE32(WAV_SAMPLE_RATE),
		LE32(WAV_SAMPLE_RATE * WAV_CHANNELS * WAV_BITS_PER_SAMPLE / 8),
		LE16(WAV_CHANNELS * WAV_BITS_PER_SAMPLE / 8),
		LE16(WAV_BITS_PER_SAMPLE),
		
		'd', 'a', 't', 'a',
		LE32(data_size) // data chunk size
	};
	
	size_t slc = min_sz(WAV_HEADER_LEN - *offset, *to_send);

	//printf("fs_read(... sending header %ld\n", slc);

	memcpy(buf_pos, &header + *offset, slc); //TODO Check non-zero offsets

	*offset += slc;
	*to_send -= slc;
	return slc;
}

void fs_read(
	fuse_req_t req,
	fuse_ino_t ino,
	size_t size,
	off_t offset,
	struct fuse_file_info *fi
) {
	//printf("fs_read(... ino=%ld, size=%ld, offset=%ld\n", ino, size, offset);
	
	struct cda_file_handle *fh = (struct cda_file_handle*) fi->fh;
	
	char* buf;
	size_t data_size = fh->track_len * CD_AUDIO_FRAME_SIZE;
	size_t file_size = data_size + WAV_HEADER_LEN;

	if (offset >= file_size) {
		fuse_reply_buf(req, NULL, 0);
		return;
	}
	
	if (offset + size > file_size)
		size = file_size - offset;
	
	buf = malloc(size);
	if (!buf) {
		fuse_reply_err(req, ENOMEM);
		return;
	}
	
	char* buf_pos = buf;
	size_t to_send = size;
	
	if (offset < WAV_HEADER_LEN) {
		buf_pos += fill_wav_header(buf_pos, &offset, &to_send, data_size);
	}
	
	size_t data_offset = offset - WAV_HEADER_LEN;

	while (to_send > 0){
		int block_num = data_offset / BLOCK_SIZE;
		int block_pos = data_offset % BLOCK_SIZE;
		size_t slc = min_sz(BLOCK_SIZE - block_pos, to_send);
		
		if (fh->cached_block_num != block_num) {
			int res = cdrom_read_frames(fh->track_num, 
				block_num * FRAMES_IN_BLOCK, 
				fh->cached_block, 
				FRAMES_IN_BLOCK);
			if (res < 0) {
				fuse_reply_err(req, EIO);
				return;
			}
			fh->cached_block_num = block_num;
		}
		
		//printf("fs_read(... sending data %ld\n", slc);
		memcpy(buf_pos, fh->cached_block + block_pos, slc);
		
		buf_pos += slc;
		data_offset += slc;
		to_send -= slc;
	}
	
	//printf("fs_read(... buf prepared %ld\n", size);

	fuse_reply_buf(req, buf, size);
	free(buf);
}

void fs_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	//printf("fs_release(... ino=%ld\n", ino);
	
	struct cda_file_handle *fh = (struct cda_file_handle*) fi->fh;
	
	free(fh);

	fuse_reply_err(req, 0);
}
