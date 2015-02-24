#pragma once

#include <stddef.h>
#include <assert.h>
#include <time.h>
#include <string>
#include <string.h>    // For memset
#include "exception.h"
#include "stringid.h"

namespace Jarvis {
    enum PropertyType { t_novalue = 1, t_boolean, t_integer, t_string,
                        t_float, t_time, t_blob };

#pragma pack(push, 1)
    struct Time {
        // Always store time here in UTC for direct comparison.
        union {
            int64_t time_val;

            struct {
                uint64_t unused : 4;
                uint64_t usec : 20;  // 0-999,999
                uint64_t sec : 6;    // 0-60
                uint64_t min : 6;    // 0-59
                uint64_t hour : 5;   // 0-23
                uint64_t day : 5;    // 1-31
                uint64_t mon : 4;    // 1-12
                int64_t year : 14;   // -8192-8191
                // Store timezone for user queries to be answered in user's
                // given timezone.
                uint8_t tz_min : 2;
                int8_t tz_hour : 5;  // -12-14
                uint8_t tz_spare : 1;
            };
        };

        void fill_tm_utc(struct tm *) const;

    public:
        Time() { memset(this, 0, sizeof(Time)); }
        Time(const struct tm *, int hr_offset, int min_offset);

        // Get time in the user's timezone
        void get_tm(struct tm *) const;
        void get_utc(struct tm *) const;

        bool operator<(const Time &t) const
            { return time_val < t.time_val; }
        bool operator==(const Time &t) const
            { return time_val == t.time_val; }
    };
#pragma pack(pop)

    static_assert(sizeof (Time) == 9, "size of Time structure is wrong");

    class Property {
    public:
        struct blob_t {
            const void *value;
            std::size_t size;
            blob_t(const void *v, std::size_t s) : value(v), size(s) { }
        };

    private:
        PropertyType _type;

#if !defined(_MSC_VER)
        union {
            bool v_boolean;
            long long v_integer;
            std::string _v_string;
            double v_float;
            Time _v_time;
            blob_t _v_blob;
        };
        std::string &v_string() { return _v_string; }
        const std::string &v_string() const { return _v_string; }
        Time &v_time() { return _v_time; }
        const Time &v_time() const { return _v_time; }
        blob_t &v_blob() { return _v_blob; }
        const blob_t &v_blob() const { return _v_blob; }
#else
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX3(a, b, c) MAX(a, MAX(b, c))
#define UNION_SIZE MAX3(sizeof (std::string), sizeof (Time), sizeof (blob_t))
        union {
            bool v_boolean;
            long long v_integer;
            double v_float;
            uint8_t _u[UNION_SIZE];
        };
        std::string &v_string() { return *(std::string *)_u; }
        const std::string &v_string() const { return *(std::string *)_u; }
        Time &v_time() { return *(Time *)_u; }
        Time &v_time() const { return *(Time *)_u; }
        blob_t &v_blob() { return *(blob_t *)_u; }
        blob_t &v_blob() const { return *(blob_t *)_u; }
#undef UNION_SIZE
#undef MAX3
#undef MAX
#endif

        void check(int t) const { if (_type != t) throw Exception(property_type); }

    public:
        Property() : _type(t_novalue) { }
        Property(const Property &);
        Property(bool v) : _type(t_boolean), v_boolean(v) { }
        Property(int v) : _type(t_integer), v_integer(v) { }
        Property(long long v) : _type(t_integer), v_integer(v) { }
        Property(unsigned long long v) : _type(t_integer), v_integer(v) { }
        Property(double v) : _type(t_float), v_float(v) { }

#if !defined(_MSC_VER)
        Property(const char *s) : _type(t_string), _v_string(s) { }
        Property(const char *s, std::size_t len)
            : _type(t_string), _v_string(s, len) { }
        Property(const std::string str)
            : _type(t_string), _v_string(str) { }
#else
        Property(const char *s)
            : _type(t_string)
            { new (_u) std::string(s); }

        Property(const char *s, std::size_t len)
            : _type(t_string)
            { new (_u) std::string(s, len);  }

        Property(const std::string str)
            : _type(t_string)
            { new (_u) std::string(str);  }
#endif

        Property(const Time &v) : _type(t_time) { v_time() = v; }

        Property(const blob_t &blob) : _type(t_blob) { v_blob() = blob; }

        Property(const void *blob, std::size_t size)
            : _type(t_blob)
            { v_blob() = blob_t(blob, size);}

        ~Property();

        void operator=(const Property &);

        bool operator<(const Property &) const;

        PropertyType type() const { return _type; }
        bool bool_value() const { check(t_boolean); return v_boolean; }
        long long int_value() const { check(t_integer); return v_integer; }
        const std::string &string_value() const { check(t_string); return v_string(); }
        double float_value() const { check(t_float); return v_float; }
        Time time_value() const { check(t_time); return v_time(); }
        blob_t blob_value() const { check(t_blob); return v_blob(); }
    };

    struct PropertyPredicate {
        StringID id;
        enum op_t { dont_care,
                    eq, ne, gt, ge, lt, le,
                    gele, gelt, gtle, gtlt } op;
        Property v1, v2;
        PropertyPredicate() : id(0) { }
        PropertyPredicate(StringID i) : id(i), op(dont_care) { }
        PropertyPredicate(StringID i, op_t o, const Property &v)
            : id(i), op(o), v1(v) { assert(o > dont_care && o <= le); }
        PropertyPredicate(StringID i, op_t o,
                const Property &val1, const Property &val2)
            : id(i), op(o), v1(val1), v2(val2)
            { assert(o >= gele); }
    };

    inline bool operator==(const Property &a, const Property &b)
        { return (!(a < b) && !(b < a)); }
    inline bool operator!=(const Property &a, const Property &b)
        { return !(a == b); }
    inline bool operator>(const Property &a, const Property &b)
        { return (b < a); }
    inline bool operator<=(const Property &a, const Property &b)
        { return !(a > b); }
    inline bool operator>=(const Property &a, const Property &b)
        { return !(a < b); }
};
