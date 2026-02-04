#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include "amemory/package.h"

#include "amemory/detail/read_file.h"

namespace amemory
{


    pid_t get_pid_by_name(std::string_view p)
    {
        DIR* dir = opendir("/proc");
        if (!dir) return 0;

        char buffer[256 + 1];
        dirent* ent;

        while ((ent = readdir(dir)) != nullptr)
        {
            if (!std::isdigit(ent->d_name[0]))
                continue;

            char path[64];
            std::snprintf(path, sizeof(path), "/proc/%s/cmdline", ent->d_name);

            int fd = open(path, O_RDONLY | O_CLOEXEC);
            if (fd < 0)
                continue;

            ssize_t size = read(fd, buffer, sizeof(buffer) - 1);
            close(fd);

            if (size <= 0)
                continue;

            buffer[size] = '\0';
            if (p == buffer)
            {
                closedir(dir);
                return std::atoi(ent->d_name);
            }
        }

        closedir(dir);
        return 0;
    }

    std::vector<package> get_all_app_info()
    {
        std::vector<package> result;
        result.reserve(64);

        DIR* dir = opendir("/proc");
        if (!dir) return result;

        dirent* ent;

        while ((ent = readdir(dir)) != nullptr)
        {
            if (!std::isdigit(ent->d_name[0]))
                continue;

            char path[64];
            std::snprintf(path, sizeof(path), "/proc/%s", ent->d_name);

            package p(path);
            p.load();
            if (p.is_app())
                result.emplace_back(std::move(p));
        }

        closedir(dir);
        return result;
    }

    namespace detail
    {
        ssize_t read_file(const std::string& path, void *buffer, std::size_t buffer_size)
        {
            int fd = ::open(path.c_str(), O_RDONLY);
            if (fd == -1)
            {
                return 0;
            }

            ssize_t total_read = 0;
            while (total_read < buffer_size)
            {
                ssize_t bytes_read = read(fd, static_cast<char *>(buffer) + total_read, buffer_size - total_read);
                if (bytes_read <= 0)
                {
                    break;
                }
                total_read += bytes_read;
            }

            close(fd);
            return total_read;
        }
    }
}