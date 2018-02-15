/**
 * @file   windows.cc
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
#include <signal.h>
#include <windows.h>

#include "os.h"
#include "exception.h"

class PMGD::os::MapRegion::OSMapRegion {
    HANDLE _file_handle;
    HANDLE _map_handle;
    uint64_t _map_addr;

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
    std::string tmp = db_name;
    if (tmp.length() == 1)
        tmp = "./" + tmp;
    std::string filename = tmp + ":" + region_name;

    if ((_file_handle = CreateFile(filename.c_str(),
                                   GENERIC_READ | !read_only * GENERIC_WRITE,
                                   0,
                                   NULL,
                                   !create * !truncate * OPEN_EXISTING
                                   | !create * truncate * TRUNCATE_EXISTING
                                   | create * !truncate * OPEN_ALWAYS
                                   | create * truncate * CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL)) == INVALID_HANDLE_VALUE)
        throw PMGDException(OpenFailed, GetLastError(), filename + " (CreateFile)");

    // check size before mapping
    LARGE_INTEGER size;
    if (!GetFileSizeEx(_file_handle, &size)) {
        int err = GetLastError();
        CloseHandle(_file_handle);
        throw PMGDException(OpenFailed, err, filename + " (GetFileSize)");
    }

    if ((uint64_t)size.QuadPart == map_len) {
        create = false;
    }
    else if (size.QuadPart == 0 && create) {
        DWORD dummy;
        if (DeviceIoControl(_file_handle, FSCTL_SET_SPARSE, NULL, 0,
                            NULL, 0, &dummy, NULL) == 0) {
            int err = GetLastError();
            CloseHandle(_file_handle);
            throw PMGDException(OpenFailed, err, filename + " (DeviceIOControl)");
        }
    }
    else {
        CloseHandle(_file_handle);
        throw PMGDException(OpenFailed, filename + " was not the expected size");
    }

    if ((_map_handle = CreateFileMapping(_file_handle, NULL,
            read_only ? PAGE_READONLY : PAGE_READWRITE,
            map_len >> 32, map_len, NULL)) == NULL)
    {
        int err = GetLastError();
        CloseHandle(_file_handle);
        throw PMGDException(OpenFailed, err, filename + " (CreateFileMapping)");
    }

    if (MapViewOfFileEx(_map_handle,
            read_only ? FILE_MAP_READ : FILE_MAP_WRITE,
            0, 0, map_len, (void *)map_addr) != (void *)map_addr)
    {
        int err = GetLastError();
        CloseHandle(_map_handle);
        CloseHandle(_file_handle);
        throw PMGDException(OpenFailed, err, filename + " (MapViewOfFileEx)");
    }

    _map_addr = map_addr;
}


PMGD::os::MapRegion::OSMapRegion::~OSMapRegion()
{
    UnmapViewOfFile((void *)_map_addr);
    CloseHandle(_map_handle);
    CloseHandle(_file_handle);
}

#ifdef SEH
#include <eh.h>
// Windows throws the SEH (Structured Excepting Handling) exception
// EXCEPTION_IN_PAGE_ERROR when an attempted access to a memory-mapped
// file cannot be satisfied because there is no space left on the device.
// To properly handle this exception, the PMGD library and programs
// that use it must be compiled with /EHa.
// There are other reasons that EXCEPTION_IN_PAGE_ERROR could occur.
// This handler will mask such errors and make them appear to be an
// out-of-space condition. It might be possible to distinguish by examining
// the faulting address.
// The SigHandler constructor must be called on each thread in the program
// that calls PMGD functions. The PMGD library does this on the thread
// that opens the Graph. The application is responsible for constructing
// an instance of SigHandler on any additional threads that it creates.
namespace PMGD {
    namespace os {
        static void seh_translation(unsigned e, _EXCEPTION_POINTERS *exp)
        {
            if (e == EXCEPTION_IN_PAGE_ERROR)
                throw PMGDException(OutOfSpace);
        }
    }
};
#endif

PMGD::os::SigHandler::SigHandler()
{
   // _set_se_translator(seh_translation);
}

void PMGD::os::SigHandler::sigbus_handler(int)
{
}

size_t PMGD::os::get_default_region_size() { return SIZE_1GB; }

size_t PMGD::os::get_alignment(size_t size)
{
    SYSTEM_INFO sys_info;

    GetSystemInfo(&sys_info);
    return sys_info.dwAllocationGranularity;
}
