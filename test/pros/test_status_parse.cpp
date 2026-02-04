//
// Created by 30980 on 2026/1/3.
//

#include "timer.hpp"
#include "amemory/package.h"

namespace memory_test
{
    void test_status_parse(pid_t pid)
    {
        timer timer("status parse");

        auto apps = amemory::get_all_app_info();
        for (auto& app : apps)
        {
            std::cout << app[amemory::status_keys::name][0].as_string() << std::endl;
        }
    }
}
