#include "readn.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>

ScopedFile::~ScopedFile() {
  if (!is_mmap_) {
    free(data_);
  } else {
    munmap(data_, sb_.st_size);
  }
  free(path_);
  close(fd_);
}

bool ScopedFile::Read() {
  fd_ = open(path_, O_RDONLY);
  if (fd_ == -1)
    return false;
  
  if (sb_.st_size < 1048576) {
    is_mmap_ = false;
    data_ = (char*) malloc(sb_.st_size);

    if (read(fd_, data_, sb_.st_size) != sb_.st_size)
      return false;
  } else {
    is_mmap_ = true;
    data_ = (char*) mmap(NULL, sb_.st_size, PROT_READ,
		 	 MAP_SHARED | MAP_POPULATE, fd_, 0);
    if (data_ == MAP_FAILED)
      return false;
  }
  return true;
}
