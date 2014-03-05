#pragma once

namespace Jarvis {
    struct Exception {
        enum {
            e_undefined_exception,
            e_bad_alloc,
            e_null_iterator,

            e_map_failed,
            e_property_type,
            e_property_not_found,

            e_tx_recovery_failed,
            e_tx_alloc_failed,
            e_tx_small_journal,
            e_tx_aborted,

            e_not_implemented = 100,
            e_internal_error,
        };

        // Which exception
        int num;            ///< Exception number
        const char *name;   ///< Exception name

        // Where it was thrown
        const char *file;   ///< Filename
        int line;           ///< Line number

        Exception(int e, const char *n, const char *f, int l)
            : num(e), name(n), file(f), line(l) {}
    };

#define Exception(name) \
    Exception(Exception::e_##name, #name, __FILE__, __LINE__)
};
