#ifndef _CF_H_
#define _CF_H_

enum {
	CD_AUDIO_FRAME_SIZE = 2352,
};

int cdrom_init(const char* name);
int cdrom_is_disk_changed();
int cdrom_track_count();
int cdrom_track_len(int track);
int cdrom_frames_count();
int cdrom_read_frames(int track, int frame, unsigned char* buf, int count);

#endif
