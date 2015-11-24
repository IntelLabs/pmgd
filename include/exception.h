#pragma once

#include <string>

namespace Jarvis {
    enum ExceptionType {
        UndefinedException,

        BadAlloc,
        NullIterator,
        VacantIterator,

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

        InternalErrorBase = 100,
        NotImplemented,
        PropertyTypeInvalid,

        UtilExceptionBase = 200,
        AppExceptionBase = 300,
    };

    struct Exception {
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
    Exception(Jarvis::name, #name, ##__VA_ARGS__, __FILE__, __LINE__)
};
