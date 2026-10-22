#pragma once

#include "inc/commondefs.hpp"
#include "inc/file_util.hpp"
#include "inc/str_util.hpp"
#include "inc/hash/hash.hpp"
#include "inc/mt_synchro.hpp"
#include "inc/math/math_util.hpp"
#include "inc/json.hpp"
#ifndef NOT_USING_DFI_USE_EMHASH8_MAP
#include "inc/emhash/hash_set8.hpp"
#include "inc/emhash/hash_table8.hpp"
#endif

#ifdef PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define NSTR(s) L##s
#else
#include <unistd.h>
#include <cstring>
#if defined(PLATFORM_APPLE)
#include <mach-o/dyld.h>
#elif defined(PLATFORM_FREEBSD)
#include <sys/sysctl.h>
#endif
#include <glob.h>
#define NSTR(s) s
#endif


#define DFIFUN(ARGS) [](std::vector<dfi::valbox> &ARGS) -> dfi::valbox
#define DFICLOSURE(ARGS) [&](std::vector<dfi::valbox> &ARGS) -> dfi::valbox
#define DFINUMARG(ARGS, INDX, TYPE) ARGS[INDX].cast_num_to_num<TYPE>()
#define DFIBOOLARG(ARGS, INDX) ARGS[INDX].cast_to_bool()
#define DFI_CHCK_FUN_PARMS_NUM_EQ(ARGS, NUM_ARGS) \
    if(ARGS.size() != (NUM_ARGS)) { \
        throw std::runtime_error{"wrong actual function arguments number"}; \
    }
#define DFI_CHCK_FUN_PARMS_NUM_LE(ARGS, NUM_ARGS) \
    if(ARGS.size() > (NUM_ARGS)) { \
        throw std::runtime_error{"wrong actual function arguments number"}; \
    }
#define DFI_CHCK_FUN_PARMS_NUM_GE(ARGS, NUM_ARGS) \
    if(ARGS.size() < (NUM_ARGS)) { \
        throw std::runtime_error{"wrong actual function arguments number"}; \
    }
#define DFI_CHCK_FUN_PARMS_NUM_IN_RANGE(ARGS, NUM_ARGS_MIN, NUM_ARGS_MAX) \
    if(!(ARGS.size() >= (NUM_ARGS_MIN) && ARGS.size() <= (NUM_ARGS_MAX))) { \
        throw std::runtime_error{"wrong actual function arguments number"}; \
    }
#define DFICLASSARG(ARGS, INDX, TYPE) ARGS[INDX].as_class<TYPE>()
#define DFITHIS(ARGS, TYPE) ARGS[0].as_class<TYPE>()

namespace dfi {

#define DFI_DEFINE_BASE_LINE_COL_ERROR(CLASS, BASE_CLASS) class CLASS: public BASE_CLASS { \
    public: \
        CLASS(std::int64_t l, std::int64_t c, std::string const &msg): \
            BASE_CLASS{msg}, \
            l_{l}, \
            c_{c} \
        { \
        } \
        char const *what() const noexcept override { \
            try { \
                std::stringstream ss{}; \
                ss << "at " << l_ + 1 << ":" << c_ + 1 << " - " << BASE_CLASS::what(); \
                buf_ = ss.str(); \
                return buf_.c_str(); \
            } catch(...) { \
            } \
            return def_; \
        } \
    private: \
        static inline char const *def_{""}; \
        mutable std::string buf_{}; \
        std::int64_t l_{}; \
        std::int64_t c_{}; \
    };

#define DFI_DEFINE_DERIVED_ERROR(CLASS, BASE_CLASS) class CLASS: public BASE_CLASS { \
    public: \
        CLASS(std::int64_t l, std::int64_t c, const std::string &msg): BASE_CLASS{l, c, msg} { \
        } \
    };

    DFI_DEFINE_BASE_LINE_COL_ERROR(logic_error, std::logic_error)
    DFI_DEFINE_BASE_LINE_COL_ERROR(runtime_error, std::runtime_error)
    DFI_DEFINE_DERIVED_ERROR(compilation_error, logic_error)
    DFI_DEFINE_DERIVED_ERROR(domain_error, logic_error)
    DFI_DEFINE_DERIVED_ERROR(future_error, logic_error)
    DFI_DEFINE_DERIVED_ERROR(invalid_argument, logic_error)
    DFI_DEFINE_DERIVED_ERROR(length_error, logic_error)
    DFI_DEFINE_DERIVED_ERROR(out_of_range, logic_error)
    DFI_DEFINE_DERIVED_ERROR(range_error, runtime_error)
    DFI_DEFINE_DERIVED_ERROR(overflow_error, runtime_error)
    DFI_DEFINE_DERIVED_ERROR(underflow_error, runtime_error)
    DFI_DEFINE_DERIVED_ERROR(system_error, runtime_error)

