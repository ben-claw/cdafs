#include <config.h>

#include "cd.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/cdrom.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "debug.h"

static int cdrom_fd;
static __s64 disk_timestamp;
static struct cdrom_tochdr tochdr;
static int track_addr[256];

/*return -1 if error or changed, 0 if not.*/
static int check_disk_changed() {
	DEBUG_BEGIN_FN();

	int res = -1;

	if (disk_timestamp < 0)
		goto cleanup;

	struct cdrom_timed_media_change_info info = {
		.last_media_change = disk_timestamp,
		.media_flags = 0
	};

	res = ioctl(cdrom_fd, CDROM_TIMED_MEDIA_CHANGE, &info);
	if (res < 0)
		goto cleanup;

	DEBUG_PRINT
	        ("Checked for media chnged: diskts=%lld, lastts=%lld; chflag=%lld\n",
	         disk_timestamp, info.last_media_change, info.media_flags);

	if (info.media_flags & MEDIA_CHANGED_FLAG) {
		disk_timestamp = -1;
		res = -1;
	}
	/*if (info.last_media_change > disk_timestamp) */

cleanup:
	DEBUG_END_FN();
	return res;
}

int cdrom_is_disk_changed() {
	return (disk_timestamp < 0);
}

int cdrom_init(const char *name) {
	DEBUG_BEGIN_FN();
	DEBUG_PRINT("cdrom_init(... name=%s\n", name);

	int res = -1;

	struct cdrom_timed_media_change_info info = {
		.last_media_change = 0,
		.media_flags = 0
	};

	cdrom_fd = open(name, O_RDONLY | O_NONBLOCK);
	if (cdrom_fd < 0)
		goto cleanup;

	res = ioctl(cdrom_fd, CDROM_TIMED_MEDIA_CHANGE, &info);
	if (res < 0)
		goto cleanup;
	disk_timestamp = info.last_media_change;

	/* Is tochdr.cdth_trk0 is always 1? */
	res = ioctl(cdrom_fd, CDROMREADTOCHDR, &tochdr);
	if (res < 0)
		goto cleanup;
	DEBUG_PRINT("Tracks: %u - %u\n", tochdr.cdth_trk0, tochdr.cdth_trk1);

	struct cdrom_tocentry entry;

	for (int i = tochdr.cdth_trk0; i <= tochdr.cdth_trk1; i++) {
		entry.cdte_track = i;
		entry.cdte_format = CDROM_LBA;
		res = ioctl(cdrom_fd, CDROMREADTOCENTRY, &entry);
		if (res < 0)
			goto cleanup;
		track_addr[i - 1] = entry.cdte_addr.lba;
		DEBUG_PRINT("Track %02u at %u\n", i, entry.cdte_addr.lba);
	}

	res = ioctl(cdrom_fd, CDROM_LAST_WRITTEN,
	            &track_addr[tochdr.cdth_trk1]);
	if (res < 0)
		goto cleanup;
	DEBUG_PRINT("Last track ends at %u\n", track_addr[tochdr.cdth_trk1]);

cleanup:
	if (res < 0)
		DEBUG_PRINT("errno = %d\n", errno);
	DEBUG_END_FN();
	return res;
}

int cdrom_track_count() {
	int res = check_disk_changed();

	if (res < 0)
		return res;

	return tochdr.cdth_trk1 - tochdr.cdth_trk0 + 1;
}

int cdrom_track_len(int track) {
	int res = check_disk_changed();

	if (res < 0)
		return res;

	return track_addr[track] - track_addr[track - 1];
}

int cdrom_frames_count() {
	int res = check_disk_changed();

	if (res < 0)
		return res;

	return track_addr[tochdr.cdth_trk1];
}

int cdrom_read_frames(int track, int frame, unsigned char *buf, int count) {
	DEBUG_BEGIN_FN();
	DEBUG_PRINT("cdrom_read_frames(... track=%d, frame=%d, count=%d\n",
	            track, frame, count);

	int res = check_disk_changed();

	if (res < 0)
		goto cleanup;

	track = (tochdr.cdth_trk0 - 1 + track) - 1;

	struct cdrom_read_audio ra = {
		.addr.lba = track_addr[track] + frame, //TODO: check bounds
		.addr_format = CDROM_LBA,
		.nframes = count,
		.buf = buf
	};

	DEBUG_PRINT("Read %d frames from %d\n", ra.nframes, ra.addr.lba);
	res = ioctl(cdrom_fd, CDROMREADAUDIO, &ra);
	if (res < 0)
		DEBUG_PRINT("errno = %d\n", errno);

cleanup:
	DEBUG_END_FN();
	return res;
}
