#pragma once

#include <string>

namespace Jarvis {
    struct Exception {
        enum {
            e_undefined_exception,
            e_bad_alloc,
            e_null_iterator,

            e_open_failed,
            e_read_only,
            e_out_of_space,

            e_property_type,
            e_property_not_found,

            e_no_transaction,
            e_tx_recovery_failed,
            e_tx_alloc_failed,
            e_tx_small_journal,
            e_tx_aborted,

            e_invalid_id,

            e_not_implemented = 100,
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
    Exception(Exception::e_##name, #name, ##__VA_ARGS__, __FILE__, __LINE__)
};