#undef DFI_DEFINE_BASE_LINE_COL_ERROR
#undef DFI_DEFINE_DERIVED_ERROR

    template <std::uint8_t T_numBytes>
    using UintSelector =
        typename std::conditional<T_numBytes == 1, std::uint8_t,
            typename std::conditional<T_numBytes == 2, std::uint16_t,
                typename std::conditional<T_numBytes == 3 || T_numBytes == 4, std::uint32_t,
                    std::uint64_t
                >::type
            >::type
        >::type;

#ifdef DFI_DEBUGGING
    template<typename K_T, typename V_T>
    using num_map_t = std::map<K_T, V_T>;
    template<typename V_T>
    using str_map_t = std::map<std::string, V_T>;
#else
    #ifndef NOT_USING_DFI_USE_EMHASH8_MAP
    template<typename K_T, typename V_T>
    using num_map_t = emhash_8::HashMap<K_T, V_T, num_cast_hash<K_T>>;
    template<typename V_T>
    using str_map_t = emhash_8::HashMap<std::string, V_T>;
    #else
    template<typename K_T, typename V_T>
    using num_map_t = std::unordered_map<K_T, V_T>;
    template<typename V_T>
    using str_map_t = std::unordered_map<std::string, V_T>;
    #endif
#endif


#ifdef DFI_USE_CUSTOM_SHARED_MUTEX
    using shared_mutex = mt::atomic_rw_spin_mutex;
#else
    using shared_mutex = std::shared_mutex;
#endif

#ifdef DFI_USE_CUSTOM_MUTEX
    using mutex = mt::atomic_spin_mutex;
#else
    using mutex = std::mutex;
