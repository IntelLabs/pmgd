#pragma once

#if _MSC_VER
    #define THREAD __declspec(thread)
#else
    #define THREAD thread_local
#endif
