/**
 * Timing routines for benchmarking.
 */

#pragma once

#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>

/**
 * Report time and usage information
 * @param o  the put-to stream
 * @param t1 time sample at the beginning
 * @param t2 time sample at the end
 * @param u1 usage sample at the beginning
 * @param u2 usage sample at the end
 */
void print_delta(std::ostream &o, struct timeval &t1, struct timeval &t2, struct rusage &u1, struct rusage &u2)
{
    long long time1 = t1.tv_sec * 1000000 + t1.tv_usec;

    long long time2 = t2.tv_sec * 1000000 + t2.tv_usec;

    o << "Elapsed time is " << time2 - time1 << " microseconds\n";

    long long utime1 = u1.ru_utime.tv_sec * 1000000 + u1.ru_utime.tv_usec;
    long long stime1 = u1.ru_stime.tv_sec * 1000000 + u1.ru_stime.tv_usec;

    long long utime2 = u2.ru_utime.tv_sec * 1000000 + u2.ru_utime.tv_usec;
    long long stime2 = u2.ru_stime.tv_sec * 1000000 + u2.ru_stime.tv_usec;

    o << "User time is " << utime2 - utime1 << " microseconds (" << 100.0*double(utime2 - utime1)/(time2 - time1) << "%)\n";
    o << "System time is " << stime2 - stime1 << " microseconds (" << 100.0*double(stime2 - stime1)/(time2 - time1) << "%)\n";
}

#define START_TIMING                                     \
        struct timeval t1, t2;                           \
        struct rusage u1, u2;                            \
        getrusage(RUSAGE_SELF, &u1);                     \
        gettimeofday(&t1, NULL);                         \


#define END_TIMING                                       \
        gettimeofday(&t2, NULL);                         \
        getrusage(RUSAGE_SELF, &u2);                     \
        if (timing) print_delta(std::cout, t1, t2, u1, u2);   \

