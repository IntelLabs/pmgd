#include <stdio.h>
#include <string.h>     // For memset
#include "jarvis.h"
#include "util.h"

#ifdef _MSC_VER
extern "C" char *strptime(const char *buf, const char *format, struct tm *tm);
#else
#include <time.h>
#endif

using namespace Jarvis;

std::string property_text(const Property &p)
{
    switch (p.type()) {
        case PropertyType::NoValue: return "no value";
        case PropertyType::Boolean: return p.bool_value() ? "T" : "F";
        case PropertyType::Integer: return std::to_string(p.int_value());
        case PropertyType::String: return p.string_value();
        case PropertyType::Float: return std::to_string(p.float_value());
        case PropertyType::Time: return time_to_string(p.time_value());
        case PropertyType::Blob: return "<blob value>";
        default: throw Exception(PropertyTypeInvalid);
    }
}

std::string property_text(const PropertyRef &p)
{
    switch (p.type()) {
        case PropertyType::NoValue: return "no value";
        case PropertyType::Boolean: return p.bool_value() ? "T" : "F";
        case PropertyType::Integer: return std::to_string(p.int_value());
        case PropertyType::String: return p.string_value();
        case PropertyType::Float: return std::to_string(p.float_value());
        case PropertyType::Time: return time_to_string(p.time_value());
        case PropertyType::Blob: return "<blob value>";
        default: throw Exception(PropertyTypeInvalid);
    }
}

template <typename T>
std::string tag_text(const T &n)
{
    std::string tag = n.get_tag().name();
    if (tag != "")
        tag = " #" + tag;
    return tag;
}

template std::string tag_text<Node>(const Node &);
template std::string tag_text<Edge>(const Edge &);
template std::string tag_text<EdgeRef>(const EdgeRef &);

// Handle EDT/EST, PDT/PST, MDT/MST and +HHMM formats only.
// Others default to UTC time.
static void get_timezone(const char *str, int tz_len, int *hr_offset, int *min_offset)
{
    if (str[0] == '+' || str[0] == '-') { // +HHMM or -HHMM
        if (tz_len != 5) {
            *hr_offset = *min_offset = 0;
            return;
        }
        int dir = (str[0] == '+') ? 1 : -1;
        str++;
        // The user could still pass random characters in HHMM.
        // So use atoi() to correct for that.
        char num[3] = {0};
        num[0] = *str; str++;
        num[1] = *str; str++;
        *hr_offset = dir * atoi(num);
        num[0] = *str; str++;
        num[1] = *str; str++;
        *min_offset = atoi(num);
        return;
    }
    *min_offset = 0;
    *hr_offset = 0;
    // This if handles PST, PDT, EST, EDT, MST, MDT, CST, CDT
    // UT and GMT will get handled as part of not matching any of these
    // cases. Any other value also gets a default UTC value.
    if (tz_len == 3 && toupper(str[2]) == 'T') {
        char mid = toupper(str[1]);
        if (mid == 'D')
            *hr_offset = 1;
        else if (mid != 'S')
            return;
        switch (toupper(str[0])) {
            case 'E':
                *hr_offset += -5;
                break;
            case 'C':
                *hr_offset += -6;
                break;
            case 'M':
                *hr_offset += -7;
                break;
            case 'P':
                *hr_offset += -8;
                break;
            default:
                *hr_offset = 0;
                return;
        }
    }
    return;
}

// Conversion function from date time string to Jarvis time format.
// TODO: Distinguish between the various time format options
bool string_to_tm(const std::string &tstr, struct tm *user_tz_tm,
                     int *hr_offset, int *min_offset)
{
    // Parse the user time
    // strptime does not fill in all fields.
    memset(user_tz_tm, 0, sizeof(struct tm));

    // The function does not parse timezones. So add a different
    // parsing step to extract year that follows the timezone string.
    const char *left_over = strptime(tstr.c_str(), "%a %b %d %X ", user_tz_tm);
    if (left_over == NULL) // Incomplete format.
        return false;

    // Make sure the parsed value is actually a time value
    if (user_tz_tm->tm_mday == 0) // day of the month can never be 0 in a valid tm
        return false;

    // Timezone abbreviations that we support can be 2-5 chars long.
    int tz_len = 0;
    while (left_over[tz_len] != '\0' && !isblank(left_over[tz_len]))
        tz_len++;
    if (tz_len == 0 || left_over[tz_len] == '\0') // Missing timezone + year
        return false;
    get_timezone(left_over, tz_len, hr_offset, min_offset);

    if (strptime(left_over + tz_len, "%Y", user_tz_tm) == NULL) // Missing year
        return false;

    return true;
}

std::string time_to_string(const Time &t, bool utc)
{
    struct tm tm;

    if (utc)
        t.get_utc(&tm);
    else
        t.get_tm(&tm);
    // Return a ISO 8601 standard representation of time in user's
    // timezone (one that was used to create t) if utc is false.
    // Pick some string length since the length of the time value
    // depends on the locale.
    char str[80];

#if defined _MSC_VER
    // The strftime used here is from windows dll and doesn't support all
    // the same format specifiers as Linux.
    size_t num_bytes = strftime(str, 80, "%Y-%m-%dT%H:%M:%S", &tm);
#else
    size_t num_bytes = strftime(str, 80, "%FT%T", &tm);
#endif

    // Check for remaining bytes to make sure there is room for the
    // timezone portion.
    assert(num_bytes > 0 && num_bytes < 70);
    // Add the timezone now.
    if (utc)
        sprintf(str + num_bytes, "Z");
    else
        sprintf(str + num_bytes, "%+.2d:%.2d", t.tz_hour, t.tz_min * 15);
    return str;
}
