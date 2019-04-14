#include <sys/stat.h>
#include <stddef.h>

struct ScopedFile {
 public:
  ScopedFile(char* path, struct stat sb): path_(path), is_mmap_(false), sb_(sb) {}
  ~ScopedFile();

  bool Read();
  char* path() { return path_; }
  char* data() { return data_; }
  size_t size() { return sb_.st_size; }

 private:
  char* path_;
  int fd_;
  bool is_mmap_;
  struct stat sb_;
  char* data_;
};
