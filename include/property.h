/**
 * @file   property.h
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

#pragma once

#include <stddef.h>
#include <assert.h>
#include <time.h>
#include <string>
#include <string.h>    // For memset
#include "exception.h"
#include "stringid.h"

namespace PMGD {
    enum class PropertyType { NoValue = 1, Boolean, Integer, String,
                              Float, Time, Blob };

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
        Time(const struct tm *, unsigned long usec, int hr_offset, int min_offset);
        Time(const struct tm *time, int hr_offset, int min_offset)
            : Time(time, 0, hr_offset, min_offset)
            { }

        // Get time in the user's timezone
        void get_tm(struct tm *) const;
        void get_utc(struct tm *) const;

        // Return time since 1970
        time_t get_time() const;
        uint64_t get_time_in_usec() const;
        uint64_t get_time_in_msec() const
            { return get_time_in_usec() / 1000; }

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

        void check(PropertyType t) const { if (_type != t) throw PMGDException(PropertyTypeMismatch); }

    public:
        Property() : _type(PropertyType::NoValue) { }
        Property(const Property &);
        Property(bool v) : _type(PropertyType::Boolean), v_boolean(v) { }
        Property(int v) : _type(PropertyType::Integer), v_integer(v) { }
        Property(long long v) : _type(PropertyType::Integer), v_integer(v) { }
        Property(unsigned long long v) : _type(PropertyType::Integer), v_integer(v) { }
        Property(double v) : _type(PropertyType::Float), v_float(v) { }

#if !defined(_MSC_VER)
        Property(const char *s) : _type(PropertyType::String), _v_string(s) { }
        Property(const char *s, std::size_t len)
            : _type(PropertyType::String), _v_string(s, len) { }
        Property(const std::string str)
            : _type(PropertyType::String), _v_string(str) { }
#else
        Property(const char *s)
            : _type(PropertyType::String)
            { new (_u) std::string(s); }

        Property(const char *s, std::size_t len)
            : _type(PropertyType::String)
            { new (_u) std::string(s, len);  }

        Property(const std::string str)
            : _type(PropertyType::String)
            { new (_u) std::string(str);  }
#endif

        Property(const Time &v) : _type(PropertyType::Time) { v_time() = v; }

        Property(const blob_t &blob) : _type(PropertyType::Blob) { v_blob() = blob; }

        Property(const void *blob, std::size_t size)
            : _type(PropertyType::Blob)
            { v_blob() = blob_t(blob, size);}

        ~Property();

        void operator=(const Property &);

        bool operator<(const Property &) const;

        PropertyType type() const { return _type; }
        bool bool_value() const { check(PropertyType::Boolean); return v_boolean; }
        long long int_value() const { check(PropertyType::Integer); return v_integer; }
        const std::string &string_value() const { check(PropertyType::String); return v_string(); }
        double float_value() const { check(PropertyType::Float); return v_float; }
        Time time_value() const { check(PropertyType::Time); return v_time(); }
        blob_t blob_value() const { check(PropertyType::Blob); return v_blob(); }
    };

    struct PropertyPredicate {
        StringID id;
        enum Op { DontCare, Eq, Ne, Gt, Ge, Lt, Le, GeLe, GeLt, GtLe, GtLt } op;
        Property v1, v2;
        PropertyPredicate() : id(0) { }
        PropertyPredicate(StringID i) : id(i), op(DontCare) { }
        PropertyPredicate(StringID i, Op o, const Property &v)
            : id(i), op(o), v1(v) { assert(o > DontCare && o <= Le); }
        PropertyPredicate(StringID i, Op o,
                const Property &val1, const Property &val2)
            : id(i), op(o), v1(val1), v2(val2)
            { assert(o >= GeLe); }
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
