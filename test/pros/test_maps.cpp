//
// Created by 30980 on 2025/12/19.
//

#include "timer.hpp"

#include "amemory/maps.h"

namespace memory_test
{
    void test_maps_parse(pid_t pid)
    {
        timer t("maps parse");
        amemory::maps maps(pid);
        maps.load();
        for (auto m : amemory::to_vector(maps))
        {
            auto mt = amemory::to_memory_type(m);
            if (mt == amemory::memory_t::ANONYMOUS)
                std::cout << std::hex << "Address: " << m.start() << std::dec << " Type: " << amemory::to_string(mt) << std::endl;
        }
    }
}
