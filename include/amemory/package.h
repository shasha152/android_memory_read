#pragma once

#include "amemory/detail/base.h"
#include "amemory/detail/parse/parse_status.h"

#include "amemory/detail/read_file.h"
#include "amemory/meta/to_string_view.hpp"

#include <unordered_map>

namespace amemory
{
    struct status_keys
    {
        inline static std::string name = "Name";
        inline static std::string uid = "Uid";
    };

    template <typename Data = detail::data<1024 * 2>, typename Parse = detail::parse_status>
    class basic_package : public detail::base
    {
    public:
        using data_type = std::unordered_map<std::string, std::vector<detail::status_value>>;
    private:
        Data data_;
        Parse parse_;
        data_type data_map_;
        ssize_t data_size_{0};
    public:
        explicit basic_package(pid_t pid) : base(pid)
        {
        }
        explicit basic_package(std::string_view p) : base(p) {}

        basic_package(basic_package&& other) noexcept
            : base(std::move(other))
              , data_(std::move(other.data_))
              , parse_(std::move(other.parse_))
              , data_map_(std::move(other.data_map_))
        {
            parse_.content(
                meta::to_string_view(data_.get_data(), data_size_)
            );
        }

        [[nodiscard]]
        bool is_app() const noexcept
        {
            return uid_is_app() && name_is_app();
        }

        void load()
        {
            auto size = load_file();
            if (size <= 0 || size > data_.max_size())
                return;
            parse_.content(meta::to_string_view(data_.get_data(), size));

            while (parse_.max_pos() > parse_.curr_pos())
            {
                auto [key, values] = parse_();
                data_map_.emplace(std::string(key), std::move(values));
                parse_.next_line();
            }
        }

        template <typename Type, typename = std::enable_if_t<
            std::is_same_v<
                std::remove_cvref_t<Type>, typename data_type::key_type>>>
        std::optional<std::reference_wrapper<std::vector<detail::status_value>>>
        try_at(Type&& key) noexcept
        {
            auto it = data_map_.find(std::forward<Type>(key));
            if (it == data_map_.end())
                return std::nullopt;
            return std::make_optional(std::ref(it->second));
        }

        template <typename Type, typename = std::enable_if_t<
            std::is_same_v<
                std::remove_cvref_t<Type>, typename data_type::key_type>>>
        std::vector<detail::status_value>& at(Type&& key)
        {
            return data_map_.at(std::forward<Type>(key));
        }

        template <typename Type, typename = std::enable_if_t<
            std::is_same_v<
                std::remove_cvref_t<Type>, typename data_type::key_type>>>
        std::vector<detail::status_value>& operator[](Type&& key)
        {
            return data_map_[std::forward<Type>(key)];
        }

        template <typename Type, typename = std::enable_if_t<
            std::is_same_v<
                std::remove_cvref_t<Type>, typename data_type::key_type>>>
        [[nodiscard]]
        std::optional<std::reference_wrapper<const std::vector<detail::status_value>>>
        try_at(Type&& key) const noexcept
        {
            auto it = data_map_.find(std::forward<Type>(key));
            if (it == data_map_.end())
                return std::nullopt;
            return std::make_optional(std::ref(it->second));
        }

        template <typename Type, typename = std::enable_if_t<
            std::is_same_v<
                std::remove_cvref_t<Type>, typename data_type::key_type>>>
        [[nodiscard]]
        const std::vector<detail::status_value>& at(Type&& key) const
        {
            return data_map_.at(std::forward<Type>(key));
        }

        data_type::iterator begin() noexcept { return data_map_.begin(); }
        data_type::iterator end() noexcept { return data_map_.end(); }
        [[nodiscard]]
        data_type::const_iterator begin() const noexcept { return data_map_.begin(); }
        [[nodiscard]]
        data_type::const_iterator end() const noexcept { return data_map_.end(); }
    private:
        ssize_t load_file()
        {
            auto path_ = std::string(path())+ "/status";
            auto size = detail::read_file(path_, data_.get_data(), data_.max_size());
            data_size_ = size;
            return size;
        }

        [[nodiscard]]
        bool uid_is_app() const noexcept
        {
            auto it_uid = data_map_.find(status_keys::uid);
            if (it_uid == data_map_.end() || it_uid->second.empty() || it_uid->second[0].is_string())
                return false;
            std::int64_t uid = it_uid->second[0].as_integer();
            return uid >= 10000;
        }

        [[nodiscard]]
        bool name_is_app() const noexcept
        {
            auto it_name = data_map_.find(status_keys::name);
            if (it_name == data_map_.end() || it_name->second.empty() || it_name->second[0].is_integer())
                return false;


            const std::string process_name = std::string(it_name->second[0].as_string());
            auto pkg = process_name.substr(0, process_name.find(':'));

            for (const auto& u : std::filesystem::directory_iterator("/data/user"))
            {
                if (std::filesystem::exists(u.path() / pkg))
                    return true;
            }
            return false;
        }
    };

    using package = basic_package<>;

    std::vector<package> get_all_app_info();

    pid_t get_pid_by_name(std::string_view p);
}
