#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <locale>

namespace Jarvis {
    // String representation for Index nodes
    // This version of the class will always create a PM copy.
    class IndexString {
        class TransientIndexString;

        void operator=(const TransientIndexString &istr);
    protected:
        static const unsigned PREFIX_LEN = 8;

        char _prefix[PREFIX_LEN];
        char *_remainder;
        uint32_t _len;

        IndexString() : _remainder(NULL) { }

    public:
        // Use this constructor to create a PM copy.
        IndexString(const IndexString &istr);

        // The tree code currently does an assignment at delete time.
        // Ideally long strings should be reference counted. But this solution
        // destroys anything at the original and creates a full copy.
        // TODO: Change to reference counting of strings based on overhead
        // analysis or modify tree code to move pointers instead of content.
        void operator=(const IndexString &istr);

        ~IndexString();

        // Function returns 0 for equal, <0 if *this < istr and >0 if *this > istr.
        int compare(const IndexString &) const;

        bool operator==(const IndexString &istr) const
            { return (compare(istr) == 0); }

        bool operator<(const IndexString &istr) const
            { return (compare(istr) < 0); }

        bool operator>(const IndexString &istr) const
            { return (compare(istr) > 0); }

        size_t get_remainder_size()
            { return (_len > PREFIX_LEN)? _len - PREFIX_LEN : 0; }
    };

    // We need to create this class to know which version of _remainder
    // lives in PM vs. DRAM so that we do not try to free something that
    // doesnt belong to our allocators.
    // **This version should be used for all non PM instances of IndexString.
    class TransientIndexString : public IndexString {
        TransientIndexString();

        // Avoid accidental assignments and copies.
        TransientIndexString(const TransientIndexString&);
        TransientIndexString& operator=(const TransientIndexString&);
    public:
        // Doesn't create a copy in PM. But the locale based transformation
        // happens here and gets copied at a DRAM location.
        TransientIndexString(const std::string &str, const std::locale &loc);

        ~TransientIndexString();
    };
}
