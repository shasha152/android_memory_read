//
// Created by 30980 on 2026/1/4.
//

#include <vector>

#include "timer.hpp"

#include "amemory/memory.h"

namespace am = amemory;

namespace memory_test
{
    struct data4
    {
        am::offset_t<std::int64_t, 0x8> v1;
        am::offset_t<std::int64_t, 0x10> v2;
    };
    struct data3
    {
        am::pointer_t<data4, 0x0> v1;
        am::offset_int32_t<0x10> v2;
    };


    struct data2
    {
        float v1;
        am::pointer_t<data3, 0x8> v2;
    };

    struct data1
    {
        am::pointer_t<float, 0x8> v1;
        am::pointer_t<data2, 0x18> v2;
        float v3;
    };

    void test_read(pid_t pid)
    {
        data1 data;
        am::default_access access(pid);
        am::manager<am::default_access> manager(access);

        manager.read(0x7B2B823400, data);
        std::cout << *data.v1 << std::endl;
        std::cout << data.v2->v2->v1->v2 << std::endl;
    }
}