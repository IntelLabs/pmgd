#pragma once

namespace Jarvis {
    enum Exception {
        e_undefined_exception,
        e_bad_alloc,
        e_null_iterator,

        e_map_failed,
        e_property_type,

        e_not_implemented = 100,
        e_internal_error,
    };
};