#endif

    static bool is_identifier(std::string const &ident) {
        if(ident.size() == 0) {
            return false;
        }
        std::wstring wid{dfi::str_util::from_utf8(ident)};
        if(!(dfi::str_util::fltr<std::wstring>::isalpha(wid[0]) || wid[0] == '_')) {
            return false;
        }
        for(std::size_t i{1}; i < wid.size(); ++i) {
            if(!(dfi::str_util::fltr<std::wstring>::isalnum(wid[0]) || wid[0] == '_')) {
                return false;
            }
        }
        return true;
    }

    static bool is_keyword(std::wstring const &str) {
        return str == L"if" ||
               str == L"else" ||
               str == L"true" ||
               str == L"false" ||
               str == L"undefined" ||
               str == L"for" ||
               str == L"while" ||
               str == L"emit" ||
               str == L"delay" ||
               str == L"return" ||
               str == L"break" ||
               str == L"continue" ||
               str == L"try" ||
               str == L"catch" ||
               str == L"throw" ||
               str == L"function" ||
               str == L"this" ||
               str == L"sizeof" ||
               str == L"typeof"
        ;
    }

    static bool expression_part(std::wstring const &str) {
        return str != L"if" &&
               str != L"else" &&
               str != L"for" &&
               str != L"while" &&
               str != L"emit" &&
               str != L"delay" &&
               str != L"return" &&
               str != L"break" &&
               str != L"continue" &&
               str != L"try" &&
               str != L"catch" &&
               str != L"throw" &&
               str != L"function"
        ;
    }

    template<typename T>
    dfi::math::vector4<T> vec4_from_json(json const &j) {
        dfi::math::vector4<T> res{};
        try {
            if(j.is_array()) {
                for(std::size_t i = 0; i < 4 && i < j.size(); ++i) {
                    res[i] = j[i].try_as_long_double();
                }
            } else if(j.is_object()) {
                res.x() = j.key_exists("x") ? j["x"].try_as_long_double() : 0;
                res.y() = j.key_exists("y") ? j["y"].try_as_long_double() : 0;
                res.z() = j.key_exists("z") ? j["z"].try_as_long_double() : 0;
                res.w() = j.key_exists("w") ? j["w"].try_as_long_double() : 0;
            }
        } catch(...) {
            res = dfi::math::vector4<T>{};
        }
        return res;
    }

    template<typename T>
    dfi::math::vector4<T> vec4_from_str(std::string const &s) {
        dfi::math::vector4<T> res{};
        try {
            json j{json::deserialize(s)};
            res = vec4_from_json<T>(j);
        } catch(...) {
            res = dfi::math::vector4<T>{};
        }
        return res;
    }

    template<typename T>
    dfi::math::vector4<T> vec4_from_str(std::wstring const &s) {
        dfi::math::vector4<T> res{};
        try {
            json j{json::deserialize(dfi::str_util::to_utf8(s))};
            res = vec4_from_json<T>(j);
        } catch(...) {
            res = dfi::math::vector4<T>{};
        }
        return res;
    }

    template<typename T>
    dfi::math::matrix4<T> mat4_from_json(json const &j) {
        dfi::math::matrix4<T> res{};
        try {
            if(j.is_array()) {
                for(std::size_t i = 0; i < 16 && i < j.size(); ++i) {
                    res[i] = j[i].try_as_long_double();
                }
            } else if(j.is_object()) {
                res.x() = j.key_exists("x") ? j["x"].try_as_long_double() : 0;
                res.y() = j.key_exists("y") ? j["y"].try_as_long_double() : 0;
                res.z() = j.key_exists("z") ? j["z"].try_as_long_double() : 0;
                res.w() = j.key_exists("w") ? j["w"].try_as_long_double() : 0;
            }
        } catch(...) {
            res = dfi::math::matrix4<T>{};
        }
        return res;
    }

    template<typename T>
    class map_array {
    public:
        map_array() {}

        T &operator[](std::size_t indx) & {
            if(indx >= size_) {
                size_ = indx + 1;
            }
            return map_[indx];
        }

        T at(std::size_t indx) const {
            if(indx >= size_) {
                throw std::runtime_error{"index out of range"};
            }
            auto it{map_.find(indx)};
            if(it == map_.end()) {
                return T{};
            }
            return it->second;
        }

        std::size_t size() const {
            return size_;
        }

        void resize(std::size_t n) {
            size_ = n;
            for(auto it{map_.lower_bound(n)}; it != map_.end(); ) {
                it = map_.erase(it);
            }
        }

        std::map<std::size_t, T> const &m() const & {
            return map_;
        }

        map_array subarray(
            std::size_t start_index,
            std::size_t count = std::numeric_limits<std::size_t>::max()
        ) const {
            map_array res{};
            res.resize((std::min)(count, size() - start_index));
            for(auto it{map_.lower_bound(start_index)}; it != map_.end(); ++ it) {
                if(it->first - start_index >= count) {
                    break;
                }
                res[it->first] = it->second;
            }
            return res;
        }

        bool contains(size_t indx) const {
            return indx < size_ && map_.find(indx) != map_.end();
        }

        void traverse_full(
            std::function<void(size_t, T const &)> fun,
            size_t start = 0,
            size_t count = std::numeric_limits<std::size_t>::max()
        ) const {
            auto it{map_.lower_bound(start)};
            for(size_t i{start}; i < size_ && i < start + count; ++i) {
                if(it != map_.end() && i == it->first) {
                    fun(i, it->second);
                    ++it;
                } else {
                    fun(i, T{});
                }
            }
        }

        void traverse_real(
            std::function<void(size_t, T const &)> fun,
            size_t start = 0,
            size_t count = std::numeric_limits<std::size_t>::max()
        ) const {
            for(
                auto it{map_.lower_bound(start)};
                it != map_.end() && it->first < start + count;
                ++it
            ) {
                fun(it->first, it->second);
            }
        }

        void clear() {
            size_ = 0;
            map_.clear();
        }

        bool empty() const {
            return size_ == 0;
        }

        void push_back(T const &v) {
            map_[size_++] = v;
        }

        void push_back(T &&v) {
            map_[size_++] = std::move(v);
        }

        T pop_back() {
            T res{};
            if(empty()) {
                throw std::runtime_error{"array empty"};
            }
            --size_;
            auto it{map_.lower_bound(size_)};
            if(it != map_.end()) {
                res = std::move(it->second);
                map_.erase(it);
            }
            return res;
        }

    private:
        std::map<std::size_t, T> map_{};
        std::size_t size_{0};
    };

    namespace dldetail {

        namespace fs = std::filesystem;

        using nstr = fs::path::string_type;
        using nchar = fs::path::value_type;

        static bool ends_ci(const nstr& s, const nstr& lower_ascii_suffix) {
            if(s.size() < lower_ascii_suffix.size()) return false;
            const nstr::size_type off = s.size() - lower_ascii_suffix.size();
            for(nstr::size_type i = 0; i < lower_ascii_suffix.size(); ++i)
                if(/*ascii_lower*/str_util::fltr<nstr>::islower(s[off + i]) != lower_ascii_suffix[i]) return false;
            return true;
        }

        static std::optional<std::vector<unsigned long long>> parse_version(const nstr& s) {
            if(s.empty()) return std::nullopt;
            std::vector<unsigned long long> v;
            nstr::size_type start = 0;
            for(;;) {
                const nstr::size_type dot = s.find(NSTR('.'), start);
                const nstr comp = s.substr(start, dot == nstr::npos ? nstr::npos : dot - start);
                if(!str_util::fltr<nstr>::all_digits(comp)) return std::nullopt;
                v.push_back(str_util::atoui<nstr>(comp));
                if(dot == nstr::npos) break;
                start = dot + 1;
                if(start == s.size()) return std::nullopt;
            }
            return v;
        }

        static bool has_so_ext(const nstr& name) {
            nstr::size_type pos = name.rfind(NSTR(".so"));
            while(pos != nstr::npos) {
                const nstr tail = name.substr(pos + 3);
                if(tail.empty()) return true;
                if(tail[0] == NSTR('.') && parse_version(tail.substr(1))) return true;
                if(pos == 0) break;
                pos = name.rfind(NSTR(".so"), pos - 1);
            }
            return false;
        }

        static std::optional<fs::path> check(const fs::path& dir, const nstr& file) {
            const fs::path p = dir / fs::path(file);
            std::error_code ec;
            if(fs::is_regular_file(p, ec)) return p;
            return std::nullopt;
        }

        static std::optional<fs::path> best_versioned(const fs::path& dir, const nstr& so_name) {
            const nstr prefix = so_name + NSTR(".");
            bool found = false;
            nstr best;
            std::vector<unsigned long long> best_v;
            std::error_code ec;
            for(fs::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec)) {
                const nstr fn = it->path().filename().native();
                if(fn.size() <= prefix.size()) continue;
                if(fn.compare(0, prefix.size(), prefix) != 0) continue;
                const auto v = parse_version(fn.substr(prefix.size()));
                if(!v) continue;
                std::error_code ec2;
                if(!fs::is_regular_file(it->path(), ec2)) continue;
                if(!found || *v > best_v) { found = true; best_v = *v; best = fn; }
            }
            if(!found) return std::nullopt;
            return dir / fs::path(best);
        }

        static std::optional<fs::path> search_in_dir(const fs::path& dir, const nstr& name, bool win) {
            std::vector<nstr> variants;
            variants.push_back(name);
            if(!win) variants.push_back(NSTR("lib") + name);

            for(std::size_t vi = 0; vi < variants.size(); ++vi) {
                const nstr& v = variants[vi];
                const bool literal = (vi == 0);

                if(win) {
                    if(auto hit = check(dir, v)) return hit;
                    if(!ends_ci(v, NSTR(".dll")))
                        if(auto hit = check(dir, v + NSTR(".dll"))) return hit;
                } else {
                    const bool ext = has_so_ext(v);
                    if(literal || ext)
                        if(auto hit = check(dir, v)) return hit;
                    if(ext) {
                        if(auto hit = best_versioned(dir, v)) return hit;
                    } else {
                        const nstr with_so = v + NSTR(".so");
                        if(auto hit = check(dir, with_so)) return hit;
                        if(auto hit = best_versioned(dir, with_so)) return hit;
                    }
                }
            }
            return std::nullopt;
        }

        static fs::path input_to_path(std::string_view s) {
            if(s.empty()) return {};
#ifdef PLATFORM_WINDOWS
            int need = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                           s.data(), (int)s.size(), nullptr, 0);
            UINT cp = CP_UTF8; DWORD flags = MB_ERR_INVALID_CHARS;
            if(need == 0) {
                cp = CP_ACP; flags = 0;
                need = MultiByteToWideChar(cp, flags, s.data(), (int)s.size(), nullptr, 0);
                if(need == 0) return {};
            }
            std::wstring w((size_t)need, L'\0');
            MultiByteToWideChar(cp, flags, s.data(), (int)s.size(), &w[0], need);
            return fs::path(std::move(w));
