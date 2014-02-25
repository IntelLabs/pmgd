#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "os.h"
#include "exception.h"

Jarvis::os::MapRegion::MapRegion(const char *db_name, const char *region_name,
                                 uint64_t map_addr, uint64_t map_len,
                                 bool &create, bool truncate)
{
    if (create) {
        // It doesn't matter if this step fails, either because the
        // name already exists, permissions are insufficient, or any
        // other reason. In any case, success or failure of the open
        // call is what is important.
        mkdir(db_name, 0777);
    }

    std::string filename = std::string(db_name) + "/" + region_name;
    int open_flags = O_RDWR | create * O_CREAT | truncate * O_TRUNC;
    if ((_fd = open(filename.c_str(), open_flags, 0666)) < 0)
        throw Exception(map_failed);

    // check for size before mmap'ing
    struct stat sb;
    if (fstat(_fd, &sb) < 0) {
        close(_fd);
        throw Exception(map_failed);
    }

    if (sb.st_size == off_t(map_len)) {
        create = false;
    }
    else if (sb.st_size == 0 && create) {
        if (ftruncate(_fd, map_len) < 0) {
            close(_fd);
            throw Exception(map_failed);
        }
    }
    else {
        close(_fd);
        throw Exception(map_failed);
    }

    if (mmap((void *)map_addr, map_len,
             PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED,
             _fd, 0) == MAP_FAILED)
    {
        close(_fd);
        throw Exception(map_failed);
    }
}

Jarvis::os::MapRegion::~MapRegion()
{
    close(_fd);
}
