#pragma once

#include "rwbase.h"

namespace amemory::detail
{

    class p_readwrite : public rwbase
    {
        int fd_ = -1;
    public:
        explicit p_readwrite(pid_t pid);
        p_readwrite(const p_readwrite&);
        p_readwrite(p_readwrite&&) noexcept;

        p_readwrite& operator=(const p_readwrite&);
        p_readwrite& operator=(p_readwrite&&) noexcept;

        void read(rwdata& datas) noexcept override;
        void write(rwdata& datas) noexcept override;

        ~p_readwrite() override;

    private:
        void destroy() noexcept;
    };
}