#else
            return fs::path(std::string(s));
#endif
        }

        static std::optional<fs::path> exe_path() {
#if defined(PLATFORM_WINDOWS)
            for(DWORD cap = 512; cap <= (1u << 16); cap *= 2) {
                std::wstring buf(cap, L'\0');
                const DWORD n = GetModuleFileNameW(nullptr, &buf[0], cap);
                if(n != 0 && n < cap) { buf.resize(n); return fs::path(std::move(buf)); }
                if(n == 0 && GetLastError() != ERROR_INSUFFICIENT_BUFFER) break;
            }
            return std::nullopt;
#elif defined(PLATFORM_LINUX)
            for(std::size_t cap = 512; cap <= (std::size_t)1 << 16; cap *= 2) {
                std::string buf(cap, '\0');
                const ssize_t n = ::readlink("/proc/self/exe", buf.data(), cap);
                if(n < 0) break;
                if((std::size_t)n < cap) { buf.resize((std::size_t)n); return fs::path(std::move(buf)); }
            }
            return std::nullopt;
#elif defined(PLATFORM_APPLE)
            uint32_t sz = 0;
            if(_NSGetExecutablePath(nullptr, &sz) == -1 && sz > 0) {
                std::string buf(sz, '\0');
                if(_NSGetExecutablePath(&buf[0], &sz) == 0) {
                    buf.resize(std::strlen(buf.c_str()));
                    std::error_code ec;
                    const fs::path raw(buf);
                    const fs::path c = fs::canonical(raw, ec);
                    return ec ? raw : c;
                }
            }
            return std::nullopt;
#elif defined(PLATFORM_BSD)
            int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
            std::size_t len = 0;
            if(sysctl(mib, 4, nullptr, &len, nullptr, 0) == 0 && len > 1) {
                std::string buf(len, '\0');
                if(sysctl(mib, 4, &buf[0], &len, nullptr, 0) == 0) {
                    buf.resize(std::strlen(buf.c_str()));
                    return fs::path(std::move(buf));
                }
            }
            return std::nullopt;
#else
            return std::nullopt;
#endif
        }

