#pragma once

#include <chrono>
#include <iostream>
#include <string_view>

namespace memory_test
{
    class timer
    {
    public:
        explicit timer(std::string_view name)
            : name_(name), start_(std::chrono::high_resolution_clock::now())
        {
        }
        ~timer()
        {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
            std::cout << name_ << " took " << std::dec << duration << " ns" << std::endl;
        }
    private:
        std::string_view name_;
        std::chrono::high_resolution_clock::time_point start_;
    };

}
