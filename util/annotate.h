/**
 * @file   annotate.h
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

#include <stdint.h>
#include <string>

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
