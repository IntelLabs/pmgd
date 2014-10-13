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
