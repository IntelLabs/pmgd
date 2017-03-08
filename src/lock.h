/**
 * @file   lock.h
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
#include <thread>
#include <random>
#include <vector>
#include <algorithm>

#include "arch.h"

// All locking structures live in DRAM. We use stripe locks
// to balance out the space used by locks for a large database
// vs. parallelism.
namespace PMGD {
    class RWLock
    {
        static const uint16_t LOCK_READER_MASK = 0x7fff;
        static const uint16_t READER_INCR      = 1;
        static const uint16_t WRITER_LOCK_BIT  = 15;
        static const uint16_t WRITE_LOCK       = 1UL << WRITER_LOCK_BIT;

        // Backoff variables.
        // *** Tune experimentally
        static const size_t MIN_BACKOFF_DELAY = 100000;
        static const size_t MAX_BACKOFF_DELAY = 10000000;
        static const unsigned MAX_ATTEMPTS      = 10;

        uint16_t xadd(volatile uint16_t &m, uint16_t v)
            { return ::xadd<uint16_t>(m, v); }
        void atomic_and(volatile uint16_t &m, uint16_t v)
            { ::atomic_and<uint16_t>(m, v); }

        volatile uint16_t _rw_lock;

        // Ideas from here: https://geidav.wordpress.com/tag/exponential-back-off
        void backoff(size_t &cur_max_delay)
        {
            std::random_device rd;
            thread_local std::uniform_int_distribution<size_t> dist;

            // The simplest pseudo-random number generator that gives an int.
            thread_local std::minstd_rand gen(rd());

            size_t delay = cur_max_delay;
            size_t temp = 2 * cur_max_delay;
            cur_max_delay = (temp < MAX_BACKOFF_DELAY) ? temp : MAX_BACKOFF_DELAY;
            const size_t count = dist(gen, decltype(dist)::param_type{delay, cur_max_delay});
            for (size_t i = 0; i < count; ++i)
                pause();
        }

    public:
        RWLock() : _rw_lock(0) {}

        void read_lock()
        {
            size_t cur_max_delay = MIN_BACKOFF_DELAY;
            unsigned attempts = 0;

            while (1) {
                uint16_t r = xadd(_rw_lock, READER_INCR);
                assert((r & LOCK_READER_MASK) != LOCK_READER_MASK);

                // Check if we get lock w/o any active writers
                if ((r & WRITE_LOCK) == 0)
                    return;
                xadd(_rw_lock, -READER_INCR);

                // Wait for any active writers
                while (_rw_lock & WRITE_LOCK) {
                    if (++attempts > MAX_ATTEMPTS)
                        throw PMGDException(LockTimeout);
                    backoff(cur_max_delay);
                }
            }
        }

        void read_unlock()
        {
            assert((_rw_lock & LOCK_READER_MASK) != 0);
            xadd(_rw_lock, -READER_INCR);
        }

        void write_lock()
        {
            size_t cur_max_delay = MIN_BACKOFF_DELAY;
            unsigned attempts = 0;

            while (1) {
                // Check if we get lock w/o any active writers
                if (bts(_rw_lock, WRITER_LOCK_BIT) == 0) {
                    attempts = 0;

                    // Wait for any active readers
                    while(_rw_lock & LOCK_READER_MASK) {
                        if (++attempts > MAX_ATTEMPTS) {
                            atomic_and(_rw_lock, LOCK_READER_MASK);
                            throw PMGDException(LockTimeout);
                        }
                        backoff(cur_max_delay);
                    }
                    return;
                }

                // Wait for any active writers
                while (_rw_lock & WRITE_LOCK) {
                    if (++attempts > MAX_ATTEMPTS) {
                        throw PMGDException(LockTimeout);
                    }
                    backoff(cur_max_delay);
                }
            }
        }

        // This function should be called only when the caller already possesses
        // a read lock. Rather than making this recursive, we are relying on the
        // caller to use this correctly, hence this is not visible to applications.
        void upgrade_write_lock()
        {
            size_t cur_max_delay = MIN_BACKOFF_DELAY;
            unsigned attempts = 0;

            while (1) {
                // Check if we get lock w/o any active writers
                if (bts(_rw_lock, WRITER_LOCK_BIT) == 0) {
                    attempts = 0;

                    // Wait for any active readers
                    while ((_rw_lock & LOCK_READER_MASK) > 1) {
                        if (++attempts > MAX_ATTEMPTS) {
                            atomic_and(_rw_lock, LOCK_READER_MASK);
                            throw PMGDException(LockTimeout);
                        }
                        backoff(cur_max_delay);
                    }

                    // Don't need reader lock anymore
                    xadd(_rw_lock, -READER_INCR);
                    return;
                }

                // Wait for any active writers
                // Give this another extra attempt
                while (_rw_lock & WRITE_LOCK) {
                    if (attempts++ > MAX_ATTEMPTS) {
                        throw PMGDException(LockTimeout);
                    }
                    backoff(cur_max_delay);
                }
            }
        }

        void write_unlock()
        {
            assert((_rw_lock & WRITE_LOCK) != 0);
            atomic_and(_rw_lock, LOCK_READER_MASK);
        }

        bool is_write_locked() { return _rw_lock & WRITE_LOCK; }

        uint16_t reader_count() const { return _rw_lock & LOCK_READER_MASK; }
    };

    // This should be created one per large data structure.
    class StripedLock
    {
        std::vector<RWLock> _locks;

        // Mask to find index when given an address.
        const uint64_t _maskbits;

        // To spread the address across different locks, ignore
        // the lower few bits, as indicated this variable.
        // This is computed using the log2 of stripe_width provided
        // in the constructor. stripe_width gives how many bytes
        // of an object does the caller wish to cover with one lock.
        const uint64_t _shift;

        static unsigned floor_log2(unsigned long long n)
            { return n <= 1 ? 0 : floor_log2(n/2) + 1; }

    public:
        StripedLock() = delete;

        StripedLock(const size_t tot_bytes, const unsigned stripe_width)
            : _locks(tot_bytes / sizeof(RWLock)),
              _maskbits(_locks.size() - 1),
              _shift(ceiling_log2(stripe_width))
        {
            // For mask bits.
            assert(!(tot_bytes & (tot_bytes - 1)));
        }

        static unsigned ceiling_log2(unsigned long long n)
            { return (n & (n - 1)) == 0 ? floor_log2(n) : floor_log2(n - 1) + 1; }

        uint64_t read_lock(const void *addr)
        {
            uint64_t stripeid = get_stripe_id(addr);
            _locks[stripeid].read_lock();
            return stripeid;
        }

        void read_lock(const uint64_t stripeid)
          { _locks[stripeid].read_lock(); }

       void read_unlock(const uint64_t stripeid)
          { _locks[stripeid].read_unlock(); }

        uint64_t write_lock(const void *addr)
        {
            uint64_t stripeid = get_stripe_id(addr);
            _locks[stripeid].write_lock();
            return stripeid;
        }

        void write_lock(const uint64_t stripeid)
          { _locks[stripeid].write_lock(); }

        void upgrade_lock(const uint64_t stripeid)
          { _locks[stripeid].upgrade_write_lock(); }

        void write_unlock(const uint64_t stripeid)
          { _locks[stripeid].write_unlock(); }

        bool is_write_locked(const uint64_t stripeid)
          { return _locks[stripeid].is_write_locked(); }

        uint64_t get_stripe_id(const void *addr) const
          { return reinterpret_cast<uint64_t>(addr) >> _shift & _maskbits; }

        uint16_t reader_count(const uint64_t stripeid) const
          { return _locks[stripeid].reader_count(); }
    };
}
