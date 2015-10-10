/* COVERAGE: rename renameat */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

// To test for glibc support for renameat():
//
// Since glibc 2.10:
//	_XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L
// Before glibc 2.10:
//	_ATFILE_SOURCE

#define GLIBC_SUPPORT \
  (_XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L \
   || defined(_ATFILE_SOURCE))

int main()
{
  int fd1;

  fd1 = creat("file1", S_IREAD|S_IWRITE);
  close (fd1);
  
  mkdir("dir1", S_IREAD|S_IWRITE|S_IEXEC);
  mkdir("dir3", S_IREAD|S_IWRITE|S_IEXEC);

  fd1 = creat("dir3/file", S_IREAD|S_IWRITE);
  close (fd1);

  rename("file1", "file2");
  //staptest// rename ("file1", "file2") = 0

  rename("dir1", "dir2");
  //staptest// rename ("dir1", "dir2") = 0

  // This will fail since the target isn't empty.
  rename("dir2", "dir3");
  //staptest// rename ("dir2", "dir3") = -NNNN (ENOTEMPTY!!!!EEXIST)

  // This will fail since you can't rename a file to a directory.
  rename("file2", "dir2");
  //staptest// rename ("file2", "dir2") = -NNNN (EISDIR)

  // You can't rename a directory to a subdirectory of itself.
  rename("dir2", "dir2/dir4");
  //staptest// rename ("dir2", "dir2/dir4") = -NNNN (EINVAL)

  // You can't rename a directory to a file.
  rename("dir2", "file2");
  //staptest// rename ("dir2", "file2") = -NNNN (ENOTDIR)

  rename("file2", (char *)-1);
  //staptest// rename ("file2", XXXX) = -NNNN (EFAULT)

#if GLIBC_SUPPORT
  renameat(AT_FDCWD, "file2", AT_FDCWD, "file1");
  //staptest// renameat (AT_FDCWD, "file2", AT_FDCWD, "file1") = 0

  renameat(AT_FDCWD, "dir2", AT_FDCWD, "dir1");
  //staptest// renameat (AT_FDCWD, "dir2", AT_FDCWD, "dir1") = 0

  // This will fail since the target isn't empty.
  renameat(AT_FDCWD, "dir1", AT_FDCWD, "dir3");
  //staptest// renameat (AT_FDCWD, "dir1", AT_FDCWD, "dir3") = -NNNN (ENOTEMPTY!!!!EEXIST)

  // This will fail since you can't rename a file to a directory.
  renameat(AT_FDCWD, "file1", AT_FDCWD, "dir1");
  //staptest// renameat (AT_FDCWD, "file1", AT_FDCWD, "dir1") = -NNNN (EISDIR)

  // You can't rename a directory to a subdirectory of itself.
  renameat(AT_FDCWD, "dir1", AT_FDCWD, "dir1/dir4");
  //staptest// renameat (AT_FDCWD, "dir1", AT_FDCWD, "dir1/dir4") = -NNNN (EINVAL)

  // You can't rename a directory to a file.
  renameat(AT_FDCWD, "dir1", AT_FDCWD, "file1");
  //staptest// renameat (AT_FDCWD, "dir1", AT_FDCWD, "file1") = -NNNN (ENOTDIR)

  renameat(AT_FDCWD, "file1", AT_FDCWD, (char *)-1);
  //staptest// renameat (AT_FDCWD, "file1", AT_FDCWD, XXXX) = -NNNN (EFAULT)

#endif
  return 0;
}
