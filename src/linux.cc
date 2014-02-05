#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "os.h"
#include "exception.h"

Jarvis::os::MapHandle::MapHandle(const char *dir, const char *name,
                                 uint64_t map_addr, uint64_t map_len,
                                 bool &create, bool truncate)
{
    int open_flags = O_RDWR | (create ? O_CREAT : 0) | (truncate ? O_TRUNC : 0);
    if ((_fd = open(name, open_flags)) < 0)
        throw e_open_failed;

    // check for size before mmap'ing
    struct stat sb;
    if (fstat(_fd, &sb) < 0) {
        close(_fd);
        throw e_open_failed;
    }

    if (sb.st_size == off_t(map_len)) {
        create = false;
    }
    else if (sb.st_size == 0 && create) {
        if (ftruncate(_fd, map_len) < 0) {
            close(_fd);
            throw e_open_failed;
        }
    }
    else {
        close(_fd);
        throw e_open_failed;
    }

    if (mmap((void *)map_addr, map_len,
             PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED,
             _fd, 0) == MAP_FAILED)
    {
        close(_fd);
        throw e_open_failed;
    }
}

Jarvis::os::MapHandle::~MapHandle()
{
    close(_fd);
}
