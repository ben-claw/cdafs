**English | [Русский](README_RU.md)**
# CD Audio Filesystem (CDAFS)
cdafs is a FUSE-based userspace filesystem that presents an audio CD as a regular directory containing WAV files. File contents are generated on demand when read and extracted directly from the audio CD. The program is designed to be easy to use and to have a minimal number of external dependencies.
## Building and installation
At present, the only way to install the program is to build it from source code yourself. You can obtain the program's source code in two ways:
1) Download an archive of a specific release:
  ```sh
  wget https://github.com/ben-claw/cdafs/releases/download/v0.1.0/cdafs-0.1.0.tar.gz
  tar -xf cdafs-0.1.0.tar.gz
  cd cdafs-0.1.0
  ```
2) Clone the current version from the repository (`git` and `GNU Autotools` are required):
  ```sh
  git clone https://github.com/ben-claw/cdafs
  cd cdafs
  autoreconf --install --force
  ```
Then build the program by running:
  ```sh
  CFLAGS="-O2" ./configure [--enable-debug] --prefix="$HOME/.local"
  make
  ```
The optional `--enable-debug` option enables additional debug output and is not recommended for normal users.
The `--prefix` option specifies the installation path prefix. The default value is `/usr/local`.

If the build completes successfully, the executable file `./src/mount.cdafs` will be created. To install it into `$(PREFIX)/sbin`, run (root privileges may be required):
  ```sh
  make install
  ```
## Usage
The program is very simple to use. Just create a mount point and run:
  ```sh
  mount.cdafs /dev/cdrom /mount/point/
  ```
After mounting, the `/mount/point/` directory will contain (pseudo-)files named `trackNNN.wav`, where NNN is the track number with leading zeros. These files can be copied to a local disk or played in your favorite player like normal WAV files.

Mounting does not require `root` privileges, but it does require read permissions for `/dev/cdrom` and permissions to mount at `/mount/point/`.
## Runtime dependencies
- a Linux kernel with FUSE support (tested with `6.18.35`);
- `libfuse` `2.6` or later (tested with `2.9.9`);
- the standard C library (tested with `glibc 2.43`);
- access to a CD drive.
## Build dependencies
- GNU Autoconf (tested with `2.72`);
- GNU Automake (tested with `1.18.1`);
- `libfuse` header files matching the installed library version;
- `pkg-config` (tested with `2.5.1`);
- a C compiler (tested with `gcc 15.3.0`);
- `make` (tested with `GNU Make 4.4.1`).
## Future Plans
- support for FUSE 3, with the option to choose between FUSE 2 and FUSE 3 at configuration time;
- switching to `libcdio`/`libcdio-paranoia` to enable portability across other Unix-like operating systems;
- support for access control and permissions for different users;
- bug fixes and improved stability.
## Feedback
The program is provided “as is”, without any warranties, but with best wishes. Please submit bug reports, suggestions for improvement, and general feedback on [GitHub](https://github.com/ben-claw/cdafs/issues) or send them to [ben-claw@mail.ru](mailto:ben-claw@mail.ru).

Thank you for your interest in the project and for your feedback!
