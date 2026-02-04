#include <iostream>
#include <unistd.h>


#include "amemory/package.h"
#include "amemory/detail/search/memory_splitter.hpp"
#include "pros/timer.hpp"


namespace memory_test
{
    void test_maps_parse(pid_t);
    void test_search(pid_t);
    void test_status_parse(pid_t);
    void test_read(pid_t);
}

int main(int argn, char* argv[])
{
    std::cout << "欢迎使用amemory库" << std::endl;

    pid_t pid = amemory::get_pid_by_name("bin.mt.plus");
    std::cout << "Target Pid: " << pid << std::endl;
    std::cout << "Self Pid: " << getpid() << std::endl;
    memory_test::timer total_timer("Total Execution");
    // memory_test::test_maps_parse(pid);
    // memory_test::test_search(pid);
    // memory_test::test_status_parse(amemory::self);
    // memory_test::test_read(pid);
    return 0;
}