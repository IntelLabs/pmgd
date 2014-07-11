#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>

#include "os.h"
#include "exception.h"

Jarvis::os::MapRegion::MapRegion(const char *db_name, const char *region_name,
                                 uint64_t map_addr, uint64_t map_len,
                                 bool &create, bool truncate, bool read_only)
{
    if (create) {
        // It doesn't matter if this step fails, either because the
        // name already exists, permissions are insufficient, or any
        // other reason. In any case, success or failure of the open
        // call is what is important.
        mkdir(db_name, 0777);
    }

    std::string filename = std::string(db_name) + "/" + region_name;
    int open_flags = read_only * O_RDONLY | !read_only * O_RDWR
                     | create * O_CREAT | truncate * O_TRUNC;
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
             PROT_READ | !read_only * PROT_WRITE,
             MAP_SHARED | MAP_FIXED, _fd, 0) == MAP_FAILED)
    {
        close(_fd);
        throw Exception(map_failed);
    }
}

Jarvis::os::MapRegion::~MapRegion()
{
    close(_fd);
}


// Linux delivers SIGBUS when an attempted access to a memory-mapped
// file cannot be satisfied, either because the access is beyond the
// end of the file or because there is no space left on the device.
// This code translates SIGBUS to an out-of-space exception.
// It is not portable to throw an exception from a signal handler.
// GCC supports it if the code is compiled with the -fnon-call-exceptions
// option.
// There are other reasons that a SIGBUS could occur. In most if not
// all cases, it is a programming error. This will mask such
// errors and make them appear to be an out-of-space condition.
// It might be possible to distinguish by examining the faulting
// address.
Jarvis::os::SigHandler::SigHandler()
{
    struct sigaction sa;
    sa.sa_handler = sigbus_handler;
    sa.sa_flags = 0;
    sigfillset(&sa.sa_mask);
    sigaction(SIGBUS, &sa, NULL);
}

void Jarvis::os::SigHandler::sigbus_handler(int)
{
    throw Exception(out_of_space);
}