#ifdef PLATFORM_WINDOWS
        static std::wstring get_env_w(const wchar_t* name) {
            DWORD n = GetEnvironmentVariableW(name, nullptr, 0);
            if(n == 0) return {};
            std::wstring buf(n, L'\0');
            const DWORD m = GetEnvironmentVariableW(name, &buf[0], n);
            if(m == 0) return {};
            buf.resize(m);
            return buf;
        }

        static void append_env_win(std::vector<fs::path>& out, const std::wstring& val) {
            std::size_t start = 0;
            for(;;) {
                const std::size_t sc = val.find(L';', start);
                const std::wstring comp = val.substr(start, sc == std::wstring::npos ? std::wstring::npos : sc - start);
                if(!comp.empty()) out.push_back(fs::path(comp));
                if(sc == std::wstring::npos) break;
                start = sc + 1;
            }
        }

        static fs::path system_dir_w() {
            std::wstring buf(1024, L'\0');
            UINT n = GetSystemDirectoryW(&buf[0], (UINT)buf.size());
            if(n == 0) return {};
            if(n > buf.size()) {
                buf.resize(n);
                n = GetSystemDirectoryW(&buf[0], (UINT)buf.size());
                if(n == 0 || n > buf.size()) return {};
            }
            return fs::path(buf.substr(0, n));
        }

        static fs::path windows_dir_w() {
            std::wstring buf(1024, L'\0');
            UINT n = GetWindowsDirectoryW(&buf[0], (UINT)buf.size());
            if(n == 0) return {};
            if(n > buf.size()) {
                buf.resize(n);
                n = GetWindowsDirectoryW(&buf[0], (UINT)buf.size());
                if(n == 0 || n > buf.size()) return {};
            }
            return fs::path(buf.substr(0, n));
        }
