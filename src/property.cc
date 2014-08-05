#include "property.h"
#include "iterator.h"

Jarvis::Property::Property(const Property &a)
    : _type(a._type)
{
    switch (a._type) {
        case t_novalue: break;
        case t_boolean: v_boolean = a.v_boolean; break;
        case t_integer: v_integer = a.v_integer; break;
        case t_string: new(&v_string) std::string(a.v_string); break;
        case t_float: v_float = a.v_float; break;
        case t_time: v_time = a.v_time; break;
        case t_blob: v_blob = a.v_blob; break;
        default: assert(0);
    }
}

Jarvis::Property::~Property()
{
    if (_type == t_string) {
        v_string.std::string::~string();
    }
}

void Jarvis::Property::operator=(const Property &a)
{
    if (_type == t_string)
        v_string.std::string::~string();
    _type = a._type;
    switch (a._type) {
        case t_novalue: break;
        case t_boolean: v_boolean = a.v_boolean; break;
        case t_integer: v_integer = a.v_integer; break;
        case t_string: new(&v_string) std::string(a.v_string); break;
        case t_float: v_float = a.v_float; break;
        case t_time: v_time = a.v_time; break;
        case t_blob: v_blob = a.v_blob; break;
        default: assert(0);
    }
}

bool Jarvis::Property::operator<(const Property &a) const
{
    check(a._type);

    switch (_type) {
        case t_novalue: return false; // no ordering
        case t_boolean: return v_boolean < a.v_boolean;
        case t_integer: return v_integer < a.v_integer;
        case t_string: return v_string < a.v_string; // collation order!
        case t_float: return v_float < a.v_float;
        case t_time: return v_time < a.v_time;
        case t_blob: return false; // no ordering
        default: assert(0); return false;
    }
}

Jarvis::Time::Time(const struct tm *tm, int hr_offset, int min_offset)
    : time_val(0)
{
    struct tm utc_tm = *tm;
    utc_tm.tm_hour -= hr_offset;
    utc_tm.tm_min -= min_offset * (hr_offset >= 0 ? 1 : -1);
    utc_tm.tm_isdst = -1;
    mktime(&utc_tm);

    // Fill in our time fields with the UTC version.
    sec = utc_tm.tm_sec;
    min = utc_tm.tm_min;
    hour = utc_tm.tm_hour;
    day = utc_tm.tm_mday;
    mon = utc_tm.tm_mon + 1;  // tm month starts from 0
    year = utc_tm.tm_year + 1900; // tm stores year - 1900
    tz_min = min_offset / 15;
    tz_hour = hr_offset;
    tz_spare = 0;
}

void Jarvis::Time::fill_tm_utc(struct tm *tm) const
{
    memset(tm, 0, sizeof(struct tm));
    tm->tm_sec = sec;
    tm->tm_min = min;
    tm->tm_hour = hour;
    tm->tm_mday = day;
    tm->tm_mon = mon - 1;
    tm->tm_year = year - 1900;
}

void Jarvis::Time::get_utc(struct tm *tm) const
{
    if (tm == NULL)
        return;
    fill_tm_utc(tm);
    tm->tm_isdst = -1;
    mktime(tm);  // Fills wday
}

void Jarvis::Time::get_tm(struct tm *tm) const
{
    if (tm == NULL)
        return;
    fill_tm_utc(tm);
    tm->tm_hour += tz_hour;
    tm->tm_min += tz_min * (tz_hour >= 0 ? 1 : -1) * 15;
    tm->tm_isdst = -1;
    mktime(tm);  // Fills wday
}
