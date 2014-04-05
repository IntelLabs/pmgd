#pragma once

#include <stdint.h>

// Issue the CPUID instruction with special parameters, for use with Hypersim.
static inline int cpuid(uint32_t a, uint32_t b = 0, uint32_t c = 0,
                        uint32_t d = 0, uint32_t S = 0, uint32_t D = 0)
{
    int status;

    asm volatile("cpuid"
        : "=a" (status), "+b" (b), "+c" (c), "+d" (d)
        : "0" (a), "S" (S), "D" (D));

    return status;
}

// Add an annotation to the Hypersim log.
static inline void annotate(const std::string &s)
{
#ifdef HSPM
    assert((uint64_t)s.data() < 0x100000000ULL);
    cpuid(0x56583263, 8, (uint32_t)(uint64_t)s.data(), s.length());
#endif
}
