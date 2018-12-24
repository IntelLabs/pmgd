/**
 * @file   linux.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>
#include <errno.h>
#include <list>

#include "os.h"
#include "exception.h"

class PMGD::os::MapRegion::OSMapRegion {
    int _fd;
public:
    OSMapRegion(const char *db_name, const char *region_name,
                uint64_t map_addr, uint64_t map_len,
                bool &create, bool truncate, bool read_only);

    ~OSMapRegion();
};

PMGD::os::MapRegion::MapRegion(const char *db_name, const char *region_name,
                                 uint64_t map_addr, uint64_t map_len,
                                 bool &create, bool truncate, bool read_only)
    : _s(new OSMapRegion(db_name, region_name, map_addr, map_len,
                         create, truncate, read_only))
{
}

PMGD::os::MapRegion::~MapRegion()
{
    delete _s;
}

PMGD::os::MapRegion::OSMapRegion::OSMapRegion
    (const char *db_name, const char *region_name,
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
        throw PMGDException(OpenFailed, errno, filename + " (open)");

    // check for size before mmap'ing
    struct stat sb;
    if (fstat(_fd, &sb) < 0) {
        int err = errno;
        close(_fd);
        throw PMGDException(OpenFailed, err, filename + " (fstat)");
    }

    if (sb.st_size == off_t(map_len)) {
        create = false;
    }
    else if (sb.st_size == 0 && create) {
        if (read_only)
            throw PMGDException(ReadOnly);

        if (ftruncate(_fd, map_len) < 0) {
            int err = errno;
            close(_fd);
            throw PMGDException(OpenFailed, err, filename + " (ftruncate)");
        }
    }
    else {
        close(_fd);
        throw PMGDException(OpenFailed, filename + " was not the expected size");
    }

    if (mmap((void *)map_addr, map_len,
             PROT_READ | !read_only * PROT_WRITE,
             MAP_SHARED | MAP_FIXED, _fd, 0) == MAP_FAILED)
    {
        int err = errno;
        close(_fd);
        throw PMGDException(OpenFailed, err, filename + " (mmap)");
    }
}

PMGD::os::MapRegion::OSMapRegion::~OSMapRegion()
{
    close(_fd);
}

static const unsigned PAGE_SIZE = 4096;
static const unsigned PAGE_OFFSET = PAGE_SIZE - 1;
static const uint64_t PAGE_MASK = ~uint64_t(PAGE_OFFSET);

void PMGD::os::flush(void *addr, RangeSet &pending_commits)
{
    uint64_t aligned_addr = (uint64_t)addr & PAGE_MASK;
    pending_commits.add(aligned_addr, aligned_addr + PAGE_SIZE);
}

void PMGD::os::commit(RangeSet &pending_commits)
{
    for (auto i = pending_commits.begin(); i != pending_commits.end(); i++)
        msync((void *)i->start, i->end - i->start, MS_SYNC);
    pending_commits.clear();
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
PMGD::os::SigHandler::SigHandler()
{
    struct sigaction sa;
    sa.sa_handler = sigbus_handler;
    sa.sa_flags = 0;
    sigfillset(&sa.sa_mask);
    sigaction(SIGBUS, &sa, NULL);
}

void PMGD::os::SigHandler::sigbus_handler(int)
{
    throw PMGDException(OutOfSpace);
}

size_t PMGD::os::get_default_region_size() { return SIZE_1TB; }

size_t PMGD::os::get_alignment(size_t size)
{
    if (size >= SIZE_1GB)
        return SIZE_1GB;
    else if (size >= SIZE_2MB)
        return SIZE_2MB;
    else
        return SIZE_4KB;
}
