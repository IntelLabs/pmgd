#pragma once

#if _MSC_VER
    #define THREAD __declspec(thread)
#else
    #define THREAD thread_local
#endif

#define EXPECT_FALSE(X) __builtin_expect((X), false)
#define EXPECT_TRUE(X)  __builtin_expect((X), true)
