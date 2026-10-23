#pragma once

#include "commondefs.hpp"
#include "str_util.hpp"

namespace dfi {

#ifdef PLATFORM_WINDOWS
    using so_handle = HMODULE;
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID) || defined(PLATFORM_APPLE) || defined(PLATFORM_FREEBSD)
    using so_handle = void *;
#else
    #error "unsupported platform"
#endif

#ifdef PLATFORM_WINDOWS
    namespace so_detail {
        inline std::wstring utf8_to_wide(const std::string &s) {
            int need = ::MultiByteToWideChar(
                CP_UTF8, MB_ERR_INVALID_CHARS,
                s.data(), (int)s.size(), nullptr, 0);
            UINT cp = CP_UTF8;
            DWORD flags = MB_ERR_INVALID_CHARS;
            if (need == 0) {
                cp = CP_ACP;
                flags = 0;
                need = ::MultiByteToWideChar(cp, flags, s.data(), (int)s.size(), nullptr, 0);
                if (need == 0) return std::wstring();
            }
            std::wstring w((size_t)need, L'\0');
            ::MultiByteToWideChar(cp, flags, s.data(), (int)s.size(), &w[0], need);
            return w;
        }
    }
#endif

    class so {
    public:
        so() = default;

        explicit so(std::string_view file_name) {
            static_cast<void>(open(file_name));
        }

        so(so const &) = delete;
        so &operator=(so const &) = delete;
        so(so &&) = delete;
        so &operator=(so &&) = delete;

        ~so() { close(); }

        [[nodiscard]] bool open(std::string_view file_name) {
            const std::string name{file_name};
            std::unique_lock lock{lib_handle_mtp_};
            if (lib_handle_ != nullptr) return false;
            lib_handle_ = open_impl(name);
            return true;
        }

        void close() noexcept {
            std::unique_lock lock{lib_handle_mtp_};
            if (lib_handle_ != nullptr) {
                close_impl(lib_handle_);
                lib_handle_ = nullptr;
            }
        }

        [[nodiscard]] bool ok() const noexcept {
            std::shared_lock lock{lib_handle_mtp_};
            return lib_handle_ != nullptr;
        }

        explicit operator bool() const noexcept { return ok(); }

        template <typename T>
        [[nodiscard]] T symbol(std::string_view sym_name) const noexcept {
            static_assert(std::is_pointer_v<T>, "T должен быть указателем (обычно на функцию)");
            const std::string name{sym_name};
            std::shared_lock lock{lib_handle_mtp_};
            void *raw = nullptr;
            if (lib_handle_ != nullptr)
                raw = sym_impl(lib_handle_, name);
            return reinterpret_cast<T>(raw);
        }

    private:
        static so_handle open_impl(const std::string &name) {
#ifdef PLATFORM_WINDOWS
            const std::wstring wide = so_detail::utf8_to_wide(name);
            so_handle h = ::LoadLibraryW(wide.c_str());
            if (h == nullptr)
                throw std::system_error{static_cast<int>(::GetLastError()),
                                        std::system_category(),
                                        "LoadLibrary(\"" + name + "\")"};
            return h;
#else
            ::dlerror();
            so_handle h = ::dlopen(name.c_str(), RTLD_NOW);
            if (h == nullptr) {
                const char *msg = ::dlerror();
                throw std::runtime_error{
                    "dlopen(\"" + name + "\"): " + (msg ? msg : "неизвестная ошибка")
                };
            }
            return h;
#endif
        }

        static void close_impl(so_handle h) noexcept {
#ifdef PLATFORM_WINDOWS
            ::FreeLibrary(h);
#else
            ::dlclose(h);
#endif
        }

        static void *sym_impl(so_handle h, const std::string &name) noexcept {
#ifdef PLATFORM_WINDOWS
            return reinterpret_cast<void *>(::GetProcAddress(h, name.c_str()));
#else
            return ::dlsym(h, name.c_str());
#endif
        }

        mutable std::shared_mutex lib_handle_mtp_{};
        so_handle lib_handle_{nullptr};
    };

}
