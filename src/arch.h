/**
 * @file   arch.h
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

template <typename T>
static inline bool cmpxchg(volatile T &m, T old_val, T new_val)
{
    bool result = 0;
    __asm__ volatile ("lock cmpxchg %3, %1; sete %0"
                  : "=q"(result), "+m"(m), "+a"(old_val)
                  : "r"(new_val)
                  : "memory", "cc");
    return result;
}

template <typename T>
static inline bool bts(volatile T &m, int bit)
{
    bool result = 0;
    __asm__ volatile ("lock bts%z1 %2, %1\n; setc %0"
            : "=q"(result), "+m"(m) : "Ir"(T(bit)) : "memory", "cc");
    return result;
}

template <typename T>
static inline void atomic_and(volatile T &m, T v)
  { asm volatile ("lock and%z0 %1, %0" : "+m"(m) : "ri"(v)); }

template <typename T>
static inline unsigned bsr(T value)
{
    T r;
    // Find the index of the highest bit = 1
    __asm__("bsr %1,%0" : "=r"(r) : "r"(value));
    return unsigned(r);
}

template <typename T>
static inline T atomic_inc(volatile T &m)
{
    T r = 1;
    __asm__ volatile ("lock xadd %1, %0" : "+m"(m), "+r"(r) : : "memory", "cc");
    return r;
}

static inline void memory_barrier() // Instruct compiler not to re-order
{
    __asm__ volatile ("" : : : "memory");
}

template <typename T>
static inline T xadd(volatile T &m, T v)
{
    T r = v;
    asm volatile ("lock xadd %1, %0" : "+m"(m), "+r"(r));
    return r;
}

static inline void pause()
{
    asm("pause");
}

#if defined(HSPM) || !defined(NOPM)
__asm__ (
    ".macro pcommit\n\t"
    ".byte 0x66, 0x0f, 0xae, 0xf8\n\t"
    ".endm\n\t"

    ".macro clflushopt mem\n\t"
    ".byte 0x66\n\t"
    "clflush \\mem\n\t"
    ".endm\n\t"

    ".macro mysfence param\n\t"
    ".byte 0x66\n\t"
    "lfence\n\t"
    "mov \\param, %al\n\t"
    ".endm\n\t"

    ".macro mypcommit param\n\t"
    ".byte 0x66\n\t"
    "sfence\n\t"
    "mov \\param, %al\n\t"
    ".endm\n\t"

    ".macro mymemset\n\t"
    ".byte 0x66\n\t"
    "mfence\n\t"
    ".endm\n\t"
);
#endif

static inline void clflush(void *addr)
{
#ifndef NOPM
    __asm__("clflushopt \"%0\"" : "+m"(*(char *)addr) : : "memory");
#endif
}

static inline void persistent_barrier(uint8_t param)
{
#if defined(HSPM)
    __asm__ volatile ("mysfence %0"  : : "N"(param));
    __asm__ volatile ("mypcommit %0" : : "N"(param+1));
    __asm__ volatile ("mysfence %0"  : : "N"(param+2));
#elif !defined(NOPM)
    __asm__ volatile ("sfence" : : : "memory");
    __asm__ volatile ("pcommit" : : : "memory");
    __asm__ volatile ("sfence" : : : "memory");
#endif
}
