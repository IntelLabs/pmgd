#pragma once

#include <string>

namespace Jarvis {
    struct Exception {
        enum {
            UndefinedException,
            BadAlloc,
            NullIterator,

            OpenFailed,
            VersionMismatch,
            ReadOnly,
            InvalidConfig,
            OutOfSpace,

            PropertyTypeMismatch,
            PropertyNotFound,

            NoTransaction,
            OutOfTransactions,
            OutOfJournalSpace,

            InvalidID,

            NotImplemented = 100,
            PropertyTypeInvalid,
        };

        // Which exception
        int num;            ///< Exception number
        const char *name;   ///< Exception name

        // Additional information
        std::string msg;
        int errno_val;

        // Where it was thrown
        const char *file;   ///< Source file name
        int line;           ///< Source line number

        Exception(int exc, const char *exc_name, const char *f, int l)
            : num(exc), name(exc_name),
              msg(), errno_val(0),
              file(f), line(l)
        {}

        Exception(int exc, const char *exc_name,
                  const std::string &m,
                  const char *f, int l)
            : num(exc), name(exc_name),
              msg(m), errno_val(0),
              file(f), line(l)
        {}

        Exception(int exc, const char *exc_name,
                  int err, const std::string &m,
                  const char *f, int l)
            : num(exc), name(exc_name),
              msg(m), errno_val(err),
              file(f), line(l)
        {}
    };

#define Exception(name, ...) \
    Exception(Exception::name, #name, ##__VA_ARGS__, __FILE__, __LINE__)
};
