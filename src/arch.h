#pragma once

template <typename T>
static inline bool cmpxchg(volatile T &m, T old_val, T new_val)
{
    bool result = 0;
    asm volatile ("lock cmpxchg %3, %1; sete %0"
                  : "=q"(result), "+m"(m), "+a"(old_val)
                  : "r"(new_val)
                  : "memory", "cc");
    return result;
}

template <typename T>
static inline bool bts(volatile T &m, int bit)
{
    bool result = 0;
    asm volatile ("lock bts%z1 %2, %1\n; setc %0"
            : "=q"(result), "+m"(m) : "Ir"(T(bit)) : "memory", "cc");
    return result;
}

template <typename T>
static inline unsigned bsr(T value)
{
    T r;
    // Find the index of the highest bit = 1
    asm("bsr %1,%0" : "=r"(r) : "r"(value));
    return unsigned(r);
}

template <typename T>
static inline T atomic_inc(volatile T &m)
{
    T r = 1;
    asm volatile ("lock xadd %1, %0" : "+m"(m), "+r"(r) : : "memory", "cc");
    return r;
}

static inline void memory_barrier() // Instruct compiler not to re-order
{
    asm volatile ("" : : : "memory");
}

asm (
    ".macro clflushopt mem\n\t"
    ".byte 0x66\n\t"
    "clflush \\mem\n\t"
    ".endm\n\t"
);

static inline void sfence()
{
#ifndef NOPM
    asm("sfence" : : : "memory");
#endif
}

static inline void clflush(void *addr)
{
#ifndef NOPM
    asm("clflushopt \"%0\"" : "+m"(*(char *)addr));
#endif
}

static inline void pcommit()
{
#ifndef NOPM
    asm("pcommit" : : : "memory");
#endif
}
