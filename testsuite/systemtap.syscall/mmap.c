/* COVERAGE: mmap2 munmap msync mlock mlockall munlock munlockall mprotect mremap fstat open close */
#include <sys/types.h>
#include <sys/stat.h>
#define __USE_GNU
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
	int fd, ret;
	struct stat fs;
	void * r;

	/* create a file with something in it */
	fd = open("foobar",O_WRONLY|O_CREAT|O_TRUNC, 0600);
	//staptest// open ("foobar", O_WRONLY|O_CREAT[[[[.O_LARGEFILE]]]]?|O_TRUNC, 0600) = NNNN

	// Why 64k? ppc64 has 64K pages. ia64 has 16k
	// pages. x86_64/i686 has 4k pages. When we specify an offset
	// to mmap(), it must be a multiple of the page size, so we
	// use the biggest.
	lseek(fd, 65536, SEEK_SET);
	write(fd, "abcdef", 6);
	close(fd);
	//staptest// close (NNNN) = 0

	fd = open("foobar", O_RDONLY);
	//staptest// open ("foobar", O_RDONLY[[[[.O_LARGEFILE]]]]?) = NNNN

	/* stat for file size */
	ret = fstat(fd, &fs);
	//staptest// fstat (NNNN, XXXX) = 0

	r = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd, 0);
	//staptest// mmap[2]* (0x0, 4096, PROT_READ, MAP_SHARED, NNNN, 0) = XXXX

	mlock(r, 4096);
	//staptest// mlock (XXXX, 4096) = 0

	msync(r, 4096, MS_SYNC);	
	//staptest// msync (XXXX, 4096, MS_SYNC) = 0

	munlock(r, 4096);
	//staptest// munlock (XXXX, 4096) = 0

	mlockall(MCL_CURRENT);
	//staptest// mlockall (MCL_CURRENT) = 

	munlockall();
	//staptest// munlockall () = 0

	munmap(r, 4096);
	//staptest// munmap (XXXX, 4096) = 0

	// Ensure the 6th argument is handled correctly..
	r = mmap(NULL, 6, PROT_READ, MAP_PRIVATE, fd, 65536);
	//staptest// mmap[2]* (0x0, 6, PROT_READ, MAP_PRIVATE, NNNN, 65536) = XXXX

	munmap(r, 6);
	//staptest// munmap (XXXX, 6) = 0

	close(fd);

	r = mmap(NULL, 12288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	//staptest// mmap[2]* (0x0, 12288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = XXXX

	mprotect(r, 4096, PROT_READ);
	//staptest// mprotect (XXXX, 4096, PROT_READ) = 0

	munmap(r, 12288);
	//staptest// munmap (XXXX, 12288) = 0

	r = mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	//staptest// mmap[2]* (0x0, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = XXXX

	r = mremap(r, 8192, 4096, 0);
	//staptest// mremap (XXXX, 8192, 4096, 0x0, XXXX) = XXXX

	munmap(r, 4096);
	//staptest// munmap (XXXX, 4096) = 0

	// powerpc64's glibc rejects this one. On ia64, the call
	// fails, while on most other architectures it works. So,
	// ignore the return values.
#ifndef __powerpc64__
	r = mmap((void *)-1, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	//staptest// mmap[2]* (0x[f]+, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 

	munmap(r, 8192);
	//staptest// munmap (XXXX, 8192) = 
#endif

	r = mmap(NULL, -1, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#if __WORDSIZE == 64
	//staptest// mmap[2]* (0x0, 18446744073709551615, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = -XXXX (ENOMEM)
#else
	//staptest// mmap[2]* (0x0, 4294967295, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = -XXXX (ENOMEM)
#endif

	// powerpc's glibc (both 32-bit and 64-bit) rejects this one.
#ifndef __powerpc__
	r = mmap(NULL, 8192, -1, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	//staptest// mmap[2]* (0x0, 8192, PROT_[^ ]+|XXXX, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = XXXX

	munmap(r, 8192);
	//staptest// munmap (XXXX, 8192) = 0
#endif

	r = mmap(NULL, 8192, PROT_READ|PROT_WRITE, -1, -1, 0);
	//staptest// mmap[2]* (0x0, 8192, PROT_READ|PROT_WRITE, MAP_[^ ]+|XXXX, -1, 0) = -XXXX

	// Unfortunately, glibc and/or the syscall wrappers will
	// reject a -1 offset, especially on a 32-bit exe on a 64-bit
	// OS. So, we can't really test this one.
	//
	// r = mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, -1);

	return 0;
}