#else
        static void append_env_posix(std::vector<fs::path>& out, const char* val) {
            if(!val) return;
            const std::string s(val);
            if(s.empty()) return;
            std::size_t start = 0;
            for(;;) {
                const std::size_t colon = s.find(':', start);
                std::string comp = s.substr(start, colon == std::string::npos ? std::string::npos : colon - start);
                if(comp.empty()) comp = ".";
                out.push_back(fs::path(comp));
                if(colon == std::string::npos) break;
                start = colon + 1;
            }
        }

        static void add_conf_file(const std::string& file, int depth, std::vector<fs::path>& out);

        static void expand_include_glob(const std::string& pattern, int depth, std::vector<fs::path>& out) {
            glob_t g;
            std::memset(&g, 0, sizeof g);
            if(glob(pattern.c_str(), 0, nullptr, &g) == 0)
                for(std::size_t i = 0; i < g.gl_pathc; ++i)
                    add_conf_file(g.gl_pathv[i], depth, out);
            globfree(&g);
        }

        static void add_conf_file(const std::string& file, int depth, std::vector<fs::path>& out) {
            if(depth > 4) return;
            std::ifstream in(file);
            if(!in) return;
            std::string line;
            while(std::getline(in, line)) {
                const std::size_t h = line.find('#');
                if(h != std::string::npos) line.resize(h);
                const auto sp = [](char c) { return c == ' ' || c == '\t' || c == '\r'; };
                std::size_t b = 0, e = line.size();
                while(b < e && sp(line[b])) ++b;
                while(e > b && sp(line[e - 1])) --e;
                line = line.substr(b, e - b);
                if(line.empty()) continue;
                if(line.compare(0, 7, "include") == 0 &&
                    (line.size() == 7 || line[7] == ' ' || line[7] == '\t')) {
                    std::size_t pb = 7;
                    while(pb < line.size() && (line[pb] == ' ' || line[pb] == '\t')) ++pb;
                    if(pb < line.size()) expand_include_glob(line.substr(pb), depth + 1, out);
                    continue;
                }
                out.push_back(fs::path(line));
            }
        }
#endif

        static std::vector<fs::path> unique_existing_dirs(std::vector<fs::path> raw) {
            std::vector<fs::path> out;
            std::set<fs::path> seen;
            for(fs::path& d : raw) {
                if(d.empty()) continue;
                std::error_code ec;
                if(!fs::is_directory(d, ec)) continue;
                fs::path c = fs::canonical(d, ec);
                if(ec) c = fs::absolute(d, ec).lexically_normal();
                if(ec) continue;
                if(!seen.insert(c).second) continue;
                out.push_back(std::move(c));
            }
            return out;
        }

        static std::vector<fs::path> build_search_dirs() {
            std::vector<fs::path> raw;


            if(auto exe = exe_path()) raw.push_back(exe->parent_path());

#ifdef PLATFORM_WINDOWS
            if(fs::path sd = system_dir_w(); !sd.empty()) raw.push_back(std::move(sd));
            if(fs::path wd = windows_dir_w(); !wd.empty()) {
                raw.push_back(wd / L"System");
                raw.push_back(std::move(wd));
            }
#else
            for(const char* d : {"/lib", "/usr/lib", "/lib64", "/usr/lib64", "/usr/local/lib"})
                raw.push_back(fs::path(d));
            add_conf_file("/etc/ld.so.conf", 1, raw);
#endif
            {
                std::error_code ec;
                fs::path c = fs::current_path(ec);
                if(!ec) raw.push_back(std::move(c));
            }
#ifdef PLATFORM_WINDOWS
            append_env_win(raw, get_env_w(L"PATH"));
#else
            append_env_posix(raw, std::getenv("LD_LIBRARY_PATH"));
            append_env_posix(raw, std::getenv("PATH"));
#endif
            return unique_existing_dirs(std::move(raw));
        }

    }

    static std::optional<std::string> locate_dynamic_library(
        std::string_view name,
        std::string_view first_dir
    ) {
        namespace fs = std::filesystem;

        if(name.empty()) return std::nullopt;
#if defined(PLATFORM_WINDOWS)
        constexpr bool win = true;
#else
        constexpr bool win = false;
#endif
        using dldetail::nstr;

        const fs::path in = dldetail::input_to_path(name);
        const nstr   fname = in.filename().native();
        const fs::path dirpart = in.parent_path();

        if(fname.empty()) {
            std::error_code ec;
            if(fs::is_regular_file(in, ec)) { return in; }
            return std::nullopt;
        }

        if(!first_dir.empty()) {
            if(auto hit = dldetail::search_in_dir(first_dir, fname, win)) { return hit; }
        }

        if(!dirpart.empty()) {
            if(auto hit = dldetail::search_in_dir(dirpart, fname, win)) { return hit; }
        }

        for(const fs::path& d : dldetail::build_search_dirs()) {
            if(auto hit = dldetail::search_in_dir(d, fname, win)) { return hit; }
        }

        return std::nullopt;
    }

}
